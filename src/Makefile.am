bin_PROGRAMS = zenity

bin_SCRIPTS = gdialog

zenity_SOURCES = \
	main.c		\
	zenity.h	\
	calendar.c	\
	msg.c		\
	fileselection.c	\
	entry.c		\
	text.c		\
	progress.c	\
	tree.c		\
	notification.c	\
	eggtrayicon.c	\
	eggtrayicon.h	\
	about.c		\
	util.h		\
	util.c

INCLUDES = \
	$(ZENITY_CFLAGS)				\
	-I$(includedir)					\
	-DGNOMELOCALEDIR=\""$(zenitylocaledir)"\"	\
	-DZENITY_DATADIR=\""$(datadir)/zenity"\"

zenity_LDADD = \
	$(ZENITY_LIBS)		\
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
