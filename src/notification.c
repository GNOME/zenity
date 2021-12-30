/* vim: colorcolumn=80 ts=4 sw=4
 */
/*
 * notification.c
 *
 * Copyright © 2002 Sun Microsystems, Inc.
 * Copyright © 2006 Christian Persch
 * Copyright © 2021 Logan Rathbone
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
#ifdef HAVE_LIBNOTIFY
#include <libnotify/notify.h>

#include "util.h"
#include "zenity.h"

#define MAX_HINTS 16

static NotifyNotification *
zenity_notification_new (char *message, char *icon_file)
{
	NotifyNotification *notif;
	g_auto(GStrv) text = NULL;

	text = g_strsplit (g_strcompress (message), "\n", 2);
	if (*text == NULL)
	{
		g_printerr (_ ("Could not parse message\n"));
		return NULL;
	}

	notif = notify_notification_new (text[0], /* title */
		text[1], /* summary */
		icon_file);

	return notif;
}

static void
on_notification_default_action (NotifyNotification *n,
		const char *action, void *user_data)
{
	ZenityData *zen_data = user_data;

	notify_notification_close (n, NULL);

	zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);

	zenity_util_gapp_quit (NULL);
}

static GHashTable *
zenity_notification_parse_hints_array (char **hints)
{
	g_autoptr(GHashTable) result = NULL;
	g_auto(GStrv) pair = NULL;
	int i;

	result = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

	for (i = 0; i < (int)g_strv_length (hints); i++)
	{
		pair = g_strsplit (hints[i], ":", 2);
		g_hash_table_replace (result, g_strdup (pair[0]), g_strdup (pair[1]));
	}

	if (g_hash_table_size (result) == 0) {
		return NULL;
	} else {
		return result;
	}
}

static GHashTable *
zenity_notification_parse_hints (char *hints)
{
	GHashTable *result;
	g_auto(GStrv) hint_array = NULL;

	hint_array = g_strsplit (g_strcompress (hints), "\n", MAX_HINTS);
	result = zenity_notification_parse_hints_array (hint_array);

	return result;
}

static void
zenity_notification_set_hint (gpointer key, gpointer value,
		gpointer user_data)
{
	NotifyNotification *notification = user_data;
	char *hint_name = key;
	char *string_value = value;

	GVariant *hint_value;
	gboolean boolean_value;
	gint32 int_value;
	guchar byte_value;

	if ((g_ascii_strcasecmp ("action-icons", hint_name) == 0) ||
		(g_ascii_strcasecmp ("resident", hint_name) == 0) ||
		(g_ascii_strcasecmp ("suppress-sound", hint_name) == 0) ||
		(g_ascii_strcasecmp ("transient", hint_name) == 0))
	{
		/* boolean hints */
		if (g_ascii_strcasecmp ("true", string_value) == 0) {
			boolean_value = TRUE;
		} else if (g_ascii_strcasecmp ("false", string_value) == 0) {
			boolean_value = FALSE;
		} else {
			g_printerr (_ ("Invalid value for a boolean typed hint.\nSupported "
						   "values are 'true' or 'false'.\n"));
			return;
		}
		hint_value = g_variant_new_boolean (boolean_value);
	}
	else if ((g_ascii_strcasecmp ("category", hint_name) == 0) ||
		(g_ascii_strcasecmp ("desktop-entry", hint_name) == 0) ||
		(g_ascii_strcasecmp ("image-path", hint_name) == 0) ||
		(g_ascii_strcasecmp ("image_path", hint_name) == 0) ||
		(g_ascii_strcasecmp ("sound-file", hint_name) == 0) ||
		(g_ascii_strcasecmp ("sound-name", hint_name) == 0))
	{
		/* string hints */
		hint_value = g_variant_new_string (string_value);
	}
	else if ((g_ascii_strcasecmp ("image-data", hint_name) == 0) ||
		(g_ascii_strcasecmp ("image_data", hint_name) == 0) ||
		(g_ascii_strcasecmp ("icon-data", hint_name) == 0))
	{
		/* (iibiiay) */
		g_printerr (_ ("Unsupported hint. Skipping.\n"));
		return;
	}
	else if ((g_ascii_strcasecmp ("x", hint_name) == 0) ||
		(g_ascii_strcasecmp ("y", hint_name) == 0))
	{
		/* int hints */
		if (string_value == NULL)
			string_value = "";
		int_value = (gint32) g_ascii_strtoll (string_value, NULL, 0);
		hint_value = g_variant_new_int32 (int_value);
	}
	else if ((g_ascii_strcasecmp ("urgency", hint_name) == 0))
	{
		/* byte hints */
		if (string_value == NULL)
			string_value = "";
		byte_value = (guchar) g_ascii_strtoll (string_value, NULL, 0);
		hint_value = g_variant_new_byte (byte_value);
	}
	else
	{
		/* unknown hints */
		g_printerr (_("Unknown hint name. Skipping.\n"));
		return;
	}

	notify_notification_set_hint (notification, hint_name, hint_value);
}

static void
zenity_notification_set_hints (NotifyNotification *notification,
		GHashTable *hints)
{
	if (hints == NULL)
		return;

	g_hash_table_foreach (hints, zenity_notification_set_hint, notification);
}

static gboolean
zenity_notification_handle_stdin (GIOChannel *channel, GIOCondition condition,
		gpointer user_data)
{
	g_autofree char *icon_file = NULL;
	g_autoptr(GHashTable) notification_hints = NULL;

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
				icon_file = g_strdup (value);
			}
			else if (!g_ascii_strcasecmp (command, "hints"))
			{
				notification_hints = zenity_notification_parse_hints (value);
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
					NotifyNotification *notif;
					error = NULL;

					notif = zenity_notification_new (value, icon_file);
					if (notif == NULL)
						continue;

					zenity_notification_set_hints (notif, notification_hints);

					notify_notification_show (notif, &error);
					if (error) {
						g_warning (
							"Error showing notification: %s", error->message);
						error = NULL;
					}

					g_object_unref (notif);
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
					NotifyNotification *notif =
						zenity_notification_new (value, icon_file);

					if (notif == NULL)
						continue;

					zenity_notification_set_hints (notif, notification_hints);

					notify_notification_show (notif, &error);
					if (error)
					{
						g_warning ("Error showing notification: %s",
								error->message);

						error = NULL;
					}
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
	g_autoptr(GError) error = NULL;
	NotifyNotification *notification;
	g_autoptr(GHashTable) notification_hints = NULL;

	/* create the notification widget */
	if (!notify_is_initted ()) {
		notify_init (_ ("Zenity notification"));
	}

	if (notification_data->listen)
	{
		zenity_notification_listen_on_stdin (data);

		zenity_util_gapp_main (NULL);
	}
	else
	{
		if (notification_data->notification_text == NULL)
			exit (1);

		notification =
			zenity_notification_new (notification_data->notification_text,
					notification_data->icon);

		if (notification == NULL)
			exit (1);

		/* if we aren't listening for changes, then close on default action */
		notify_notification_add_action (notification,
			"default",
			"Do Default Action",
			(NotifyActionCallback) on_notification_default_action,
			data,
			NULL);

		/* set the notification hints for the displayed notification */
		if (notification_data->notification_hints != NULL)
		{
			notification_hints = zenity_notification_parse_hints_array (
				notification_data->notification_hints);

			zenity_notification_set_hints (notification, notification_hints);
		}

		/* Show icon and wait */
		error = NULL;
		if (! notify_notification_show (notification, &error))
		{
			if (error != NULL) {
				g_warning ("Error showing notification: %s", error->message);
			}
			exit (1);
		}
	}

	if (data->timeout_delay > 0) {
		g_timeout_add_seconds (data->timeout_delay,
			(GSourceFunc) zenity_util_timeout_handle,
			NULL);

		zenity_util_gapp_main (NULL);
	}
}

#endif
