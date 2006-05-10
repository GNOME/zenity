bin_PROGRAMS = zenity

bin_SCRIPTS = gdialog

zenity_SOURCES = \
	main.c		\
	option.c	\
	option.h	\
	zenity.h	\
	calendar.c	\
	msg.c		\
	scale.c		\
	fileselection.c	\
	entry.c		\
	text.c		\
	progress.c	\
	tree.c		\
	eggtrayicon.h	\
	eggtrayicon.c	\
	notification.c	\
	about.c		\
	util.h		\
	util.c

zenity_CPPFLAGS = \
	-I$(includedir)					\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-DZENITY_DATADIR=\""$(pkgdatadir)"\"		\
	$(AM_CPPFLAGS)

zenity_CFLAGS = \
	$(ZENITY_CFLAGS)	\
	$(LIBNOTIFY_CFLAGS)	\
	$(WARN_CFLAGS)		\
	$(AM_CFLAGS)

zenity_LDFLAGS = \
	$(AM_LDFLAGS)

zenity_LDADD = \
	$(ZENITY_LIBS)		\
	$(LIBNOTIFY_LIBS)	\
	$(X_LIBS)

gladedir = $(datadir)/zenity

glade_DATA = \
	zenity.glade

DISTCLEANFILES= \
	gdialog

EXTRA_DIST = \
	$(glade_DATA) \
	gdialog \
	gdialog.in
