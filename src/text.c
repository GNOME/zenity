/*
 * text.c
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

static ZenityTextData	*zen_text_data;

static void zenity_text_dialog_response (GtkWidget *widget, int response, gpointer data);

void
zenity_text (ZenityData *data, ZenityTextData *text_data)
{
  GladeXML *glade_dialog = NULL;
  GtkWidget *dialog;
  GtkWidget *text_view;
  GtkTextBuffer *text_buffer;

  zen_text_data = text_data;
  glade_dialog = zenity_util_load_glade_file ("zenity_text_dialog");
	
  if (glade_dialog == NULL) {
    data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
    return;
  }
	
  glade_xml_signal_autoconnect (glade_dialog);
	  
  dialog = glade_xml_get_widget (glade_dialog, "zenity_text_dialog");
	
  g_signal_connect (G_OBJECT (dialog), "response",
                    G_CALLBACK (zenity_text_dialog_response), data);
	
  if (data->dialog_title)
    gtk_window_set_title (GTK_WINDOW (dialog), data->dialog_title);

  if (data->window_icon)
    zenity_util_set_window_icon (dialog, data->window_icon);
  else
    zenity_util_set_window_icon (dialog, ZENITY_IMAGE_FULLPATH ("zenity-text.png"));

  gtk_window_set_default_size (GTK_WINDOW (dialog), data->width, data->height);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CLOSE);

  text_buffer = gtk_text_buffer_new (NULL);
  text_view = glade_xml_get_widget (glade_dialog, "zenity_text_view");
  gtk_text_view_set_buffer (GTK_TEXT_VIEW (text_view), text_buffer);
  gtk_text_view_set_editable (GTK_TEXT_VIEW(text_view), text_data->editable);

  if (text_data->uri)
    zenity_util_fill_file_buffer (text_buffer, text_data->uri);

  if (text_data->editable)
    zen_text_data->buffer = text_buffer;

  gtk_widget_show (dialog);

  if (glade_dialog)
    g_object_unref (glade_dialog);

  gtk_main ();
}

static void
zenity_text_dialog_response (GtkWidget *widget, int response, gpointer data)
{
  ZenityData *zen_data = data;

  switch (response) {
    case GTK_RESPONSE_CLOSE:
      if (zen_text_data->editable) {
        GtkTextIter start, end;
				    
        gtk_text_buffer_get_bounds (zen_text_data->buffer, &start, &end);
        g_print (gtk_text_buffer_get_text (zen_text_data->buffer, &start, &end, 0));
      }
      zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
      gtk_main_quit ();
      break;

    default:
      /* Esc dialog */
      zen_data->exit_code = zenity_util_return_exit_code (ZENITY_ESC);
      break;
  }
}
