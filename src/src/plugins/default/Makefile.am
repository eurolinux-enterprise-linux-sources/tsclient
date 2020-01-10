
INCLUDES =                      \
	-DGLADEFILE=\""$(datadir)/tsclient/tsclient.glade"\"          \
        -DSYSCONFDIR=\""$(sysconfdir)"\"        \
        -DLIBDIR=\""$(libdir)"\"        \
        -DPLUGINDIR=\""$(libdir)/tsclient/plugins"\"   \
        -I$(top_srcdir)         \
        -I$(top_srcdir)/src      \
        $(TSC_CFLAGS)


tsc_plugindir = $(libdir)/tsclient/plugins
tsc_plugin_LTLIBRARIES = libtsc-default.la

tsc_scriptdir = $(libdir)/tsclient
tsc_script_SCRIPTS = tsc-ssh-askpass 

libtsc_default_la_SOURCES =		\
	tsc-default.c			\
	tsc-generic-connection.c	\
	tsc-generic-connection.h	\
	tsc-generic-edit-dialog.c	\
	tsc-generic-edit-dialog.h	\
	tsc-generic-provider.c		\
	tsc-generic-provider.h		\
	tsc-rdp-connection.c		\
	tsc-rdp-connection.h		\
	tsc-rdp-edit-dialog.c		\
	tsc-rdp-edit-dialog.h		\
	tsc-rdp-provider.c		\
	tsc-rdp-provider.h		\
	tsc-mainframe-connection.c	\
	tsc-mainframe-connection.h	\
	tsc-mainframe-edit-dialog.c	\
	tsc-mainframe-edit-dialog.h	\
	tsc-mainframe-provider.c	\
	tsc-mainframe-provider.h	\
	tsc-ica-provider.c		\
	tsc-ica-provider.h		\
	tsc-ica-connection.c		\
	tsc-ica-connection.h		\
	tsc-xdmcp-connection.c		\
	tsc-xdmcp-connection.h		\
	tsc-xdmcp-edit-dialog.c		\
	tsc-xdmcp-edit-dialog.h		\
	tsc-xdmcp-provider.c		\
	tsc-xdmcp-provider.h		\
	tsc-ssh-connection.c		\
	tsc-ssh-connection.h		\
	tsc-ssh-edit-dialog.c		\
	tsc-ssh-edit-dialog.h		\
	tsc-ssh-provider.c			\
	tsc-ssh-provider.h			\
	tsc-web-connection.c		\
	tsc-web-connection.h		\
	tsc-web-edit-dialog.c		\
	tsc-web-edit-dialog.h		\
	tsc-web-provider.c			\
	tsc-web-provider.h			\
	tsc-vnc-connection.c		\
	tsc-vnc-connection.h		\
	tsc-vnc-edit-dialog.c		\
	tsc-vnc-edit-dialog.h		\
	tsc-vnc-provider.c			\
	tsc-vnc-provider.h

libtsc_default_la_LDFLAGS = -module -avoid-version

EXTRA_DIST = tsc-ssh-askpass
