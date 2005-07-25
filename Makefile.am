#
# FIXME:
#  currently scrollkeeper-update does not seem to
#  remove its database when there is no useful
#  information contained there. Therefore, with
#  "make uninstall" you're left with loads of
#  stuff in /var/scrollkeeper and distcheck
#  fails.
#
#  http://mail.gnome.org/archives/gnome-doc-list/2003-May/msg00008.html
#
#  This overrides the uninstall check so distcheck
#  passes
#
# distuninstallcheck_listfiles = find . -type f -print | grep -v 'scrollkeeper' | grep -v 'omf' | grep -v 'figures'
distuninstallcheck:
	@:
                                                                                                                                                             
distcleancheck_listfiles = find . -type f -print | grep -v 'omf\.out' | grep -v 'legal.xml'

SUBDIRS = \
	src	\
	po	\
	data	\
	help

EXTRA_DIST = \
	intltool-extract.in	\
	intltool-merge.in	\
	intltool-update.in	\
	zenity.spec		\
	zenity.spec.in		\
	omf.make		\
	xmldocs.make		\
	COPYING			\
	HACKING			\
	ChangeLog		\
	NEWS			\
	README			\
	AUTHORS			\
	gnome-doc-utils.make

DISTCLEANFILES =                \
	intltool-extract        \
	intltool-merge          \
	intltool-update		\
	gnome-doc-utils.make

DISTCHECK_CONFIGURE_FLAGS = --disable-scrollkeeper
