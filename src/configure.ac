### init & version stuff

AC_INIT(src/tsc-main.c)
AC_PREREQ(2.52)

MAJOR_VERSION=2
MINOR_VERSION=0
MICRO_VERSION=2
VERSION=$MAJOR_VERSION.$MINOR_VERSION.$MICRO_VERSION

AM_INIT_AUTOMAKE(tsclient, $VERSION)

AC_SUBST(MAJOR_VERSION)
AC_SUBST(MINOR_VERSION)
AC_SUBST(MICRO_VERSION)

AC_DEFINE_UNQUOTED(MAJOR_VERSION, $MAJOR_VERSION, [Major Version Number])
AC_DEFINE_UNQUOTED(MINOR_VERSION, $MINOR_VERSION, [Minor Version Number])
AC_DEFINE_UNQUOTED(MICRO_VERSION, $MICRO_VERSION, [Micro Version Number])

AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

### basic checks

AC_PROG_CC
AC_ISC_POSIX
AC_STDC_HEADERS
AM_PROG_CC_STDC
AC_HEADER_STDC
AM_PROG_LIBTOOL

### get cflags and libs

PKG_CHECK_MODULES(TSC, glib-2.0 gmodule-2.0 gobject-2.0 gtk+-2.0 libglade-2.0 libgnome-2.0 gnome-desktop-2.0 gnome-vfs-2.0 libnotify gconf-2.0 libnm_glib)
TSC_CFLAGS="-Wall $TSC_CFLAGS"

### Internationalization support
AC_PROG_INTLTOOL([0.33])

GETTEXT_PACKAGE=tsclient
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext package])

ALL_LINGUAS="de es fr it ja ko nl zh_CN zh_TW"
AM_GLIB_GNU_GETTEXT

# AM_GLIB_GNU_GETTEXT above subst. $DATADIRNAME
# this is the directory where the *.{mo,gmo} files are installed
localedir='${prefix}/${DATADIRNAME}/locale'
AC_SUBST(localedir)

dnl GConf
AC_PATH_PROG(GCONFTOOL, gconftool-2, no)
AM_GCONF_SOURCE_2

### build makefiles

AC_OUTPUT([
Makefile
data/Makefile
src/Makefile
src/plugins/Makefile
src/plugins/default/Makefile
po/Makefile.in
])
