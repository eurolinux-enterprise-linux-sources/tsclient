
desktopdir = $(datadir)/applications
desktop_in_files = tsclient.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

autostartdir = $(datadir)/gnome/autostart
autostart_DATA = tsc-autostart.desktop

tscdir = $(datadir)/tsclient
tsc_DATA = tsclient.glade \
	tsclient.xml

icondir = $(datadir)/icons/hicolor/scalable/apps
icon_DATA = tsclient.svg

schemadir = $(GCONF_SCHEMA_FILE_DIR)
schema_DATA = tsc-handlers.schemas

@INTLTOOL_DESKTOP_RULE@

if GCONF_SCHEMAS_INSTALL
install-data-local:
	if [ -z "$(DESTDIR)" ]; then \
		GCONF_CONFIG_SOURCE="" $(GCONFTOOL) --makefile-install-rule $(schema_DATA); \
	fi
endif

#CLEANFILES = $(desktop_DATA)

EXTRA_DIST =				\
	$(tsc_DATA)			\
	$(icon_DATA)			\
	$(desktop_in_files)		\
	$(schema_DATA)			\
	tsc-autostart.desktop		\
	tsclient.xml
