#ifndef UTIL_H 
#define UTIL_H

#include <gtk/gtk.h>
#include <glade/glade.h>
#include "zenity.h"

G_BEGIN_DECLS

#define ZENITY_GLADE_FILE_FULLPATH              ZENITY_DATADIR "/zenity.glade"
#define ZENITY_GLADE_FILE_RELATIVEPATH          "./zenity.glade"
#define ZENITY_IMAGE_FULLPATH(filename)         (g_strconcat (ZENITY_DATADIR, "/", filename, NULL))

GladeXML*       zenity_util_load_glade_file               (const gchar    *widget_root);
gchar *         zenity_util_strip_newline                 (gchar          *string);
gboolean        zenity_util_fill_file_buffer              (GtkTextBuffer  *buffer, 
                                                           const gchar    *filename);
void            zenity_util_set_window_icon               (GtkWidget      *widget,
                                                           const gchar    *filename);
void            zenity_util_set_window_icon_from_stock    (GtkWidget      *widget,
                                                           const gchar    *stock_id);
gboolean        zenity_util_show_help                     (const gchar    *path,
                                                           const gchar    *document,
                                                           GError        **error);
gint		zenity_util_return_exit_code 		  (ZenityExitCode value);                      
                               
G_END_DECLS

#endif /* UTIL_H */
