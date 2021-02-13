/* vim: colorcolumn=80 ts=4 sw=4
 */
/*
 * calendar.c
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
 * Authors: Glynn Foster <glynn.foster@sun.com>
 */


#include "util.h"
#include "zenity.h"

#include <time.h>

#include <config.h>

static GtkWidget *calendar;
static ZenityCalendarData *zen_cal_data;

static void zenity_calendar_dialog_response (GtkWidget *widget,
		int response, gpointer data);
#if 0
static void zenity_calendar_day_selected (GtkCalendar *calendar,
		gpointer data);
#endif

void
zenity_calendar (ZenityData *data, ZenityCalendarData *cal_data)
{
	GtkBuilder *builder;
	GtkWidget *dialog;
	GtkWidget *button;
	GObject *text;

	zen_cal_data = cal_data;

	builder = zenity_util_load_ui_file ("zenity_calendar_dialog", NULL);

	if (builder == NULL) {
		data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
		return;
	}

	dialog = GTK_WIDGET (gtk_builder_get_object (builder,
				"zenity_calendar_dialog"));

	g_signal_connect (dialog, "response",
		G_CALLBACK(zenity_calendar_dialog_response), data);

	if (data->dialog_title)
		gtk_window_set_title (GTK_WINDOW (dialog), data->dialog_title);

	zenity_util_set_window_icon (dialog,
		data->window_icon,
		ZENITY_IMAGE_FULLPATH ("zenity-calendar.png"));

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

	if (cal_data->month > 0 || cal_data->year > 0)
	{
		g_object_set (calendar,
				"month",	cal_data->month - 1,
				"year",		cal_data->year,
				NULL);
	}

	if (cal_data->day > 0)
	{
		g_object_set (calendar,
				"day", cal_data->day - 1,
				NULL);
	}

	/* day-selected-double-click is gone in gtk4, and having this emit upon
	 * single-click violates POLA more than just disabling the behaviour,
	 * IMO. */
#if 0
	g_signal_connect (calendar, "day-selected",
		G_CALLBACK(zenity_calendar_day_selected), data);
#endif

	gtk_label_set_mnemonic_widget (GTK_LABEL (text), calendar);
	zenity_util_show_dialog (dialog);

	if (data->timeout_delay > 0) {
		g_timeout_add_seconds (data->timeout_delay,
			(GSourceFunc) zenity_util_timeout_handle,
			dialog);
	}

	if (data->extra_label)
	{
		for (int i = 0; data->extra_label[i] != NULL; ++i) {
			gtk_dialog_add_button (GTK_DIALOG(dialog),
					data->extra_label[i], i);
		}
	}

	if (data->ok_label) {
		button = GTK_WIDGET (gtk_builder_get_object (builder,
					"zenity_calendar_ok_button"));
		gtk_button_set_label (GTK_BUTTON (button), data->ok_label);
	}

	if (data->cancel_label) {
		button = GTK_WIDGET (gtk_builder_get_object (builder,
					"zenity_calendar_cancel_button"));
		gtk_button_set_label (GTK_BUTTON (button), data->cancel_label);
	}

	g_object_unref (builder);

	zenity_util_gapp_main (GTK_WINDOW(dialog));
}

static void
zenity_calendar_dialog_output (void)
{
	int day, month, year;
	char *time_string;
	GDateTime *date;

	g_object_get (calendar,
			"day", &day,
			"month", &month,
			"year", &year,
			NULL);

	date = g_date_time_new_local (year, month + 1, day + 1,
			0, 0, 0);

	time_string = g_date_time_format (date, zen_cal_data->date_format);
	g_print ("%s\n", time_string);

	g_date_time_unref (date);
	g_free (time_string);
}

static void
zenity_calendar_dialog_response (GtkWidget *widget, int response,
		gpointer data)
{
	ZenityData *zen_data = data;

	switch (response)
	{
		case GTK_RESPONSE_OK:
			zenity_calendar_dialog_output ();
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
			break;

		case GTK_RESPONSE_CANCEL:
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_CANCEL);
			break;

		case ZENITY_TIMEOUT:
			zenity_calendar_dialog_output ();
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_TIMEOUT);
			break;

		default:
			if (zen_data->extra_label &&
				response < (int)g_strv_length (zen_data->extra_label))
				printf ("%s\n", zen_data->extra_label[response]);
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_ESC);
			break;
	}
	zenity_util_gapp_quit (GTK_WINDOW(gtk_widget_get_native (widget)));
}

#if 0
static void
zenity_calendar_day_selected (GtkCalendar *cal, gpointer data)
{
	zenity_calendar_dialog_response (GTK_WIDGET(cal), GTK_RESPONSE_OK, data);
}
#endif
