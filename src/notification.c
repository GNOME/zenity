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

EggTrayIcon *tray_icon;

static gboolean
zenity_notification_icon_press_callback (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  ZenityData *zen_data;

  zen_data = data;

  zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
  gtk_main_quit ();
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

static gboolean
zenity_notification_icon_destroy_callback (GtkWidget *widget, gpointer data)
{
  ZenityData *zen_data;

  zen_data = data;
  gtk_widget_destroy (GTK_WIDGET (tray_icon));

  zen_data->exit_code = zenity_util_return_exit_code (ZENITY_ESC);
  gtk_main_quit ();
}

void 
zenity_notification (ZenityData *data, ZenityNotificationData *notification_data)
{
  GtkWidget *icon_image;
  GtkWidget *icon_event_box;
  GtkTooltips *tooltips;
  GdkPixbuf *pixbuf = NULL;

  tray_icon = egg_tray_icon_new (_("Zenity notification"));
  tooltips = gtk_tooltips_new ();

  if (data->window_icon != NULL)
    pixbuf = zenity_util_pixbuf_new_from_file (GTK_WIDGET (tray_icon), data->window_icon);
  else
    pixbuf = gdk_pixbuf_new_from_file (ZENITY_IMAGE_FULLPATH ("zenity-notification.png"), NULL);

  icon_event_box = gtk_event_box_new ();

  if (pixbuf) {
    icon_image = gtk_image_new_from_pixbuf (pixbuf);
    gdk_pixbuf_unref (pixbuf);
  } else {
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

  g_signal_connect (tray_icon, "button_press_event",
		    G_CALLBACK (zenity_notification_icon_press_callback), data);

  g_signal_connect (tray_icon, "destroy",
		    G_CALLBACK (zenity_notification_icon_destroy_callback), data);

  g_signal_connect (tray_icon, "expose_event",
		    G_CALLBACK (zenity_notification_icon_expose_callback), data);

  gtk_widget_show_all (GTK_WIDGET (tray_icon));
  
  /* Does nothing at the moment */
  gtk_main ();
}
