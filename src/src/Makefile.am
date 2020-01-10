
SUBDIRS = plugins

INCLUDES =								\
	-DDATADIR=\""$(datadir)"\"					\
	-DGLADEFILE=\""$(datadir)/tsclient/tsclient.glade"\"		\
	-DUIFILE=\""$(datadir)/tsclient/tsclient.xml"\"		\
	-DSYSCONFDIR=\""$(sysconfdir)"\"				\
	-DPLUGINDIR=\""$(libdir)/tsclient/plugins"\"	\
	-I$(top_srcdir)							\
	$(TSC_CFLAGS)

bin_PROGRAMS = tsclient

BUILT_SOURCES = tsc-marshal.c tsc-marshal.h

tscincludedir = $(includedir)/tsclient
tscinclude_HEADERS =				\
	tsc-connection.h			\
	tsc-connection-model.h			\
	tsc-edit-dialog.h			\
	tsc-manager.h				\
	tsc-marshal.h				\
	tsc-network.h				\
	tsc-provider.h				\
	tsc-spawn.h				\
	tsc-util.h				\
	tsc-bookmarkfile.h			\
	tsc-window.h

tsclient_SOURCES =				\
	$(tscinclude_HEADERS)			\
	tsc-connection.c			\
	tsc-connection-model.c			\
	tsc-edit-dialog.c			\
	tsc-main.c				\
	tsc-marshal.c				\
	tsc-manager.c				\
	tsc-network.c				\
	tsc-provider.c				\
	tsc-spawn.c				\
	tsc-util.c				\
	tsc-bookmarkfile.c			\
	tsc-window.c

tsclient_LDADD = $(TSC_LIBS)

tsc-marshal.h: tsc-marshal.list
	glib-genmarshal --prefix=tsc_marshal $(srcdir)/tsc-marshal.list --header > $(srcdir)/$@

tsc-marshal.c: tsc-marshal.list
	glib-genmarshal --prefix=tsc_marshal $(srcdir)/tsc-marshal.list --body > $(srcdir)/$@

EXTRA_DIST = tsc-marshal.list
