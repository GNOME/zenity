/* vim: colorcolumn=80 ts=4 sw=4
 */
/*
 * calendar.c
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
 * Authors: Glynn Foster <glynn.foster@sun.com>
 */


#include "util.h"
#include "zenity.h"

#include <time.h>

#include <config.h>

static GtkWidget *calendar;
static ZenityCalendarData *zen_cal_data;

static void zenity_calendar_dialog_response (GtkWidget *widget, char *rstr, gpointer data);

void
zenity_calendar (ZenityData *data, ZenityCalendarData *cal_data)
{
	g_autoptr(GtkBuilder) builder = NULL;
	GtkWidget *dialog;
	GObject *text;

	zen_cal_data = cal_data;

	builder = zenity_util_load_ui_file ("zenity_calendar_dialog", "zenity_calendar_box", NULL);

	if (builder == NULL) {
		data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
		return;
	}

	dialog = GTK_WIDGET (gtk_builder_get_object (builder,
				"zenity_calendar_dialog"));

	g_signal_connect (dialog, "response",
		G_CALLBACK(zenity_calendar_dialog_response), data);

	zenity_util_setup_dialog_title (dialog, data);

	gtk_window_set_icon_name (GTK_WINDOW(dialog),
			"x-office-calendar");

	if (data->width > -1 || data->height > -1)
		gtk_window_set_default_size (GTK_WINDOW(dialog),
				data->width, data->height);

	if (data->modal)
		gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

	text = gtk_builder_get_object (builder, "zenity_calendar_text");

	if (cal_data->dialog_text)
		gtk_label_set_markup (GTK_LABEL(text),
				g_strcompress (cal_data->dialog_text));

	calendar = GTK_WIDGET(gtk_builder_get_object (builder, "zenity_calendar"));

	if (cal_data->month > 0 || cal_data->year > 0 || cal_data->day > 0)
	{
		g_autoptr(GDateTime) date = g_date_time_new_local (cal_data->year,
				cal_data->month, cal_data->day, 0, 0, 0);

		if (date)
			gtk_calendar_select_day (GTK_CALENDAR(calendar), date);
		else
			g_printerr (_("Invalid date provided. Falling back to today's date.\n"));
	}

	gtk_label_set_mnemonic_widget (GTK_LABEL (text), calendar);
	zenity_util_show_dialog (dialog);

	if (data->timeout_delay > 0)
	{
		ZENITY_UTIL_SETUP_TIMEOUT (dialog)
	}

	if (data->extra_label)
	{
		ZENITY_UTIL_ADD_EXTRA_LABELS (dialog)
	}

	if (data->ok_label)
	{
		ZENITY_UTIL_SETUP_OK_BUTTON_LABEL (dialog);
	}

	if (data->cancel_label)
	{
		ZENITY_UTIL_SETUP_CANCEL_BUTTON_LABEL (dialog);
	}

	zenity_util_gapp_main (GTK_WINDOW(dialog));
}

static void
zenity_calendar_dialog_output (void)
{
	g_autofree char *time_string = NULL;
	g_autoptr(GDateTime) date = NULL;

	date = gtk_calendar_get_date (GTK_CALENDAR(calendar));

	time_string = g_date_time_format (date, zen_cal_data->date_format);
	g_print ("%s\n", time_string);
}

static void
zenity_calendar_dialog_response (GtkWidget *widget, char *rstr, gpointer data)
{
	ZenityData *zen_data = data;
	int response = zenity_util_parse_dialog_response (rstr);

	switch (response)
	{
		case ZENITY_OK:
			zenity_calendar_dialog_output ();
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
			break;

		case ZENITY_CANCEL:
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_CANCEL);
			break;

		case ZENITY_TIMEOUT:
			zenity_calendar_dialog_output ();
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_TIMEOUT);
			break;

		case ZENITY_ESC:
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_ESC);
			break;

		default:
			ZENITY_UTIL_RESPONSE_HANDLE_EXTRA_BUTTONS
			break;
	}
	zenity_util_gapp_quit (GTK_WINDOW(gtk_widget_get_native (widget)), zen_data);
}
