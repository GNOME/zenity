/* vim: colorcolumn=80 ts=4 sw=4
 */
/*
 * progress.c
 *
 * Copyright © 2002 Sun Microsystems, Inc.
 * Copyright © 2021 Logan Rathbone
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 * Original Author: Glynn Foster <glynn.foster@sun.com>
 */


#include "util.h"
#include "zenity.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <config.h>

static GtkBuilder *builder;
static ZenityData *zen_data;
static GIOChannel *channel;

static int pulsate_timeout = -1;
static gboolean autokill;
static gboolean no_cancel;
static gboolean auto_close;

gint zenity_progress_timeout (gpointer data);
gint zenity_progress_pulsate_timeout (gpointer data);

static void zenity_progress_dialog_response (GtkWidget *widget,
		int response, gpointer data);

static gboolean
zenity_progress_pulsate_progress_bar (gpointer user_data)
{
	gtk_progress_bar_pulse (GTK_PROGRESS_BAR (user_data));

	return TRUE;
}

static void
zenity_progress_pulsate_stop (void)
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
				zenity_progress_pulsate_progress_bar, progress_bar);
	}
}

static void
zenity_progress_update_time_remaining (ZenityProgressData *progress_data)
{
	static GObject *progress_time = NULL;
	static time_t start_time = (time_t) (-1);
	float percentage = progress_data->percentage;

	if (progress_time == NULL)
		progress_time = gtk_builder_get_object (builder,
				"zenity_progress_time");

	if (start_time == (time_t) (-1) || percentage <= 0.0 ||
		percentage >= 100.0)
	{
		start_time = time (NULL);
		gtk_label_set_text (GTK_LABEL (progress_time), "");
	}
	else
	{
		time_t current_time = time (NULL);
		time_t elapsed_time = current_time - start_time;
		time_t total_time =
			(time_t) (100.0 * elapsed_time / progress_data->percentage);
		time_t remaining_time = total_time - elapsed_time;
		gulong hours, minutes, seconds;
		char *remaining_message;

		seconds = (gulong) (remaining_time % 60);
		remaining_time /= 60;
		minutes = (gulong) (remaining_time % 60);
		remaining_time /= 60;
		hours = (gulong) remaining_time;

		remaining_message =
			g_strdup_printf (_("Time remaining: %lu:%02lu:%02lu"),
					hours, minutes, seconds);
		gtk_label_set_text (GTK_LABEL (progress_time), remaining_message);

		g_free (remaining_message);
	}
}

static float
stof (const char *s)
{
	float rez = 0, fact = 1;

	if (*s == '-') {
		s++;
		fact = -1;
	}

	for (int point_seen = 0; *s; s++)
	{
		int d;

		if (*s == '.' || *s == ',') {
			point_seen = 1;
			continue;
		}

		d = *s - '0';
		if (d >= 0 && d <= 9) {
			if (point_seen) fact /= 10.0f;
			rez = rez * 10.0f + (float)d;
		}
	}
	return rez * fact;
}

static gboolean
zenity_progress_handle_stdin (GIOChannel *channel, GIOCondition condition,
		gpointer data)
{
	static ZenityProgressData *progress_data;
	static GObject *progress_bar;
	static GObject *progress_label;
	static GtkWindow *parent;
	float percentage = 0.0;
	GIOStatus status = G_IO_STATUS_NORMAL;

	progress_data = data;
	progress_bar = gtk_builder_get_object (builder, "zenity_progress_bar");
	progress_label = gtk_builder_get_object (builder, "zenity_progress_text");
	parent = GTK_WINDOW(gtk_widget_get_native (GTK_WIDGET(progress_bar)));

	if ((condition & G_IO_IN) != 0)
	{
		GString *string = g_string_new (NULL);
		GError *error = NULL;

		while (channel->is_readable != TRUE)
			;
		do {
			do {
				status = g_io_channel_read_line_string (
					channel, string, NULL, &error);

				while (g_main_context_pending (NULL)) {
					g_main_context_iteration (NULL, FALSE);
				}
			} while (status == G_IO_STATUS_AGAIN);

			if (status != G_IO_STATUS_NORMAL)
			{
				if (error) {
					g_warning ("%s: %s",
							__func__, error->message);
					g_error_free (error);
					error = NULL;
				}
				continue;
			}

			if (! g_ascii_strncasecmp (string->str, "#", 1))
			{
				char *match;

				/* We have a comment, so let's try to change the label */
				match = g_strstr_len (string->str, strlen (string->str), "#");
				match++;
				gtk_label_set_text (GTK_LABEL (progress_label),
					g_strcompress (g_strchomp (g_strchug (match))));

			}
			else if (g_str_has_prefix (string->str, "pulsate"))
			{
				char *colon, *command, *value;

				zenity_util_strip_newline (string->str);

				colon = strchr (string->str, ':');
				if (colon == NULL) {
					continue;
				}

				/* split off the command and value */
				command =
					g_strstrip (g_strndup (string->str, colon - string->str));

				value = colon + 1;
				while (*value && g_ascii_isspace (*value))
					value++;

				if (! g_ascii_strcasecmp (value, "false"))
				{
					zenity_progress_pulsate_stop ();

					gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(progress_bar),
						progress_data->percentage / 100.0);
				}
				else {
					zenity_progress_pulsate_start (progress_bar);
				}

				g_free (command);
			}
			else
			{
				if (! g_ascii_isdigit (*(string->str)))
					continue;

				/* Now try to convert the thing to a number */
				percentage = CLAMP (stof (string->str), 0, 100);

				gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(progress_bar),
						percentage / 100.0);

				progress_data->percentage = percentage;

				if (progress_data->time_remaining == TRUE)
					zenity_progress_update_time_remaining (progress_data);

				if (percentage == 100)
				{
					GObject *button;

					button = gtk_builder_get_object (builder,
							"zenity_progress_ok_button");
					gtk_widget_set_sensitive (GTK_WIDGET (button), TRUE);
					gtk_widget_grab_focus (GTK_WIDGET (button));

					if (progress_data->autoclose)
					{
						zen_data->exit_code =
							zenity_util_return_exit_code (ZENITY_OK);

						zenity_util_gapp_quit (parent);
					}
				}
			}

		} while ((g_io_channel_get_buffer_condition (channel) & G_IO_IN) ==
				G_IO_IN &&
			status != G_IO_STATUS_EOF);
		g_string_free (string, TRUE);
	}

	if ((condition & G_IO_IN) != G_IO_IN || status == G_IO_STATUS_EOF)
	{
		/* We assume that we are done, so stop the pulsating and de-sensitize
		 * the buttons */
		GtkWidget *button;

		button = GTK_WIDGET(gtk_builder_get_object (builder,
					"zenity_progress_ok_button"));
		gtk_widget_set_sensitive (button, TRUE);
		gtk_widget_grab_focus (button);

		button = GTK_WIDGET (gtk_builder_get_object (builder,
					"zenity_progress_cancel_button"));

		gtk_widget_set_sensitive (button, FALSE);
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress_bar), 1.0);

		zenity_progress_pulsate_stop ();

		g_object_unref (builder);

		if (progress_data->autoclose)
		{
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
			zenity_util_gapp_quit (parent);
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
	g_io_add_watch (channel,
		G_IO_IN | G_IO_HUP,
		zenity_progress_handle_stdin,
		progress_data);
	/* We need to check the pulsate state here, because, the g_io_add_watch
	   doesn't call the zenity_progress_handle_stdin function if there's no
	   input. This fix the Bug 567663 */
	if (progress_data->pulsate)
	{
		GObject *progress_bar =
			gtk_builder_get_object (builder, "zenity_progress_bar");

		zenity_progress_pulsate_start (progress_bar);
	}
}

void
zenity_progress (ZenityData *data, ZenityProgressData *progress_data)
{
	GtkWidget *dialog;
	GtkWidget *button;
	GObject *text;
	GObject *progress_bar;
	GObject *cancel_button, *ok_button;

	zen_data = data;
	builder = zenity_util_load_ui_file ("zenity_progress_dialog", NULL);

	if (builder == NULL) {
		data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
		return;
	}

	text = gtk_builder_get_object (builder, "zenity_progress_text");

	dialog = GTK_WIDGET(gtk_builder_get_object (builder,
				"zenity_progress_dialog"));

	progress_bar = gtk_builder_get_object (builder, "zenity_progress_bar");

	g_signal_connect (dialog, "response",
		G_CALLBACK(zenity_progress_dialog_response), data);

	if (data->dialog_title)
		gtk_window_set_title (GTK_WINDOW(dialog), data->dialog_title);

	gtk_window_set_icon_name (GTK_WINDOW(dialog),
			"appointment-soon");

	if (data->width > -1 || data->height > -1)
		gtk_window_set_default_size (GTK_WINDOW(dialog),
				data->width, data->height);

	if (data->width > -1)
	{
		gtk_widget_set_size_request (GTK_WIDGET(text), data->width, -1);
	}
#if 0
	else
	{
		g_signal_connect_after (text, "size-allocate",
			G_CALLBACK(zenity_text_size_allocate), data);

		g_signal_connect_after (progress_bar, "size-allocate",
			G_CALLBACK(zenity_text_size_allocate), data);
	}
#endif

	if (data->modal)
		gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

	if (data->extra_label)
	{
		for (int i = 0; data->extra_label[i] != NULL; ++i)
		{
			gtk_dialog_add_button (GTK_DIALOG(dialog),
					data->extra_label[i], i);
		}
	}

	if (data->ok_label)
	{
		button = GTK_WIDGET(gtk_builder_get_object (builder,
					"zenity_progress_ok_button"));
		gtk_button_set_label (GTK_BUTTON(button), data->ok_label);
	}

	if (data->cancel_label)
	{
		button = GTK_WIDGET(gtk_builder_get_object (builder,
					"zenity_progress_cancel_button"));
		gtk_button_set_label (GTK_BUTTON(button), data->cancel_label);
	}

	if (progress_data->dialog_text) {
		gtk_label_set_markup (GTK_LABEL(text),
				g_strcompress (progress_data->dialog_text));
	}

	if (progress_data->percentage > -1) {
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(progress_bar),
				progress_data->percentage / 100.0);
	}

	autokill = progress_data->autokill;

	auto_close = progress_data->autoclose;
	ok_button = gtk_builder_get_object (builder, "zenity_progress_ok_button");

	no_cancel = progress_data->no_cancel;
	cancel_button =
		gtk_builder_get_object (builder, "zenity_progress_cancel_button");

	if (no_cancel) {
		gtk_widget_hide (GTK_WIDGET(cancel_button));
		gtk_window_set_deletable (GTK_WINDOW(dialog), FALSE);
	}

	if (no_cancel && auto_close)
		gtk_widget_hide (GTK_WIDGET(ok_button));

	zenity_util_show_dialog (dialog);
	zenity_progress_read_info (progress_data);

	if (data->timeout_delay > 0)
	{
		g_timeout_add_seconds (data->timeout_delay,
			(GSourceFunc) zenity_util_timeout_handle,
			NULL);
	}
	zenity_util_gapp_main (GTK_WINDOW(dialog));
}

static void
zenity_progress_dialog_response (GtkWidget *widget, int response,
		gpointer data)
{
	switch (response)
	{
		case GTK_RESPONSE_OK:
			zenity_util_exit_code_with_data (ZENITY_OK, zen_data);
			break;

		case GTK_RESPONSE_CANCEL:
			/* We do not want to kill the parent process, in order to give the
			 * user the ability to choose the action to be taken. But we want
			 * to give people the option to choose this behavior.
			*/
			if (autokill) {
				kill (getppid (), 1);
			}
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_CANCEL);
			break;

		case ZENITY_TIMEOUT:
			zenity_util_exit_code_with_data (ZENITY_TIMEOUT, zen_data);
			break;

		default:
			if (zen_data->extra_label &&
				response < (int)g_strv_length (zen_data->extra_label))
			{
				printf ("%s\n", zen_data->extra_label[response]);
			}
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_ESC);
			break;
	}
	zenity_util_gapp_quit (GTK_WINDOW(widget));
}
