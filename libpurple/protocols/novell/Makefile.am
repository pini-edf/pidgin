EXTRA_DIST = \
	Makefile.mingw

pkgdir = @PURPLE_PLUGINDIR@

NOVELLSOURCES = \
		nmfield.h \
		nmfield.c \
		nmconn.h \
		nmconn.c \
		nmconference.h \
		nmconference.c \
		nmcontact.h \
		nmcontact.c \
		nmevent.h \
		nmevent.c \
		nmmessage.h \
		nmmessage.c \
		nmrequest.h \
		nmrequest.c \
		nmrtf.h \
		nmrtf.c \
		nmuser.h \
		nmuser.c \
		nmuserrecord.h \
		nmuserrecord.c \
		novell.h \
		novell.c

AM_CFLAGS = $(st)

libnovell_la_LDFLAGS = -module @PLUGIN_LDFLAGS@

if STATIC_NOVELL

st = -DPURPLE_STATIC_PRPL
noinst_LTLIBRARIES   = libnovell.la
libnovell_la_SOURCES = $(NOVELLSOURCES)
libnovell_la_CFLAGS  = $(AM_CFLAGS)

else

st =
pkg_LTLIBRARIES      = libnovell.la
libnovell_la_SOURCES = $(NOVELLSOURCES)
libnovell_la_LIBADD  = @PURPLE_LIBS@

endif

AM_CPPFLAGS = \
	-I$(top_srcdir)/libpurple \
	-I$(top_builddir)/libpurple \
	$(DEBUG_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GPLUGIN_CFLAGS)
