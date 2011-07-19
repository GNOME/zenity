bin_PROGRAMS = zenity

bin_SCRIPTS = gdialog

zenity_SOURCES = \
	about.c		\
	calendar.c	\
	entry.c		\
	fileselection.c	\
	main.c		\
	msg.c		\
	notification.c	\
	option.c	\
	option.h	\
	progress.c	\
	scale.c		\
	text.c		\
	tree.c		\
	color.c         \
	password.c      \
	util.c		\
	util.h		\
        forms.c		\
	zenity.h

zenity_CPPFLAGS = \
	-I$(includedir)					\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-DZENITY_DATADIR=\""$(pkgdatadir)"\"		\
	$(AM_CPPFLAGS)

zenity_CFLAGS = \
	$(ZENITY_CFLAGS)	\
	$(LIBNOTIFY_CFLAGS)	\
	$(WEBKIT_CFLAGS)	\
	$(WARN_CFLAGS)		\
	$(AM_CFLAGS)

zenity_LDFLAGS = \
	$(AM_LDFLAGS)

zenity_LDADD = \
	$(ZENITY_LIBS)		\
	$(LIBNOTIFY_LIBS)	\
	$(WEBKIT_LIBS)

uidir = $(datadir)/zenity

ui_DATA = \
	zenity.ui

DISTCLEANFILES= \
	gdialog

EXTRA_DIST = \
	$(ui_DATA) \
	gdialog \
	gdialog.in
