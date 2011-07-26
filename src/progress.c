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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 * Authors: Glynn Foster <glynn.foster@sun.com>
 */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include "zenity.h"
#include "util.h"

static GtkBuilder *builder;
static ZenityData *zen_data;
static GIOChannel *channel;

static gint pulsate_timeout = -1;
static gboolean autokill;
static gboolean no_cancel;
static gboolean auto_close;

gint zenity_progress_timeout (gpointer data);
gint zenity_progress_pulsate_timeout (gpointer data);

static void zenity_progress_dialog_response (GtkWidget *widget, int response, gpointer data);

static gboolean
zenity_progress_pulsate_progress_bar (gpointer user_data)
{
  gtk_progress_bar_pulse (GTK_PROGRESS_BAR (user_data));
  return TRUE;
}

static void
zenity_progress_pulsate_stop ()
{
  if (pulsate_timeout > 0) {
    g_source_remove (pulsate_timeout);
    pulsate_timeout = -1;
  }
}

static void
zenity_progress_pulsate_start (GObject *progress_bar)
{
  if (pulsate_timeout == -1) {
    pulsate_timeout = g_timeout_add (100,
                                     zenity_progress_pulsate_progress_bar,
                                     progress_bar);
  }
}

static gboolean
zenity_progress_handle_stdin (GIOChannel   *channel,
                              GIOCondition  condition,
                              gpointer      data)
{
  static ZenityProgressData *progress_data;
  static GObject *progress_bar;
  static GObject *progress_label;
  float percentage = 0.0;
  
  progress_data = (ZenityProgressData *) data;
  progress_bar = gtk_builder_get_object (builder, "zenity_progress_bar");
  progress_label = gtk_builder_get_object (builder, "zenity_progress_text");

  if ((condition == G_IO_IN) || (condition == G_IO_IN + G_IO_HUP)) {
    GString *string;
    GError *error = NULL;

    string = g_string_new (NULL);

    if (progress_data->pulsate) {
      zenity_progress_pulsate_start (progress_bar);
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

      } else if (g_str_has_prefix (string->str, "pulsate")) {
        gchar *colon, *command, *value;

        zenity_util_strip_newline (string->str);

        colon = strchr(string->str, ':');
        if (colon == NULL) {
            continue;
        }

        /* split off the command and value */
        command = g_strstrip (g_strndup (string->str, colon - string->str));

        value = colon + 1;
        while (*value && g_ascii_isspace (*value)) value++;

        if (!g_ascii_strcasecmp (value, "false")) {
          zenity_progress_pulsate_stop ();

          gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress_bar),
                                         progress_data->percentage / 100.0);
        } else {
          zenity_progress_pulsate_start (progress_bar);
        }

        g_free (command);
      } else {

        if (!g_ascii_isdigit (*(string->str)))
          continue;

        /* Now try to convert the thing to a number */
        percentage = CLAMP(atoi (string->str), 0, 100);

        gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress_bar),
                                       percentage / 100.0);

        progress_data->percentage = percentage;

        if (percentage == 100) {
          GObject *button;

          button = gtk_builder_get_object(builder, "zenity_progress_ok_button");
          gtk_widget_set_sensitive(GTK_WIDGET (button), TRUE);
          gtk_widget_grab_focus(GTK_WIDGET (button));

          if (progress_data->autoclose) {
            zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
            gtk_main_quit();
          }
        }
      }

    } while (g_io_channel_get_buffer_condition (channel) == G_IO_IN);
    g_string_free (string, TRUE);
  }

  if ((condition != G_IO_IN) && (condition != G_IO_IN + G_IO_HUP)) {
    /* We assume that we are done, so stop the pulsating and de-sensitize the buttons */
    GtkWidget *button;

    button = GTK_WIDGET (gtk_builder_get_object (builder,
                                                 "zenity_progress_ok_button"));
    gtk_widget_set_sensitive (button, TRUE);
    gtk_widget_grab_focus (button);

    button = GTK_WIDGET (gtk_builder_get_object (builder,
                         "zenity_progress_cancel_button"));

    gtk_widget_set_sensitive (button, FALSE);

    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress_bar), 1.0);

    zenity_progress_pulsate_stop ();

    g_object_unref (builder);

    if (progress_data->autoclose) {
      zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
      gtk_main_quit();
    }

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
  GtkWidget *button;
  GObject *text;
  GObject *progress_bar;
  GObject *cancel_button,*ok_button;

  zen_data = data;
  builder = zenity_util_load_ui_file ("zenity_progress_dialog", NULL);

  if (builder == NULL) {
    data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
    return;
  }

  gtk_builder_connect_signals (builder, NULL);

  dialog = GTK_WIDGET (gtk_builder_get_object (builder,
                                               "zenity_progress_dialog"));

  g_signal_connect (G_OBJECT (dialog), "response",
                    G_CALLBACK (zenity_progress_dialog_response), data);

  if (data->dialog_title)
    gtk_window_set_title (GTK_WINDOW (dialog), data->dialog_title);

  zenity_util_set_window_icon (dialog, data->window_icon, ZENITY_IMAGE_FULLPATH ("zenity-progress.png"));

  if (data->width > -1 || data->height > -1)
    gtk_window_set_default_size (GTK_WINDOW (dialog), data->width, data->height);

  if (data->ok_label) {
    button = GTK_WIDGET (gtk_builder_get_object (builder, "zenity_progress_ok_button"));
    gtk_button_set_label (GTK_BUTTON (button), data->ok_label);
    gtk_button_set_image (GTK_BUTTON (button),
                          gtk_image_new_from_stock (GTK_STOCK_OK, GTK_ICON_SIZE_BUTTON));
  }

  if (data->cancel_label) {
    button = GTK_WIDGET (gtk_builder_get_object (builder, "zenity_progress_cancel_button"));
    gtk_button_set_label (GTK_BUTTON (button), data->cancel_label);
    gtk_button_set_image (GTK_BUTTON (button),
                          gtk_image_new_from_stock (GTK_STOCK_CANCEL, GTK_ICON_SIZE_BUTTON));
  }

  text = gtk_builder_get_object (builder, "zenity_progress_text");

  if (progress_data->dialog_text)
    gtk_label_set_markup (GTK_LABEL (text), g_strcompress (progress_data->dialog_text));

  progress_bar = gtk_builder_get_object (builder, "zenity_progress_bar");

  if (progress_data->percentage > -1)
    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress_bar), 
                                   progress_data->percentage/100.0);

  autokill = progress_data->autokill;

  auto_close = progress_data->autoclose;
  ok_button = gtk_builder_get_object (builder, "zenity_progress_ok_button");

  no_cancel = progress_data->no_cancel;
  cancel_button = gtk_builder_get_object (builder, "zenity_progress_cancel_button");

  if (no_cancel) {
     gtk_widget_hide (GTK_WIDGET(cancel_button));
     gtk_window_set_deletable (GTK_WINDOW (dialog), FALSE);
  }

  if (no_cancel && auto_close)
     gtk_widget_hide(GTK_WIDGET(ok_button));

  zenity_util_show_dialog (dialog);
  zenity_progress_read_info (progress_data);

  if(data->timeout_delay > 0) {
    g_timeout_add_seconds (data->timeout_delay, (GSourceFunc) zenity_util_timeout_handle, dialog);
  }

  gtk_main ();
}

static void
zenity_progress_dialog_response (GtkWidget *widget, int response, gpointer data)
{
  switch (response) {
    case GTK_RESPONSE_OK:
      zenity_util_exit_code_with_data(ZENITY_OK, zen_data);
      break;

    case GTK_RESPONSE_CANCEL:
      /* We do not want to kill the parent process, in order to give the user
         the ability to choose the action to be taken. See bug #310824.
         But we want to give people the option to choose this behavior.
         -- Monday 27, March 2006
      */
      if (autokill) {
        kill (getppid (), 1);
      }
      zen_data->exit_code = zenity_util_return_exit_code (ZENITY_CANCEL);
      break;

    default:
      /* Esc dialog */
      zen_data->exit_code = zenity_util_return_exit_code (ZENITY_ESC);
      break;
  }
  gtk_main_quit ();
}
