/*
 * calendar.c
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
#include "zenity.h"
#include "util.h"


static GtkWidget *calendar;

static void zenity_calendar_dialog_response (GtkWidget *widget, int response, gpointer data);

void 
zenity_calendar (ZenityData *data, ZenityCalendarData *cal_data)
{
	GladeXML *glade_dialog = NULL;
	GtkWidget *dialog;
	GtkWidget *text;

	glade_dialog = zenity_util_load_glade_file ("zenity_calendar_dialog");

	if (glade_dialog == NULL) {
		data->exit_code = -1;
		return;
	}
	
	glade_xml_signal_autoconnect (glade_dialog);

	dialog = glade_xml_get_widget (glade_dialog, "zenity_calendar_dialog");

	g_signal_connect (G_OBJECT (dialog), "response",
			  G_CALLBACK (zenity_calendar_dialog_response), data);

	if (data->dialog_title)	
		gtk_window_set_title (GTK_WINDOW (dialog), data->dialog_title);

	if (data->window_icon)
		zenity_util_set_window_icon (dialog, data->window_icon);
	else 
		zenity_util_set_window_icon (dialog, ZENITY_IMAGE_FULLPATH ("zenity-calendar.png"));

	text = glade_xml_get_widget (glade_dialog, "zenity_calendar_text");
	gtk_label_set_text (GTK_LABEL (text), cal_data->dialog_text);

	calendar = glade_xml_get_widget (glade_dialog, "zenity_calendar");
	
	if (glade_dialog)
		g_object_unref (glade_dialog);

	if (cal_data->month > 0 && cal_data->year > 0)
		gtk_calendar_select_month (GTK_CALENDAR (calendar), cal_data->month - 1, cal_data->year);
	if (cal_data->day > 0)
		gtk_calendar_select_day (GTK_CALENDAR (calendar), cal_data->day);

	gtk_label_set_mnemonic_widget (GTK_LABEL (text), calendar);
	gtk_widget_show (dialog);
	gtk_main ();
}

static void
zenity_calendar_dialog_response (GtkWidget *widget, int response, gpointer data)
{
	ZenityData *zen_data = data;
	guint day, month, year;

	switch (response) {
		case GTK_RESPONSE_OK:
			gtk_calendar_get_date (GTK_CALENDAR (calendar), &day, &month, &year);
			g_printerr ("%02d/%02d/%02d\n", year, month + 1, day);
			zen_data->exit_code = 0;
			gtk_main_quit ();
			break;

		case GTK_RESPONSE_CANCEL:
			zen_data->exit_code = 1;
			gtk_main_quit ();
			break;

		default:
			/* Esc dialog */
			zen_data->exit_code = 1;
			break;
	}
}
