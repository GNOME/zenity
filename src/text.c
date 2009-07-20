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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 * Authors: Glynn Foster <glynn.foster@sun.com>
 */

#include "config.h"

#include "zenity.h"
#include "util.h"

static ZenityTextData	*zen_text_data;

static void zenity_text_dialog_response (GtkWidget *widget, int response, gpointer data);

static gboolean
zenity_text_handle_stdin (GIOChannel  *channel,
                          GIOCondition condition,
                          gpointer     data)
{
  static GtkTextBuffer *buffer;
  gchar buf[1024];

  gsize len;

  buffer = GTK_TEXT_BUFFER (data);

  if ((condition & G_IO_IN) || (condition & (G_IO_IN | G_IO_HUP))) {
    GError *error = NULL;
    gint status;

    while (channel->is_readable != TRUE)
      ;

    do {
      status = g_io_channel_read_chars (channel, buf, 1024, &len, &error);

      while (gtk_events_pending ())
        gtk_main_iteration ();

    } while (status == G_IO_STATUS_AGAIN);

    if (status != G_IO_STATUS_NORMAL) {
      if (error) {
        g_warning ("zenity_text_handle_stdin () : %s", error->message);
        g_error_free (error);
        error = NULL;
      }
      return FALSE;
    }

    if (len > 0) {
      GtkTextIter end;
      gchar *utftext; 
      gsize localelen; 
      gsize utflen;

      gtk_text_buffer_get_end_iter (buffer, &end);

      if (!g_utf8_validate (buf, len, NULL)) {
        utftext = g_convert_with_fallback (buf, len, "UTF-8", "ISO-8859-1", NULL, &localelen, &utflen, NULL);
        gtk_text_buffer_insert (buffer, &end, utftext, utflen);
        g_free (utftext);
      } else {
        gtk_text_buffer_insert (buffer, &end, buf, len);
      }
    }
  }

  return TRUE;
}

static void
zenity_text_fill_entries_from_stdin (GtkTextBuffer *text_buffer)
{
  GIOChannel *channel; 

  channel = g_io_channel_unix_new (0);
  g_io_channel_set_encoding (channel, NULL, NULL);
  g_io_channel_set_flags (channel, G_IO_FLAG_NONBLOCK, NULL);
  g_io_add_watch (channel, G_IO_IN | G_IO_HUP, zenity_text_handle_stdin, text_buffer);
}

void
zenity_text (ZenityData *data, ZenityTextData *text_data)
{
  GtkBuilder *builder;
  GtkWidget *dialog;
  GObject *text_view;
  GtkTextBuffer *text_buffer;

  zen_text_data = text_data;
  builder = zenity_util_load_ui_file ("zenity_text_dialog",
  				      "textbuffer1", NULL);
	
  if (builder == NULL) {
    data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
    return;
  }
	
  gtk_builder_connect_signals (builder, NULL);
	  
  dialog = GTK_WIDGET (gtk_builder_get_object (builder, "zenity_text_dialog"));
	
  g_signal_connect (G_OBJECT (dialog), "response",
                    G_CALLBACK (zenity_text_dialog_response), data);
	
  if (data->dialog_title)
    gtk_window_set_title (GTK_WINDOW (dialog), data->dialog_title);

  zenity_util_set_window_icon (dialog, data->window_icon, ZENITY_IMAGE_FULLPATH ("zenity-text.png"));

  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CLOSE);

  text_buffer = gtk_text_buffer_new (NULL);
  text_view = gtk_builder_get_object (builder, "zenity_text_view");
  gtk_text_view_set_buffer (GTK_TEXT_VIEW (text_view), text_buffer);
  gtk_text_view_set_editable (GTK_TEXT_VIEW(text_view), text_data->editable);

  if (text_data->uri)
    zenity_util_fill_file_buffer (text_buffer, text_data->uri);
  else
    zenity_text_fill_entries_from_stdin (GTK_TEXT_BUFFER (text_buffer));

  if (text_data->editable)
    zen_text_data->buffer = text_buffer;

  if (data->width > -1 || data->height > -1)
    gtk_window_set_default_size (GTK_WINDOW (dialog), data->width, data->height);
  else
    gtk_window_set_default_size (GTK_WINDOW (dialog), 300, 400); 

  zenity_util_show_dialog (dialog);

  g_object_unref (builder);

  if(data->timeout_delay > 0) {
    g_timeout_add (data->timeout_delay * 1000, (GSourceFunc) zenity_util_timeout_handle, NULL);
  }

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
        gchar *text;
				    
        gtk_text_buffer_get_bounds (zen_text_data->buffer, &start, &end);
        text = gtk_text_buffer_get_text (zen_text_data->buffer, &start, &end, 0);
        g_print ("%s", text);
        g_free (text);
      }
      zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
      break;

    default:
      /* Esc dialog */
      zen_data->exit_code = zenity_util_return_exit_code (ZENITY_ESC);
      break;
  }
  gtk_main_quit ();
}
