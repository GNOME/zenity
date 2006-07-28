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
	util.c		\
	util.h		\
	zenity.h

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
