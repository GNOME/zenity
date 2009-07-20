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

#include <gtk/gtk.h>
#include <time.h>
#include <string.h>

#ifdef HAVE_LIBNOTIFY
#include <libnotify/notify.h>
#endif

#include "zenity.h"
#include "util.h"

static GtkStatusIcon *status_icon;
static gchar *icon_file;
static const gchar *icon_stock;
static gint icon_size;

static void
zenity_notification_icon_update (void)
{
  GdkPixbuf *pixbuf;
  GError *error = NULL;

  pixbuf = gdk_pixbuf_new_from_file_at_scale (icon_file, icon_size, icon_size, TRUE, &error);

  if (error) {
    g_warning ("Could not load notification icon '%s': %s",
               icon_file, error->message);
    g_clear_error (&error);
  }
  if (!pixbuf) {
    pixbuf = gdk_pixbuf_new_from_file_at_scale (ZENITY_IMAGE_FULLPATH ("zenity-notification.png"),
                                                icon_size, icon_size, TRUE, NULL);
  }

  gtk_status_icon_set_from_pixbuf (status_icon, pixbuf);

  if (pixbuf) {
    g_object_unref (pixbuf);
  }
}

static gboolean
zenity_notification_icon_size_changed_cb (GtkStatusIcon *icon,
					  gint size,
					  gpointer user_data)
{
  icon_size = size;

  /* If we're displaying not a stock icon but a custom pixbuf,
   * we need to update the icon for the new size.
   */
  if (!icon_stock) {
    zenity_notification_icon_update ();

    return TRUE;
  }

  return FALSE;
}

static gboolean
zenity_notification_icon_activate_cb (GtkWidget *widget,
				      ZenityData *data)
{
  data->exit_code = zenity_util_return_exit_code (ZENITY_OK);

  gtk_main_quit ();

  return TRUE;
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
        icon_stock = zenity_util_stock_from_filename (value);

        g_free (icon_file);
        icon_file = g_strdup (value);

        if (icon_stock) {
          gtk_status_icon_set_from_stock (status_icon, icon_stock);
        } else if (gtk_status_icon_get_visible (status_icon) &&
                   gtk_status_icon_is_embedded (status_icon)) {
          zenity_notification_icon_update ();
        }
      } else if (!g_ascii_strcasecmp (command, "message")) {
#ifdef HAVE_LIBNOTIFY
        /* display a notification bubble */
        if (!g_utf8_validate (value, -1, NULL)) {
          g_warning ("Invalid UTF-8 in input!");
        } else if (notify_is_initted ()) {
          NotifyNotification *notif;
          const gchar *icon = NULL;
          gchar *freeme = NULL;
          gchar *message;
          error = NULL;

          message = g_strcompress (value);

          if (icon_stock) {
            icon = icon_stock;
          } else if (icon_file) {
            icon = freeme = g_filename_to_uri (icon_file, NULL, NULL);
          }

          notif = notify_notification_new_with_status_icon (message, NULL /* summary */,
                					    icon, status_icon);
          g_free (message);
          g_free (freeme);

	  notify_notification_show (notif, &error);

	  if (error) {
	    g_warning ("Error showing notification: %s", error->message);
	    g_error_free (error);
	  }

	  g_object_unref (notif);
	} else {
#else
	{ /* this brace is for balance */
#endif
 	  g_warning ("Notification framework not available");
	}
      } else if (!g_ascii_strcasecmp (command, "tooltip")) {
        if (g_utf8_validate (value, -1, NULL)) {
          gtk_status_icon_set_tooltip_text (status_icon, value);
        } else {
          g_warning ("Invalid UTF-8 in input!");
        }
      } else if (!g_ascii_strcasecmp (command, "visible")) {
        if (!g_ascii_strcasecmp (value, "false")) {
          gtk_status_icon_set_visible (status_icon, FALSE);
        } else {
          gtk_status_icon_set_visible (status_icon, TRUE);
	}
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
  status_icon = gtk_status_icon_new ();
  g_signal_connect (status_icon, "size-changed",
		    G_CALLBACK (zenity_notification_icon_size_changed_cb), data);

  if (notification_data->notification_text) {
    gtk_status_icon_set_tooltip_text (status_icon, notification_data->notification_text);
  } else {
    gtk_status_icon_set_tooltip_text (status_icon, _("Zenity notification"));
  }

  icon_file = g_strdup (data->window_icon);
  icon_stock = zenity_util_stock_from_filename (data->window_icon);

  /* Only set the stock icon here; if we're going to display a
   * custom pixbuf we wait for the size-changed signal to load
   * it at the right size.
   */
  if (icon_stock) {
    gtk_status_icon_set_from_stock (status_icon, icon_stock);
  }

#ifdef HAVE_LIBNOTIFY
  /* create the notification widget */
  if (!notify_is_initted ()) {
    notify_init (_("Zenity notification"));
  }
#endif
 
  if (notification_data->listen) {
    zenity_notification_listen_on_stdin (data);
  } else {
    /* if we aren't listening for changes, then close on activate (left-click) */
    g_signal_connect (status_icon, "activate",
		      G_CALLBACK (zenity_notification_icon_activate_cb), data);
  }

  /* Show icon and wait */
  gtk_status_icon_set_visible (status_icon, TRUE);

  if(data->timeout_delay > 0) {
    g_timeout_add (data->timeout_delay * 1000, (GSourceFunc) zenity_util_timeout_handle, NULL);
  }

  gtk_main ();

  /* Cleanup */
  g_object_unref (status_icon);
  g_free (icon_file);
}
