bin_PROGRAMS = zenity

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
	about.c		\
	util.h		\
	util.c

INCLUDES = \
	$(ZENITY_CFLAGS)				\
	-I$(includedir)					\
	-DGNOMELOCALEDIR=\""$(zenitylocaledir)"\"	\
	-DZENITY_DATADIR=\""$(datadir)/zenity"\"

zenity_LDADD = \
	$(ZENITY_LIBS)

gladedir = $(datadir)/zenity

glade_DATA = \
	zenity.glade
