/*
 * progress.c
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <glade/glade.h>
#include "zenity.h"
#include "util.h"

static guint timer;
static GladeXML *glade_dialog;
static ZenityData *zen_data;
static GIOChannel *channel;

gint zenity_progress_timeout (gpointer data);
gint zenity_progress_pulsate_timeout (gpointer data);

static void zenity_progress_dialog_response (GtkWidget *widget, int response, gpointer data);

static gboolean
zenity_progress_pulsate_progress_bar (gpointer user_data)
{
  gtk_progress_bar_pulse (GTK_PROGRESS_BAR (user_data));
  return TRUE;
}

static gboolean
zenity_progress_handle_stdin (GIOChannel   *channel,
                              GIOCondition  condition,
                              gpointer      data)
{
  static ZenityProgressData *progress_data;
  static GtkWidget *progress_bar;
  static GtkWidget *progress_label;
  static gint pulsate_timeout = -1;
  float percentage = 0.0;
  
  progress_data = (ZenityProgressData *) data;
  progress_bar = glade_xml_get_widget (glade_dialog, "zenity_progress_bar");
  progress_label = glade_xml_get_widget (glade_dialog, "zenity_progress_text");

  if ((condition == G_IO_IN) || (condition == G_IO_IN + G_IO_HUP)) {
    GString *string;
    GError *error = NULL;

    string = g_string_new (NULL);

    if (progress_data->pulsate) {
      if (pulsate_timeout == -1)
        pulsate_timeout = g_timeout_add (100, zenity_progress_pulsate_progress_bar, progress_bar);
    }

    while (channel->is_readable != TRUE)
      ;
    do {
      gint status;

      do {
        status = g_io_channel_read_line_string (channel, string, NULL, &error);

        while (gtk_events_pending ())
          gtk_main_iteration ();

      } while (status == G_IO_STATUS_AGAIN);

      if (status != G_IO_STATUS_NORMAL) {
        if (error) {
          g_warning ("zenity_progress_handle_stdin () : %s", error->message);
          g_error_free (error);
          error = NULL;
        }
        continue;
      }

      if (!g_ascii_strncasecmp (string->str, "#", 1)) {
        gchar *match;

        /* We have a comment, so let's try to change the label */
        match = g_strstr_len (string->str, strlen (string->str), "#");
        match++;
        gtk_label_set_text (GTK_LABEL (progress_label), g_strcompress(g_strchomp (g_strchug (match))));
      } else {

        if (!g_ascii_isdigit (*(string->str)))
          continue;

        /* Now try to convert the thing to a number */
        percentage = atoi (string->str);
        if (percentage >= 100) {
	  GtkWidget *button;
	  button = glade_xml_get_widget( glade_dialog,"zenity_progress_ok_button");
          gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress_bar), 1.0);
	  gtk_widget_set_sensitive(GTK_WIDGET (button), TRUE);
	  gtk_widget_grab_focus(GTK_WIDGET (button));
	  if (progress_data->autoclose) {
		zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
		gtk_main_quit();
		
	  }
	} else
          gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress_bar), percentage / 100.0);
      }

    } while (g_io_channel_get_buffer_condition (channel) == G_IO_IN);
    g_string_free (string, TRUE);
  }

  if (condition != G_IO_IN) {
    /* We assume that we are done, so stop the pulsating and de-sensitize the buttons */
    GtkWidget *button;
    GtkWidget *progress_bar;

    button = glade_xml_get_widget (glade_dialog, "zenity_progress_ok_button");
    gtk_widget_set_sensitive (button, TRUE);
    gtk_widget_grab_focus (button);

    button = glade_xml_get_widget (glade_dialog, "zenity_progress_cancel_button");
    gtk_widget_set_sensitive (button, FALSE);
		
    progress_bar = glade_xml_get_widget (glade_dialog, "zenity_progress_bar");
    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress_bar), 1.0);

    if (progress_data->pulsate) {
      g_source_remove (pulsate_timeout);
      pulsate_timeout = -1;
    }

    if (glade_dialog)
      g_object_unref (glade_dialog);

    g_io_channel_shutdown (channel, TRUE, NULL);
    return FALSE;
  }
  return TRUE;
}

static void
zenity_progress_read_info (ZenityProgressData *progress_data)
{
  channel = g_io_channel_unix_new (0);
  g_io_channel_set_encoding (channel, NULL, NULL);
  g_io_channel_set_flags (channel, G_IO_FLAG_NONBLOCK, NULL);
  g_io_add_watch (channel, G_IO_IN | G_IO_HUP, zenity_progress_handle_stdin, progress_data);
}

void
zenity_progress (ZenityData *data, ZenityProgressData *progress_data)
{
  GtkWidget *dialog;
  GtkWidget *text;
  GtkWidget *progress_bar;

  zen_data = data;
  glade_dialog = zenity_util_load_glade_file ("zenity_progress_dialog");

  if (glade_dialog == NULL) {
    data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
    return;
  }
	
  glade_xml_signal_autoconnect (glade_dialog);
	
  dialog = glade_xml_get_widget (glade_dialog, "zenity_progress_dialog");

  g_signal_connect (G_OBJECT (dialog), "response",
                    G_CALLBACK (zenity_progress_dialog_response), data);
	
  if (data->dialog_title)
    gtk_window_set_title (GTK_WINDOW (dialog), data->dialog_title);

  if (data->window_icon)
    zenity_util_set_window_icon (dialog, data->window_icon);
  else
    zenity_util_set_window_icon (dialog, ZENITY_IMAGE_FULLPATH ("zenity-progress.png"));

  if (data->width > -1 || data->height > -1)
    gtk_window_set_default_size (GTK_WINDOW (dialog), data->width, data->height);

  text = glade_xml_get_widget (glade_dialog, "zenity_progress_text");
  gtk_label_set_text (GTK_LABEL (text), progress_data->dialog_text);

  progress_bar = glade_xml_get_widget (glade_dialog, "zenity_progress_bar");

  if (progress_data->percentage > -1)
    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress_bar), 
                                   progress_data->percentage/100.0);

  zenity_util_show_dialog (dialog);
  zenity_progress_read_info (progress_data);

  gtk_main ();
}

static void
zenity_progress_dialog_response (GtkWidget *widget, int response, gpointer data)
{
  switch (response) {
    case GTK_RESPONSE_OK:
      zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
      break;
		
    case GTK_RESPONSE_CANCEL:
      /* FIXME: This should kill off the parent process nicely and return an error code
       * I'm pretty sure there is a nice way to do this, but I'm clueless about this
       * stuff. Should be using SIGHUP instead of 1 though.
       */
      kill (getppid (), 1);
      zen_data->exit_code = zenity_util_return_exit_code (ZENITY_CANCEL);
      break;
  
    default:
      /* Esc dialog */
      zen_data->exit_code = zenity_util_return_exit_code (ZENITY_ESC);
      break;
  }
  gtk_main_quit ();
}
