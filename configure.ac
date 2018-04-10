AC_INIT([Zenity],[3.28.1],[https://bugzilla.gnome.org/enter_bug.cgi?product=zenity],[zenity])

AC_PREREQ([2.63])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.10 foreign no-dist-gzip dist-xz])
AM_MAINTAINER_MODE([enable])

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# Check for programs
AC_PROG_CC

GTK_REQUIRED=3.0.0

PKG_CHECK_MODULES([ZENITY],[gtk+-3.0 >= $GTK_REQUIRED glib-2.0])
AC_SUBST([ZENITY_CFLAGS])
AC_SUBST([ZENITY_LIBS])

dnl *************************
dnl Check for the GTK+ backend, if it's X11, we -lX11
dnl *************************

zenity_save_cflags="$CFLAGS"
# pull in the CFLAGS to locate gdkconfig.h
CFLAGS=`$PKG_CONFIG --cflags gtk+-3.0`
AC_COMPILE_IFELSE([
#include <gdk/gdk.h>

int main(void) {
#ifndef GDK_WINDOWING_X11
#error GDK_WINDOWING_X11 not defined
#endif
    return 0;
}
],
        GDK_BACKEND_X11=yes,
        GDK_BACKEND_X11=no)

CFLAGS="$zenity_save_cflags"

if test "x$GDK_BACKEND_X11" = "xyes"; then
	PKG_CHECK_MODULES(X, x11)
	ZENITY_CFLAGS="$ZENITY_CFLAGS $X_CFLAGS"
	ZENITY_LIBS="$ZENITY_LIBS $X_LIBS"
fi

# *******************************
# perl check
# *******************************

AC_PATH_PROG([PERL],[perl],)

# *******************************
# libnotify check
# *******************************

LIBNOTIFY_REQUIRED=0.6.1

AC_ARG_ENABLE([libnotify],
              [AS_HELP_STRING([--enable-libnotify],
                              [Enable libnotify support])],
              [],
              [enable_libnotify=yes])

if test x$enable_libnotify = xyes; then
        PKG_CHECK_MODULES([LIBNOTIFY], [libnotify >= $LIBNOTIFY_REQUIRED],
                          [HAVE_LIBNOTIFY="yes"],[HAVE_LIBNOTIFY="no"])
        AC_SUBST([LIBNOTIFY_CFLAGS])
        AC_SUBST([LIBNOTIFY_LIBS])
        if test "x$HAVE_LIBNOTIFY" = "xyes"; then
                AC_DEFINE([HAVE_LIBNOTIFY], [1], [libnotify is available on this machine])
        fi
fi

# *******************************
# webkit check
# *******************************

WEBKITGTK_REQUIRED=2.8.1

AC_ARG_ENABLE([webkitgtk],
              [AS_HELP_STRING([--enable-webkitgtk],
                              [Enable webkit support])],
              [],
              [enable_webkitgtk=yes])

if test x$enable_webkitgtk = xyes; then
        PKG_CHECK_MODULES([WEBKIT], [webkit2gtk-4.0 >= $WEBKITGTK_REQUIRED],
                          [HAVE_WEBKITGTK="yes"],[HAVE_WEBKITGTK="no"])
        AC_SUBST([WEBKIT_CFLAGS])
        AC_SUBST([WEBKIT_LIBS])
        if test "x$HAVE_WEBKITGTK" = "xyes"; then
                AC_DEFINE([HAVE_WEBKITGTK], [1], [webkitgtk is available on this machine])
        fi
fi

# *******************************
# Debug
# ******************************* 

GNOME_DEBUG_CHECK
GNOME_COMPILE_WARNINGS([maximum])

# *******************************
# Internationalization
# ******************************* 
AM_GNU_GETTEXT_VERSION([0.19.4])
AM_GNU_GETTEXT([external])

GETTEXT_PACKAGE=zenity
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],["$GETTEXT_PACKAGE"],[Gettext domain])

# *******************************
# User manual
# *******************************
YELP_HELP_INIT

# *******************************

AC_SUBST([AM_CPPFLAGS])
AC_SUBST([AM_CFLAGS])
AC_SUBST([AM_LDFLAGS])

# *******************************

AC_CONFIG_FILES([
Makefile 
po/Makefile.in
src/Makefile
src/gdialog
data/Makefile
help/Makefile
])

AC_OUTPUT
