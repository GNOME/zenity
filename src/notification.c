/* vim: colorcolumn=80 ts=4 sw=4
 */
/*
 * notification.c
 *
 * Copyright © 2002 Sun Microsystems, Inc.
 * Copyright © 2006 Christian Persch
 * Copyright © 2021-2022 Logan Rathbone
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
 * Original Author: Glynn Foster <glynn.foster@sun.com>
 */

#include <config.h>

#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "util.h"
#include "zenity.h"

#define MAX_HINTS 16

static void
zenity_send_notification (GNotification *notification)
{
	g_application_send_notification (g_application_get_default (),
			"zenity-notification",
			notification);
}

static GNotification *
zenity_notification_new (char *message, char *icon_path)
{
	g_autoptr (GNotification) notif;
	g_auto(GStrv) text = NULL;

	/* Accept a title and optional body in the form of `my title\nmy body'.
	 * The way this is displayed by the notification system is implementation
	 * defined.
	 */
	text = g_strsplit (g_strcompress (message), "\n", 2);
	if (*text == NULL)
	{
		g_printerr (_("Could not parse message\n"));
		return NULL;
	}

	notif = g_notification_new (text[0]);	/* title */

	if (text[1])
		g_notification_set_body (notif, text[1]);

	if (icon_path)
	{
		g_autoptr(GIcon) icon = NULL;

		icon = zenity_util_gicon_from_string (icon_path);
		g_notification_set_icon (notif, icon);
	}

	return g_steal_pointer (&notif);
}

static void
on_notification_default_action (GSimpleAction *self,
		GVariant *parameter,
		gpointer user_data)
{
	ZenityData *zen_data = user_data;

	zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);

	zenity_util_gapp_quit (NULL, zen_data);
}

static gboolean
zenity_notification_handle_stdin (GIOChannel *channel, GIOCondition condition,
		gpointer user_data)
{
	static char *icon_file;

	if ((condition & G_IO_IN) != 0)
	{
		g_autoptr(GString) string = NULL;
		g_autoptr(GError) error = NULL;

		while (channel->is_readable == FALSE)
			;

		string = g_string_new (NULL);

		do {
			int status;
			g_autofree char *command = NULL;
			char *value, *colon;

			do {
				status = g_io_channel_read_line_string (channel, string,
						NULL, &error);

				while (g_main_context_pending (NULL)) {
					g_main_context_iteration (NULL, FALSE);
				}

			} while (status == G_IO_STATUS_AGAIN);

			if (status != G_IO_STATUS_NORMAL)
			{
				if (error) {
					g_warning ("%s: %s",
							__func__,
							error->message);
					error = NULL;
				}
				continue;
			}

			zenity_util_strip_newline (string->str);
			colon = strchr (string->str, ':');
			if (colon == NULL)
			{
				g_printerr (_("Could not parse command from stdin\n"));
				continue;
			}

			/* split off the command and value */
			command = g_strstrip (g_strndup (string->str, colon - string->str));

			value = colon + 1;
			while (*value && g_ascii_isspace (*value))
				value++;

			if (! g_ascii_strcasecmp (command, "icon"))
			{
				g_free (icon_file);
				icon_file = g_strdup (value);
			}
			else if (!g_ascii_strcasecmp (command, "message"))
			{
				/* display a notification bubble */
				if (! g_utf8_validate (value, -1, NULL))
				{
					g_warning ("Invalid UTF-8 in input!");
				}
				else
				{
					g_autoptr(GNotification) notif = NULL;

					notif = zenity_notification_new (value, icon_file);

					if (notif == NULL)
						continue;

					zenity_send_notification (notif);
				}
			}
			else if (! g_ascii_strcasecmp (command, "tooltip"))
			{
				if (! g_utf8_validate (value, -1, NULL))
				{
					g_warning ("Invalid UTF-8 in input!");
				}
				else
				{
					g_autoptr(GNotification) notif = NULL;

					notif = zenity_notification_new (value, icon_file);

					if (notif == NULL)
						continue;

					zenity_send_notification (notif);
				}
			}
			else if (!g_ascii_strcasecmp (command, "visible"))
			{

			}
			else
			{
				g_warning ("Unknown command '%s'", command);
			}

		} while (g_io_channel_get_buffer_condition (channel) == G_IO_IN);
	}

	if ((condition & G_IO_HUP) != 0)
	{
		g_io_channel_shutdown (channel, TRUE, NULL);
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
	g_io_add_watch (channel,
			G_IO_IN | G_IO_HUP,
			zenity_notification_handle_stdin,
			data);
}

void
zenity_notification (ZenityData *data,
		ZenityNotificationData *notification_data)
{
	GNotification *notification;

	if (notification_data->listen)
	{
		zenity_notification_listen_on_stdin (data);

		zenity_util_gapp_main (NULL);
	}
	else
	{
		g_autoptr(GSimpleAction) action = NULL;

		if (notification_data->notification_text == NULL)
			exit (1);

		notification = zenity_notification_new (
				notification_data->notification_text, notification_data->icon);

		if (notification == NULL)
			exit (1);

		/* if we aren't listening for changes, then close on default action */
		action = g_simple_action_new ("app.default", NULL);
		g_signal_connect (action, "activate",
				G_CALLBACK(on_notification_default_action), data);

		g_notification_set_default_action (notification, "app.default");

		zenity_send_notification (notification);
	}

	if (data->timeout_delay > 0) {
		g_timeout_add_seconds (data->timeout_delay,
			(GSourceFunc) zenity_util_timeout_handle,
			NULL);

		zenity_util_gapp_main (NULL);
	}
}
