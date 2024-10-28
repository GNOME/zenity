/* vim: colorcolumn=80 ts=4 sw=4
 */
/*
 * progress.c
 *
 * Copyright © 2002 Sun Microsystems, Inc.
 * Copyright © 2021-2023 Logan Rathbone
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

G_GNUC_BEGIN_IGNORE_DEPRECATIONS

static GtkBuilder *builder;
static ZenityData *zen_data;

static int pulsate_timeout = -1;
static gboolean autokill;
static gboolean no_cancel;
static gboolean auto_close;

static void zenity_progress_dialog_response (GtkWidget *widget, char *rstr, gpointer data);

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
		g_autofree char *remaining_message = NULL;

		seconds = (gulong) (remaining_time % 60);
		remaining_time /= 60;
		minutes = (gulong) (remaining_time % 60);
		remaining_time /= 60;
		hours = (gulong) remaining_time;

		remaining_message =
			g_strdup_printf (_("Time remaining: %lu:%02lu:%02lu"),
					hours, minutes, seconds);
		gtk_label_set_text (GTK_LABEL (progress_time), remaining_message);
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
zenity_progress_handle_stdin (GIOChannel *source, GIOCondition condition,
		gpointer data)
{
	ZenityProgressData *progress_data;
	GObject *progress_bar;
	GObject *progress_label;
	GtkWindow *parent;
	float percentage = 0.0;
	GIOStatus status = G_IO_STATUS_NORMAL;

	progress_data = data;
	progress_bar = gtk_builder_get_object (builder, "zenity_progress_bar");
	progress_label = gtk_builder_get_object (builder, "zenity_progress_text");
	parent = GTK_WINDOW(gtk_widget_get_native (GTK_WIDGET(progress_bar)));

	if ((condition & G_IO_IN) != 0)
	{
		g_autoptr(GString) string = g_string_new (NULL);
		g_autoptr(GError) error = NULL;

		while (source->is_readable != TRUE)
			;
		do {
			do {
				status = g_io_channel_read_line_string (
					source, string, NULL, &error);

				while (g_main_context_pending (NULL)) {
					g_main_context_iteration (NULL, FALSE);
				}
			} while (status == G_IO_STATUS_AGAIN);

			if (status != G_IO_STATUS_NORMAL)
			{
				if (error) {
					g_warning ("%s: %s",
							__func__, error->message);
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
				gtk_label_set_markup (GTK_LABEL (progress_label),
					g_strcompress (g_strchomp (g_strchug (match))));

			}
			else if (g_str_has_prefix (string->str, "pulsate"))
			{
				char *colon, *value;

				zenity_util_strip_newline (string->str);

				colon = strchr (string->str, ':');
				if (colon == NULL) {
					continue;
				}

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
					if (!auto_close)
					{
						adw_message_dialog_set_response_enabled (ADW_MESSAGE_DIALOG(parent), "ok", TRUE);
						adw_message_dialog_set_default_response (ADW_MESSAGE_DIALOG(parent), "ok");
					}

					if (progress_data->autoclose)
					{
						zen_data->exit_code =
							zenity_util_return_exit_code (ZENITY_OK);

						zenity_util_gapp_quit (parent, zen_data);
					}
				}
			}

		} while ((g_io_channel_get_buffer_condition (source) & G_IO_IN) ==
				G_IO_IN &&
			status != G_IO_STATUS_EOF);
	}

	if ((condition & G_IO_IN) != G_IO_IN || status == G_IO_STATUS_EOF)
	{
		/* We assume that we are done, so stop the pulsating and de-sensitize
		 * the buttons */
		if (!no_cancel)
			adw_message_dialog_set_response_enabled (ADW_MESSAGE_DIALOG(parent), "cancel", FALSE);
		if (!auto_close)
		{
			adw_message_dialog_set_response_enabled (ADW_MESSAGE_DIALOG(parent), "ok", TRUE);
			adw_message_dialog_set_default_response (ADW_MESSAGE_DIALOG(parent), "ok");
		}

		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress_bar), 1.0);

		zenity_progress_pulsate_stop ();

		if (progress_data->autoclose)
		{
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
			zenity_util_gapp_quit (parent, zen_data);
		}

		g_io_channel_shutdown (source, TRUE, NULL);
		return FALSE;
	}
	return TRUE;
}

static void
zenity_progress_read_info (ZenityProgressData *progress_data)
{
	GIOChannel *channel = g_io_channel_unix_new (0);

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
	GObject *text;
	GObject *progress_bar;

	/* Setup globals */

	zen_data = data;
	autokill = progress_data->autokill;
	auto_close = progress_data->autoclose;
	no_cancel = progress_data->no_cancel;
	builder = zenity_util_load_ui_file ("zenity_progress_dialog", "zenity_progress_box", NULL);

	if (builder == NULL) {
		data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
		return;
	}

	text = gtk_builder_get_object (builder, "zenity_progress_text");

	dialog = GTK_WIDGET(gtk_builder_get_object (builder,
				"zenity_progress_dialog"));

	progress_bar = gtk_builder_get_object (builder, "zenity_progress_bar");

	/* Setup responses */

	/* Unlike some other dialogs, this one starts off blank and we need to add
	 * the OK/Cancel buttons depending on the options.
	 */
	if (no_cancel)
		gtk_window_set_deletable (GTK_WINDOW(dialog), FALSE);
	else
		adw_message_dialog_add_response (ADW_MESSAGE_DIALOG(dialog), "cancel", _("_Cancel"));

	if (!auto_close)
	{
		adw_message_dialog_add_response (ADW_MESSAGE_DIALOG(dialog), "ok", _("_OK"));
		if (progress_data->percentage == 100) {
			adw_message_dialog_set_response_enabled (ADW_MESSAGE_DIALOG(dialog), "ok", TRUE);
			adw_message_dialog_set_default_response (ADW_MESSAGE_DIALOG(dialog), "ok");
		} else {
			adw_message_dialog_set_response_enabled (ADW_MESSAGE_DIALOG(dialog), "ok", FALSE);
		}
	}
	else
	{
		/* If the user specifies percentage should be 100 and the dialog should
		 * auto-close, this is completely pointless and we print an error.
		 */
		if (progress_data->percentage == 100) {
			/* Translators: do not translate tokens starting with '--'; these
			 * are command-line options which are not translatable.
			 */
			g_printerr (_("Combining the options --auto-close and --percentage=100 is not supported.\n"));
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
			zenity_util_gapp_quit (NULL, zen_data);
		}
	}

	g_signal_connect (dialog, "response", G_CALLBACK(zenity_progress_dialog_response), data);

	zenity_util_setup_dialog_title (dialog, data);

	gtk_window_set_icon_name (GTK_WINDOW(dialog),
			"appointment-soon");

	if (data->width > -1 || data->height > -1)
		gtk_window_set_default_size (GTK_WINDOW(dialog),
				data->width, data->height);

	if (data->width > -1)
	{
		gtk_widget_set_size_request (GTK_WIDGET(text), data->width, -1);
	}

	if (data->modal)
		gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

	if (data->extra_label)
	{
		ZENITY_UTIL_ADD_EXTRA_LABELS (dialog)
	}

	if (data->ok_label) {
		ZENITY_UTIL_SETUP_OK_BUTTON_LABEL (dialog);
	}

	if (data->cancel_label)
	{
		ZENITY_UTIL_SETUP_CANCEL_BUTTON_LABEL (dialog);
	}

	if (progress_data->dialog_text) {
		gtk_label_set_markup (GTK_LABEL(text),
				g_strcompress (progress_data->dialog_text));
	}

	if (progress_data->percentage > -1) {
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(progress_bar),
				progress_data->percentage / 100.0);
	}

	zenity_util_show_dialog (dialog);
	zenity_progress_read_info (progress_data);

	if (data->timeout_delay > 0)
	{
		ZENITY_UTIL_SETUP_TIMEOUT (dialog)
	}

	zenity_util_gapp_main (GTK_WINDOW(dialog));
}

static void
zenity_progress_dialog_response (GtkWidget *widget, char *rstr, gpointer data)
{
	int response = zenity_util_parse_dialog_response (rstr);

	switch (response)
	{
		case ZENITY_OK:
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
			break;

		case ZENITY_CANCEL:
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
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_TIMEOUT);
			break;

		case ZENITY_ESC:
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_ESC);
			break;

		default:
			ZENITY_UTIL_RESPONSE_HANDLE_EXTRA_BUTTONS
			break;
	}
	zenity_util_gapp_quit (GTK_WINDOW(widget), zen_data);
}

G_GNUC_END_IGNORE_DEPRECATIONS
