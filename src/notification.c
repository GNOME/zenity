/*
 * notification.c
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
#include <time.h>
#include "zenity.h"
#include "eggtrayicon.h"
#include "util.h"

static EggTrayIcon *tray_icon;
static GtkWidget *icon_image;
static GtkWidget *icon_event_box;
static GtkTooltips *tooltips;


static void
set_scaled_pixbuf (GtkImage *image, GdkPixbuf *pixbuf, GtkIconSize icon_size)
{
  GdkScreen *screen;
  GtkSettings *settings;
  int width, height, desired_width, desired_height;
  GdkPixbuf *new_pixbuf;

  screen = gtk_widget_get_screen (GTK_WIDGET (image));
  settings = gtk_settings_get_for_screen (screen);
  if (!gtk_icon_size_lookup_for_settings (settings, icon_size,
					  &desired_width, &desired_height))
    return;

  width = gdk_pixbuf_get_width (pixbuf);
  height = gdk_pixbuf_get_height (pixbuf);
  if (height > desired_height || width > desired_width) {
    if (width * desired_height / height > desired_width)
      desired_height = height * desired_width / width;
    else
      desired_width = width * desired_height / height;

    new_pixbuf = gdk_pixbuf_scale_simple (pixbuf,
					  desired_width,
					  desired_height,
					  GDK_INTERP_BILINEAR);
    gtk_image_set_from_pixbuf (image, new_pixbuf);
    g_object_unref (new_pixbuf);
  } else {
    gtk_image_set_from_pixbuf (image, pixbuf);
  }
}

static gboolean
zenity_notification_icon_press_callback (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  ZenityData *zen_data;

  zen_data = data;

  zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
  gtk_main_quit ();
  return TRUE;
}

static gboolean
zenity_notification_icon_expose_callback (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  if (GTK_WIDGET_HAS_FOCUS (widget)) {
    gint focus_width, focus_pad;
    gint x, y, width, height;
                                                                                                                                                             
    gtk_widget_style_get (widget,
                          "focus-line-width", &focus_width,
                          "focus-padding", &focus_pad,
                          NULL);
    x = widget->allocation.x + focus_pad;
    y = widget->allocation.y + focus_pad;
    width = widget->allocation.width - 2 * focus_pad;
    height = widget->allocation.height - 2 * focus_pad;
                                                                                                                                                             
    gtk_paint_focus (widget->style, widget->window,
                     GTK_WIDGET_STATE (widget),
                     &event->area, widget, "button",
                     x, y, width, height);
    }
                                                                                                                                                             
    return FALSE;
}

static void
zenity_notification_icon_destroy_callback (GtkWidget *widget, gpointer data)
{
  ZenityData *zen_data;

  zen_data = data;
  gtk_widget_destroy (GTK_WIDGET (tray_icon));

  zen_data->exit_code = zenity_util_return_exit_code (ZENITY_ESC);
  gtk_main_quit ();
}

static gboolean
zenity_notification_handle_stdin (GIOChannel  *channel,
				  GIOCondition condition,
				  gpointer     user_data)
{
  ZenityData *zen_data;

  zen_data = (ZenityData *)user_data;

  if ((condition & G_IO_IN) != 0) {
    GString *string;
    GError *error = NULL;

    string = g_string_new (NULL);
    while (channel->is_readable != TRUE)
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
      command = g_strndup (string->str, colon - string->str);
      command = g_strstrip (command);
      g_strdown (command);

      value = colon + 1;
      while (*value && g_ascii_isspace (*value)) value++;

      if (!strcmp (command, "icon")) {
	GdkPixbuf *pixbuf;

	pixbuf = zenity_util_pixbuf_new_from_file (GTK_WIDGET (tray_icon),
						   value);
	if (pixbuf != NULL) {
	  set_scaled_pixbuf (GTK_IMAGE (icon_image), pixbuf,
			     GTK_ICON_SIZE_BUTTON);
	  gdk_pixbuf_unref (pixbuf);
	} else {
	  g_warning ("Could not load notification icon : %s", value);
	}
      } else if (!strcmp (command, "message")) {
	g_warning ("haven't implemented message support yet");
      } else if (!strcmp (command, "tooltip")) {
	gtk_tooltips_set_tip (tooltips, icon_event_box, value, value);
      } else if (!strcmp (command, "visible")) {
	if (!strcasecmp (value, "false")) {
          /* We need to get the parent, because just hiding the tray_icon
           * doesn't save on space. See #161539 for details
           */
	  gtk_widget_hide (gtk_widget_get_parent (GTK_WIDGET (tray_icon)));
	} else {
	  gtk_widget_show (gtk_widget_get_parent (GTK_WIDGET (tray_icon)));
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
  GdkPixbuf *pixbuf = NULL;

  tray_icon = egg_tray_icon_new (_("Zenity notification"));
  tooltips = gtk_tooltips_new ();

  if (data->window_icon != NULL)
    pixbuf = zenity_util_pixbuf_new_from_file (GTK_WIDGET (tray_icon), data->window_icon);
  else
    pixbuf = gdk_pixbuf_new_from_file (ZENITY_IMAGE_FULLPATH ("zenity-notification.png"), NULL);

  icon_event_box = gtk_event_box_new ();
  icon_image = gtk_image_new ();

  if (pixbuf) {
    set_scaled_pixbuf (GTK_IMAGE (icon_image), pixbuf,
		       GTK_ICON_SIZE_BUTTON);
    gdk_pixbuf_unref (pixbuf);
  } else {
    if (data->window_icon != NULL)  {
      g_warning ("Could not load notification icon : %s", data->window_icon);
    }
    else
      g_warning ("Could not load notification icon : %s", ZENITY_IMAGE_FULLPATH ("zenity-notification.png"));
    return; 
  }

  gtk_container_add (GTK_CONTAINER (icon_event_box), icon_image);

  if (notification_data->notification_text) 
    gtk_tooltips_set_tip (tooltips, icon_event_box, notification_data->notification_text, notification_data->notification_text);
  else
    gtk_tooltips_set_tip (tooltips, icon_event_box, _("Zenity notification"), _("Zenity notification"));
 
  gtk_widget_add_events (GTK_WIDGET (tray_icon), GDK_BUTTON_PRESS_MASK | GDK_FOCUS_CHANGE_MASK);
  gtk_container_add (GTK_CONTAINER (tray_icon), icon_event_box);

  g_signal_connect (tray_icon, "destroy",
		    G_CALLBACK (zenity_notification_icon_destroy_callback), data);

  g_signal_connect (tray_icon, "expose_event",
		    G_CALLBACK (zenity_notification_icon_expose_callback), data);

  if (notification_data->listen) {
    zenity_notification_listen_on_stdin (data);
  } else {
    /* if we aren't listening for changes, then close on button_press */
    g_signal_connect (tray_icon, "button_press_event",
		      G_CALLBACK (zenity_notification_icon_press_callback), data);
  }

  gtk_widget_show_all (GTK_WIDGET (tray_icon));
  
  /* Does nothing at the moment */
  gtk_main ();
}
