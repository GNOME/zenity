#ifndef UTIL_H 
#define UTIL_H

#include <gtk/gtk.h>
#include <glade/glade.h>

G_BEGIN_DECLS

#define ZENITY_GLADE_FILE_FULLPATH		ZENITY_DATADIR "/zenity.glade"
#define ZENITY_GLADE_FILE_RELATIVEPATH		"./zenity.glade"
#define ZENITY_IMAGE_FULLPATH(filename)		(g_strconcat (ZENITY_DATADIR, "/", filename, NULL))

GladeXML*	zenity_util_load_glade_file 		(const gchar *widget_root);

gboolean	zenity_util_fill_file_buffer 		(GtkTextBuffer *buffer, 
					      		 const gchar   *filename);

void		zenity_util_set_window_icon 		(GtkWidget	*widget,
					     		 const gchar	*filename);

void		zenity_util_set_window_icon_from_stock (GtkWidget	*widget,
							const gchar	*stock_id);

G_END_DECLS

#endif /* UTIL_H */
