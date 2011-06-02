/*
 * notification.c
 *
 * Copyright (C) 2002 Sun Microsystems, Inc.
 * Copyright (C) 2006 Christian Persch
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

#include <config.h>

#include <unistd.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <time.h>
#include <string.h>
#ifdef HAVE_LIBNOTIFY
#include <libnotify/notify.h>

#include "zenity.h"
#include "util.h"

static char *icon_file;

static void
on_notification_default_action (NotifyNotification *n,
                                const char         *action,
                                void               *user_data)
{
  ZenityData *zen_data;

  zen_data = (ZenityData *)user_data;
  notify_notification_close (n, NULL);

  zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);

  gtk_main_quit ();
}

static gboolean
zenity_notification_handle_stdin (GIOChannel *channel,
				  GIOCondition condition,
				  gpointer user_data)
{
  ZenityData *zen_data;

  zen_data = (ZenityData *)user_data;

  if ((condition & G_IO_IN) != 0) {
    GString *string;
    GError *error = NULL;

    string = g_string_new (NULL);
    while (channel->is_readable == FALSE)
      ;
    do {
      gint status;
      gchar *command, *value, *colon;

      do {
	status = g_io_channel_read_line_string (channel, string, NULL, &error);
	while (gdk_events_pending ())
	  gtk_main_iteration ();

      } while (status == G_IO_STATUS_AGAIN);

      if (status  != G_IO_STATUS_NORMAL) {
	if (error) {
	  g_warning ("zenity_notification_handle_stdin () : %s",
		     error->message);
	  g_error_free (error);
	  error = NULL;
	}
	continue;
      }

      zenity_util_strip_newline (string->str);
      colon = strchr(string->str, ':');
      if (colon == NULL) {
	g_printerr (_("could not parse command from stdin\n"));
	continue;
      }
      /* split off the command and value */
      command = g_strstrip (g_strndup (string->str, colon - string->str));

      value = colon + 1;
      while (*value && g_ascii_isspace (*value)) value++;

      if (!g_ascii_strcasecmp (command, "icon")) {
        g_free (icon_file);
        icon_file = g_strdup (value);
      } else if (!g_ascii_strcasecmp (command, "message")) {
        /* display a notification bubble */
        if (!g_utf8_validate (value, -1, NULL)) {
          g_warning ("Invalid UTF-8 in input!");
        } else {
          NotifyNotification *notif;
          gchar **message;
          error = NULL;

          /* message[1] (the summary) will be NULL in case there's
           * no \n in the string. In which case only the title is
           * defined */
          message = g_strsplit (g_strcompress (value), "\n", 2);

          if (*message == NULL) {
            g_printerr (_("Could not parse message from stdin\n"));
            continue;
          }

          notif = notify_notification_new (message[0] /* title */,
                                           message[1] /* summary */,
                                           icon_file);

          g_strfreev (message);

          notify_notification_show (notif, &error);
          if (error) {
            g_warning ("Error showing notification: %s", error->message);
            g_error_free (error);
            error = NULL;
          }

	  g_object_unref (notif);
        }
      } else if (!g_ascii_strcasecmp (command, "tooltip")) {
        if (!g_utf8_validate (value, -1, NULL)) {
          g_warning ("Invalid UTF-8 in input!");
        } else {
          NotifyNotification *notif;

          notif = notify_notification_new (value,
                                           NULL,
                                           icon_file);
          notify_notification_show (notif, &error);
          if (error) {
            g_warning ("Error showing notification: %s", error->message);
            g_error_free (error);
            error = NULL;
          }
        }
      } else if (!g_ascii_strcasecmp (command, "visible")) {

      } else {
	g_warning ("Unknown command '%s'", command);
      }
      g_free (command);

    } while (g_io_channel_get_buffer_condition (channel) == G_IO_IN);
    g_string_free (string, TRUE);
  }

  if ((condition & G_IO_HUP) != 0) {
    g_io_channel_shutdown (channel, TRUE, NULL);
    zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
    gtk_main_quit ();
    return FALSE;
  }

  return TRUE;
}

static void
zenity_notification_listen_on_stdin (ZenityData *data)
{
  GIOChannel *channel;

  channel = g_io_channel_unix_new (0);
  g_io_channel_set_encoding (channel, NULL, NULL);
  g_io_channel_set_flags (channel, G_IO_FLAG_NONBLOCK, NULL);
  g_io_add_watch (channel, G_IO_IN | G_IO_HUP,
		  zenity_notification_handle_stdin, data);
}

void
zenity_notification (ZenityData *data, ZenityNotificationData *notification_data)
{
  GError *error;
  NotifyNotification *notification;

  /* create the notification widget */
  if (!notify_is_initted ()) {
    notify_init (_("Zenity notification"));
  }

  if (notification_data->listen) {
    zenity_notification_listen_on_stdin (data);
  } else {
    if (notification_data->notification_text == NULL) {
      exit (1);
    }

    notification = notify_notification_new (notification_data->notification_text, NULL, data->window_icon);
    if (notification == NULL) {
      exit (1);
    }

    /* if we aren't listening for changes, then close on default action */
    notify_notification_add_action (notification,
                                    "default",
                                    "Do Default Action",
                                    (NotifyActionCallback) on_notification_default_action,
                                    data,
                                    NULL);

    /* Show icon and wait */
    error = NULL;
    if (!notify_notification_show (notification, &error)) {
      if (error != NULL) {
        g_warning ("Error showing notification: %s", error->message);
        g_error_free (error);
      }
      exit (1);
    }

  }

  if (data->timeout_delay > 0) {
    g_timeout_add_seconds (data->timeout_delay, (GSourceFunc) zenity_util_timeout_handle, NULL);
  }

  gtk_main ();
}
#endif
