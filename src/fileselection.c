/*
 * fileselection.c
 *
 * Copyright (C) 2002 Sun Microsystems, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors: Glynn Foster <glynn.foster@sun.com>
 */

#include <glade/glade.h>
#include "zenity.h"
#include "util.h"

ZenityData	*zen_data;

static void zenity_fileselection_dialog_response (GtkWidget *widget, int response, gpointer data);

void zenity_fileselection (ZenityData *data, ZenityFileData *file_data)
{
  GladeXML *glade_dialog;
  GtkWidget *dialog;

  zen_data = data;

  glade_dialog = zenity_util_load_glade_file ("zenity_fileselection_dialog");

  if (glade_dialog == NULL) {
    data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
    return;
  }
	
  glade_xml_signal_autoconnect (glade_dialog);
	
  dialog = glade_xml_get_widget (glade_dialog, "zenity_fileselection_dialog");
	
  if (glade_dialog)
    g_object_unref (glade_dialog);

  g_signal_connect (G_OBJECT (dialog), "response", 
                    G_CALLBACK (zenity_fileselection_dialog_response), file_data);

  if (data->dialog_title)
    gtk_window_set_title (GTK_WINDOW (dialog), data->dialog_title);
	
  if (data->window_icon)
    zenity_util_set_window_icon (dialog, data->window_icon);
  else
    zenity_util_set_window_icon (dialog, ZENITY_IMAGE_FULLPATH ("zenity-file.png"));

  gtk_window_set_default_size (GTK_WINDOW (dialog), data->width, data->height);

  if (file_data->uri)
    gtk_file_selection_set_filename (GTK_FILE_SELECTION (dialog), file_data->uri);

  if (file_data->multi)
    gtk_file_selection_set_select_multiple (GTK_FILE_SELECTION (dialog), TRUE);

  gtk_widget_show (dialog);
  gtk_main ();
}

static void
zenity_fileselection_dialog_response (GtkWidget *widget, int response, gpointer data)
{
  ZenityFileData *file_data = data;
  gchar **selections;
  gchar *separator = g_strdup(file_data->separator);
  int i;
	  
  switch (response) {
    case GTK_RESPONSE_OK:
      zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);		
      selections = gtk_file_selection_get_selections (GTK_FILE_SELECTION (widget));
      for (i=0;selections[i] != NULL; i++) {
        g_print ("%s", g_filename_to_utf8 ((gchar*)selections[i], -1, NULL, NULL, NULL));
	if (selections[i+1] != NULL)
	    g_print ("%s",separator);
      }
      g_print("\n");
      g_strfreev(selections);
      g_free(separator);

      gtk_main_quit ();
      break;

    case GTK_RESPONSE_CANCEL:
      zen_data->exit_code = zenity_util_return_exit_code (ZENITY_CANCEL);
      gtk_main_quit ();
      break;

    default:
      /* Esc dialog */
      zen_data->exit_code = zenity_util_return_exit_code (ZENITY_ESC);
      break;
  }
}
