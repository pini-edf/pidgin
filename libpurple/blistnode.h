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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#ifndef PURPLE_BLIST_NODE_H
#define PURPLE_BLIST_NODE_H
/**
 * SECTION:blistnode
 * @section_id: libpurple-blistnode
 * @short_description: <filename>blistnode.h</filename>
 * @title: Buddy List Node and Counting Node types
 */

#include <glib.h>
#include <glib-object.h>

#define PURPLE_TYPE_BLIST_NODE             (purple_blist_node_get_type())
#define PURPLE_BLIST_NODE(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), PURPLE_TYPE_BLIST_NODE, PurpleBlistNode))
#define PURPLE_BLIST_NODE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), PURPLE_TYPE_BLIST_NODE, PurpleBlistNodeClass))
#define PURPLE_IS_BLIST_NODE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), PURPLE_TYPE_BLIST_NODE))
#define PURPLE_IS_BLIST_NODE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), PURPLE_TYPE_BLIST_NODE))
#define PURPLE_BLIST_NODE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), PURPLE_TYPE_BLIST_NODE, PurpleBlistNodeClass))

typedef struct _PurpleBlistNode PurpleBlistNode;
typedef struct _PurpleBlistNodeClass PurpleBlistNodeClass;

/**
 * PurpleBlistNode:
 * @prev:    The sibling before this buddy.
 * @next:    The sibling after this buddy.
 * @parent:  The parent of this node.
 * @child:   The child of this node.
 * @ui_data: The UI data associated with this node. This is a convenience
 *           field provided to the UIs -- it is not used by the libpurple core.
 *
 * A Buddy list node.  This can represent a group, a buddy, or anything else.
 * This is a base class for PurpleBuddy, PurpleContact, PurpleGroup, and for
 * anything else that wants to put itself in the buddy list.
 */
struct _PurpleBlistNode {
	GObject gparent;

	/*< public >*/
	PurpleBlistNode *prev;
	PurpleBlistNode *next;
	PurpleBlistNode *parent;
	PurpleBlistNode *child;

	gpointer ui_data;
};

struct _PurpleBlistNodeClass {
	GObjectClass gparent_class;

	/*< private >*/
	void (*_purple_reserved1)(void);
	void (*_purple_reserved2)(void);
	void (*_purple_reserved3)(void);
	void (*_purple_reserved4)(void);
};

G_BEGIN_DECLS

/**
 * purple_blist_node_get_type:
 *
 * Returns: The #GType for the #PurpleBlistNode object.
 */
GType purple_blist_node_get_type(void);

/**
 * purple_blist_node_next:
 * @node:		A node.
 * @offline:	Whether to include nodes for offline accounts
 *
 * Returns the next node of a given node. This function is to be used to iterate
 * over the tree returned by purple_blist_get_buddy_list.
 *
 * See purple_blist_node_get_parent(), purple_blist_node_get_first_child(),
 *   purple_blist_node_get_sibling_next(), purple_blist_node_get_sibling_prev().
 *
 * Returns:	The next node
 */
PurpleBlistNode *purple_blist_node_next(PurpleBlistNode *node, gboolean offline);

/**
 * purple_blist_node_get_parent:
 * @node: A node.
 *
 * Returns the parent node of a given node.
 *
 * See purple_blist_node_get_first_child(), purple_blist_node_get_sibling_next(),
 *     purple_blist_node_get_sibling_prev(), purple_blist_node_next().
 *
 * Returns:  The parent node.
 */
PurpleBlistNode *purple_blist_node_get_parent(PurpleBlistNode *node);

/**
 * purple_blist_node_get_first_child:
 * @node: A node.
 *
 * Returns the the first child node of a given node.
 *
 * See purple_blist_node_get_parent(), purple_blist_node_get_sibling_next(),
 *     purple_blist_node_get_sibling_prev(), purple_blist_node_next().
 *
 * Returns:  The child node.
 */
PurpleBlistNode *purple_blist_node_get_first_child(PurpleBlistNode *node);

/**
 * purple_blist_node_get_sibling_next:
 * @node: A node.
 *
 * Returns the sibling node of a given node.
 *
 * See purple_blist_node_get_parent(), purple_blist_node_get_first_child(),
 *     purple_blist_node_get_sibling_prev(), purple_blist_node_next().
 *
 * Returns:  The sibling node.
 */
PurpleBlistNode *purple_blist_node_get_sibling_next(PurpleBlistNode *node);

/**
 * purple_blist_node_get_sibling_prev:
 * @node: A node.
 *
 * Returns the previous sibling node of a given node.
 *
 * See purple_blist_node_get_parent(), purple_blist_node_get_first_child(),
 *     purple_blist_node_get_sibling_next(), purple_blist_node_next().
 *
 * Returns:  The sibling node.
 */
PurpleBlistNode *purple_blist_node_get_sibling_prev(PurpleBlistNode *node);

/**
 * purple_blist_node_get_ui_data:
 * @node: The node.
 *
 * Returns the UI data of a given node.
 *
 * Returns: The UI data.
 */
gpointer purple_blist_node_get_ui_data(const PurpleBlistNode *node);

/**
 * purple_blist_node_set_ui_data:
 * @node: The node.
 * @ui_data: The UI data.
 *
 * Sets the UI data of a given node.
 */
void purple_blist_node_set_ui_data(PurpleBlistNode *node, gpointer ui_data);

/**
 * purple_blist_node_get_settings:
 * @node:  The node to from which to get settings
 *
 * Returns a node's settings
 *
 * Returns: The hash table with the node's settings
 */
GHashTable *purple_blist_node_get_settings(PurpleBlistNode *node);

/**
 * purple_blist_node_has_setting:
 * @node:  The node to check from which to check settings
 * @key:   The identifier of the data
 *
 * Checks whether a named setting exists for a node in the buddy list
 *
 * Returns: TRUE if a value exists, or FALSE if there is no setting
 */
gboolean purple_blist_node_has_setting(PurpleBlistNode *node, const char *key);

/**
 * purple_blist_node_set_bool:
 * @node:  The node to associate the data with
 * @key:   The identifier for the data
 * @value: The value to set
 *
 * Associates a boolean with a node in the buddy list
 */
void purple_blist_node_set_bool(PurpleBlistNode *node, const char *key, gboolean value);

/**
 * purple_blist_node_get_bool:
 * @node:  The node to retrieve the data from
 * @key:   The identifier of the data
 *
 * Retrieves a named boolean setting from a node in the buddy list
 *
 * Returns: The value, or FALSE if there is no setting
 */
gboolean purple_blist_node_get_bool(PurpleBlistNode *node, const char *key);

/**
 * purple_blist_node_set_int:
 * @node:  The node to associate the data with
 * @key:   The identifier for the data
 * @value: The value to set
 *
 * Associates an integer with a node in the buddy list
 */
void purple_blist_node_set_int(PurpleBlistNode *node, const char *key, int value);

/**
 * purple_blist_node_get_int:
 * @node:  The node to retrieve the data from
 * @key:   The identifier of the data
 *
 * Retrieves a named integer setting from a node in the buddy list
 *
 * Returns: The value, or 0 if there is no setting
 */
int purple_blist_node_get_int(PurpleBlistNode *node, const char *key);

/**
 * purple_blist_node_set_string:
 * @node:  The node to associate the data with
 * @key:   The identifier for the data
 * @value: The value to set
 *
 * Associates a string with a node in the buddy list
 */
void purple_blist_node_set_string(PurpleBlistNode *node, const char *key,
		const char *value);

/**
 * purple_blist_node_get_string:
 * @node:  The node to retrieve the data from
 * @key:   The identifier of the data
 *
 * Retrieves a named string setting from a node in the buddy list
 *
 * Returns: The value, or NULL if there is no setting
 */
const char *purple_blist_node_get_string(PurpleBlistNode *node, const char *key);

/**
 * purple_blist_node_remove_setting:
 * @node:  The node from which to remove the setting
 * @key:   The name of the setting
 *
 * Removes a named setting from a blist node
 */
void purple_blist_node_remove_setting(PurpleBlistNode *node, const char *key);

/**
 * purple_blist_node_set_transient:
 * @node:  The node
 * @transient: TRUE if the node should NOT be saved, FALSE if node should
 *                  be saved
 *
 * Sets whether the node should be saved with the buddy list or not
 */
void purple_blist_node_set_transient(PurpleBlistNode *node, gboolean transient);

/**
 * purple_blist_node_is_transient:
 * @node:  The node
 *
 * Gets whether the node should be saved with the buddy list or not
 *
 * Returns: TRUE if the node should NOT be saved, FALSE if node should be saved
 */
gboolean purple_blist_node_is_transient(PurpleBlistNode *node);

/**
 * purple_blist_node_get_extended_menu:
 * @n: The blist node for which to obtain the extended menu items.
 *
 * Retrieves the extended menu items for a buddy list node.
 *
 * Returns:  A list of PurpleMenuAction items, as harvested by the
 *          blist-node-extended-menu signal.
 */
GList *purple_blist_node_get_extended_menu(PurpleBlistNode *n);

G_END_DECLS

#endif /* PURPLE_BLIST_NODE_H */

