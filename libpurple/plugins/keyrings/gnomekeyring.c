/**
 * @file gnomekeyring.c Gnome keyring password storage
 * @ingroup plugins
 */

/* purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program ; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1301 USA
 */

#include "internal.h"
#include "account.h"
#include "debug.h"
#include "glibcompat.h"
#include "keyring.h"
#include "plugins.h"
#include "version.h"

#include <gnome-keyring.h>
#include <gnome-keyring-memory.h>

#warning Gnome-Keyring API is deprecated, please use libSecret keyring instead.

#define GNOMEKEYRING_NAME        N_("GNOME Keyring")
#define GNOMEKEYRING_DESCRIPTION N_("This plugin will store passwords in " \
	"GNOME Keyring.")
#define GNOMEKEYRING_ID          "keyring-gnomekeyring"
#define GNOMEKEYRING_AUTHORS \
	{ "Tomek Wasilczyk <twasilczyk@pidgin.im>", NULL }

#define GNOMEKEYRING_DOMAIN      (g_quark_from_static_string(GNOMEKEYRING_ID))

static PurpleKeyring *keyring_handler = NULL;
static GList *request_queue = NULL;
static gpointer current_request = NULL;

typedef struct
{
	enum
	{
		GNOMEKEYRING_REQUEST_READ,
		GNOMEKEYRING_REQUEST_SAVE
	} type;
	PurpleAccount *account;
	gchar *password;
	union
	{
		PurpleKeyringReadCallback read;
		PurpleKeyringSaveCallback save;
	} cb;
	gpointer cb_data;
	gboolean handled;
} gnomekeyring_request;

static void gnomekeyring_cancel_queue(void);
static void gnomekeyring_process_queue(void);

static void gnomekeyring_request_free(gnomekeyring_request *req)
{
	if (req->password != NULL) {
		memset(req->password, 0, strlen(req->password));
		G_GNUC_BEGIN_IGNORE_DEPRECATIONS
		gnome_keyring_memory_free(req->password);
		G_GNUC_END_IGNORE_DEPRECATIONS
	}
	g_free(req);
}

static void
gnomekeyring_enqueue(gnomekeyring_request *req)
{
	request_queue = g_list_append(request_queue, req);
	gnomekeyring_process_queue();
}

static void
gnomekeyring_read_cb(GnomeKeyringResult result, const char *password,
	gpointer _req)
{
	gnomekeyring_request *req = _req;
	PurpleAccount *account;
	GError *error = NULL;

	g_return_if_fail(req != NULL);

	current_request = NULL;
	account = req->account;

	if (result == GNOME_KEYRING_RESULT_OK) {
		error = NULL;
	} else if (result == GNOME_KEYRING_RESULT_NO_MATCH) {
		error = g_error_new(PURPLE_KEYRING_ERROR,
			PURPLE_KEYRING_ERROR_NOPASSWORD,
			_("No password found for account."));
	} else if (result == GNOME_KEYRING_RESULT_DENIED ||
		result == GNOME_KEYRING_RESULT_CANCELLED)
	{
		error = g_error_new(PURPLE_KEYRING_ERROR,
			PURPLE_KEYRING_ERROR_ACCESSDENIED,
			_("Access denied."));
		gnomekeyring_cancel_queue();
	} else if (result == GNOME_KEYRING_RESULT_NO_KEYRING_DAEMON ||
		result == GNOME_KEYRING_RESULT_IO_ERROR)
	{
		error = g_error_new(PURPLE_KEYRING_ERROR,
			PURPLE_KEYRING_ERROR_BACKENDFAIL,
			_("Communication with GNOME Keyring failed."));
	} else {
		error = g_error_new(PURPLE_KEYRING_ERROR,
			PURPLE_KEYRING_ERROR_BACKENDFAIL,
			_("Unknown error (code: %d)."), result);
	}

	if (error == NULL && password == NULL) {
		error = g_error_new(PURPLE_KEYRING_ERROR,
			PURPLE_KEYRING_ERROR_BACKENDFAIL,
			_("Unknown error (password empty)."));
	}

	if (error == NULL) {
		purple_debug_misc("keyring-gnome",
			"Got password for account %s (%s).\n",
			purple_account_get_username(account),
			purple_account_get_protocol_id(account));
	} else if (result == GNOME_KEYRING_RESULT_NO_MATCH) {
		if (purple_debug_is_verbose()) {
			purple_debug_info("keyring-gnome",
				"Password for account %s (%s) isn't stored.\n",
				purple_account_get_username(account),
				purple_account_get_protocol_id(account));
		}
	} else {
		password = NULL;
		purple_debug_warning("keyring-gnome", "Failed to read "
			"password for account %s (%s), code: %d.\n",
			purple_account_get_username(account),
			purple_account_get_protocol_id(account),
			result);
	}

	if (req->cb.read != NULL)
		req->cb.read(account, password, error, req->cb_data);
	req->handled = TRUE;

	if (error)
		g_error_free(error);

	gnomekeyring_process_queue();
}

static void
gnomekeyring_save_cb(GnomeKeyringResult result, gpointer _req)
{
	gnomekeyring_request *req = _req;
	PurpleAccount *account;
	GError *error = NULL;
	gboolean already_removed = FALSE;

	g_return_if_fail(req != NULL);

	current_request = NULL;
	account = req->account;

	if (result == GNOME_KEYRING_RESULT_OK) {
		error = NULL;
	} else if (result == GNOME_KEYRING_RESULT_NO_MATCH &&
		req->password == NULL)
	{
		error = NULL;
		already_removed = TRUE;
	} else if (result == GNOME_KEYRING_RESULT_DENIED ||
		result == GNOME_KEYRING_RESULT_CANCELLED)
	{
		error = g_error_new(PURPLE_KEYRING_ERROR,
			PURPLE_KEYRING_ERROR_ACCESSDENIED,
			_("Access denied."));
		gnomekeyring_cancel_queue();
	} else if (result == GNOME_KEYRING_RESULT_NO_KEYRING_DAEMON ||
		result == GNOME_KEYRING_RESULT_IO_ERROR)
	{
		error = g_error_new(PURPLE_KEYRING_ERROR,
			PURPLE_KEYRING_ERROR_BACKENDFAIL,
			_("Communication with GNOME Keyring failed."));
	} else {
		error = g_error_new(PURPLE_KEYRING_ERROR,
			PURPLE_KEYRING_ERROR_BACKENDFAIL,
			_("Unknown error (code: %d)."), result);
	}

	if (already_removed) {
		/* no operation */
	} else if (error == NULL) {
		purple_debug_misc("keyring-gnome",
			"Password %s for account %s (%s).\n",
			req->password ? "saved" : "removed",
			purple_account_get_username(account),
			purple_account_get_protocol_id(account));
	} else {
		purple_debug_warning("keyring-gnome", "Failed updating "
			"password for account %s (%s), code: %d.\n",
			purple_account_get_username(account),
			purple_account_get_protocol_id(account),
			result);
	}

	if (req->cb.save != NULL)
		req->cb.save(account, error, req->cb_data);
	req->handled = TRUE;

	if (error)
		g_error_free(error);

	gnomekeyring_process_queue();
}

static void
gnomekeyring_request_cancel(gpointer _req)
{
	gnomekeyring_request *req = _req;
	PurpleAccount *account;
	GError *error;

	g_return_if_fail(req != NULL);

	if (req->handled) {
		gnomekeyring_request_free(req);
		return;
	}

	purple_debug_warning("keyring-gnome",
		"operation cancelled (%d %s:%s)\n", req->type,
		purple_account_get_protocol_id(req->account),
		purple_account_get_username(req->account));

	account = req->account;
	error = g_error_new(PURPLE_KEYRING_ERROR,
		PURPLE_KEYRING_ERROR_CANCELLED,
		_("Operation cancelled."));
	if (req->type == GNOMEKEYRING_REQUEST_READ && req->cb.read)
		req->cb.read(account, NULL, error, req->cb_data);
	if (req->type == GNOMEKEYRING_REQUEST_SAVE && req->cb.save)
		req->cb.save(account, error, req->cb_data);
	g_error_free(error);

	gnomekeyring_request_free(req);
	gnomekeyring_process_queue();
}

static void
gnomekeyring_cancel_queue(void)
{
	GList *cancel_list = request_queue;

	if (request_queue == NULL)
		return;

	purple_debug_info("gnome-keyring", "cancelling all pending requests\n");
	request_queue = NULL;

	g_list_free_full(cancel_list, gnomekeyring_request_cancel);
}

static void
gnomekeyring_process_queue(void)
{
	gnomekeyring_request *req;
	PurpleAccount *account;
	GList *first;

	if (request_queue == NULL)
		return;

	if (current_request) {
		if (purple_debug_is_verbose())
			purple_debug_misc("keyring-gnome", "busy...\n");
		return;
	}

	first = g_list_first(request_queue);
	req = first->data;
	request_queue = g_list_delete_link(request_queue, first);
	account = req->account;

	if (purple_debug_is_verbose()) {
		purple_debug_misc("keyring-gnome",
			"%s password for account %s (%s)\n",
			req->type == GNOMEKEYRING_REQUEST_READ ? "reading" :
			(req->password == NULL ? "removing" : "updating"),
			purple_account_get_username(account),
			purple_account_get_protocol_id(account));
	}

	if (req->type == GNOMEKEYRING_REQUEST_READ) {
		G_GNUC_BEGIN_IGNORE_DEPRECATIONS
		current_request = gnome_keyring_find_password(
			GNOME_KEYRING_NETWORK_PASSWORD, gnomekeyring_read_cb,
			req, gnomekeyring_request_cancel,
			"user", purple_account_get_username(account),
			"protocol", purple_account_get_protocol_id(account),
			NULL);
		G_GNUC_END_IGNORE_DEPRECATIONS
	} else if (req->type == GNOMEKEYRING_REQUEST_SAVE &&
		req->password != NULL)
	{
		gchar *display_name = g_strdup_printf(
			_("Pidgin IM password for account %s"),
			purple_account_get_username(account));
		G_GNUC_BEGIN_IGNORE_DEPRECATIONS
		current_request = gnome_keyring_store_password(
			GNOME_KEYRING_NETWORK_PASSWORD, GNOME_KEYRING_DEFAULT,
			display_name, req->password, gnomekeyring_save_cb, req,
			gnomekeyring_request_cancel,
			"user", purple_account_get_username(account),
			"protocol", purple_account_get_protocol_id(account),
			NULL);
		G_GNUC_END_IGNORE_DEPRECATIONS
		g_free(display_name);
	} else if (req->type == GNOMEKEYRING_REQUEST_SAVE &&
		req->password == NULL)
	{
		G_GNUC_BEGIN_IGNORE_DEPRECATIONS
		current_request = gnome_keyring_delete_password(
			GNOME_KEYRING_NETWORK_PASSWORD, gnomekeyring_save_cb,
			req, gnomekeyring_request_cancel,
			"user", purple_account_get_username(account),
			"protocol", purple_account_get_protocol_id(account),
			NULL);
		G_GNUC_END_IGNORE_DEPRECATIONS
	} else {
		g_return_if_reached();
	}
}

static void
gnomekeyring_read(PurpleAccount *account, PurpleKeyringReadCallback cb,
	gpointer data)
{
	gnomekeyring_request *req;

	g_return_if_fail(account != NULL);

	req = g_new0(gnomekeyring_request, 1);
	req->type = GNOMEKEYRING_REQUEST_READ;
	req->account = account;
	req->cb.read = cb;
	req->cb_data = data;

	gnomekeyring_enqueue(req);
}

static void
gnomekeyring_save(PurpleAccount *account, const gchar *password,
	PurpleKeyringSaveCallback cb, gpointer data)
{
	gnomekeyring_request *req;

	g_return_if_fail(account != NULL);

	req = g_new0(gnomekeyring_request, 1);
	req->type = GNOMEKEYRING_REQUEST_SAVE;
	req->account = account;
	G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	req->password = gnome_keyring_memory_strdup(password);
	G_GNUC_END_IGNORE_DEPRECATIONS
	req->cb.save = cb;
	req->cb_data = data;

	gnomekeyring_enqueue(req);
}

static void
gnomekeyring_cancel(void)
{
	G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	gnomekeyring_cancel_queue();
	G_GNUC_END_IGNORE_DEPRECATIONS
	if (current_request) {
		G_GNUC_BEGIN_IGNORE_DEPRECATIONS
		gnome_keyring_cancel_request(current_request);
		G_GNUC_END_IGNORE_DEPRECATIONS
		while (g_main_iteration(FALSE));
	}
}

static void
gnomekeyring_close(void)
{
	gnomekeyring_cancel();
}

static PurplePluginInfo *
plugin_query(GError **error)
{
	const gchar * const authors[] = GNOMEKEYRING_AUTHORS;

	return purple_plugin_info_new(
		"id",           GNOMEKEYRING_ID,
		"name",         GNOMEKEYRING_NAME,
		"version",      DISPLAY_VERSION,
		"category",     N_("Keyring"),
		"summary",      "GNOME Keyring Plugin",
		"description",  GNOMEKEYRING_DESCRIPTION,
		"authors",      authors,
		"website",      PURPLE_WEBSITE,
		"abi-version",  PURPLE_ABI_VERSION,
		"flags",        PURPLE_PLUGIN_INFO_FLAGS_INTERNAL,
		NULL
	);
}

static gboolean
plugin_load(PurplePlugin *plugin, GError **error)
{
	GModule *gkr_module;

	/* libgnome-keyring may crash, if was unloaded before glib main loop
	 * termination.
	 */
	gkr_module = g_module_open("libgnome-keyring", 0);
	if (gkr_module == NULL) {
		gkr_module = g_module_open("libgnome-keyring.so.0", 0);
		if (gkr_module == NULL) {
			gkr_module = g_module_open("libgnome-keyring.so.1", 0);
		}
	}
	if (gkr_module == NULL) {
		purple_debug_info("keyring-gnome", "GNOME Keyring module not "
			"found\n");
		return FALSE;
	}
	g_module_make_resident(gkr_module);

	G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	if (!gnome_keyring_is_available()) {
		G_GNUC_END_IGNORE_DEPRECATIONS
		g_set_error(error, GNOMEKEYRING_DOMAIN, 0, "GNOME Keyring service is "
			"disabled.");
		purple_debug_info("keyring-gnome", "GNOME Keyring service is "
			"disabled\n");
		return FALSE;
	}

	keyring_handler = purple_keyring_new();

	purple_keyring_set_name(keyring_handler, _(GNOMEKEYRING_NAME));
	purple_keyring_set_id(keyring_handler, GNOMEKEYRING_ID);
	purple_keyring_set_read_password(keyring_handler, gnomekeyring_read);
	purple_keyring_set_save_password(keyring_handler, gnomekeyring_save);
	purple_keyring_set_cancel_requests(keyring_handler,
		gnomekeyring_cancel);
	purple_keyring_set_close_keyring(keyring_handler, gnomekeyring_close);

	purple_keyring_register(keyring_handler);

	return TRUE;
}

static gboolean
plugin_unload(PurplePlugin *plugin, GError **error)
{
	if (purple_keyring_get_inuse() == keyring_handler) {
		g_set_error(error, GNOMEKEYRING_DOMAIN, 0, "The keyring is currently "
			"in use.");
		purple_debug_warning("keyring-gnome",
			"keyring in use, cannot unload\n");
		return FALSE;
	}

	gnomekeyring_close();

	purple_keyring_unregister(keyring_handler);
	purple_keyring_free(keyring_handler);
	keyring_handler = NULL;

	return TRUE;
}

PURPLE_PLUGIN_INIT(gnome_keyring, plugin_query, plugin_load, plugin_unload);
