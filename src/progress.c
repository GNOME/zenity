/*
 * progress.c
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

#include <stdio.h>
#include <glade/glade.h>
#include "zenity.h"
#include "util.h"

static guint timer;
static GladeXML *glade_dialog;

gint zenity_progress_timeout (gpointer data);
gint zenity_progress_pulsate_timeout (gpointer data);

static void zenity_progress_dialog_response (GtkWidget *widget, int response, gpointer data);

void
zenity_progress (ZenityData *data, ZenityProgressData *progress_data)
{
	GtkWidget *dialog;
	GtkWidget *text;
	GtkWidget *progress_bar;
	guint input;

	glade_dialog = zenity_util_load_glade_file ("zenity_progress_dialog");

	if (glade_dialog == NULL) {
		data->exit_code = -1;
		return;
	}
	
	glade_xml_signal_autoconnect (glade_dialog);

	dialog = glade_xml_get_widget (glade_dialog, "zenity_progress_dialog");

	g_signal_connect (G_OBJECT (dialog), "response",
			  G_CALLBACK (zenity_progress_dialog_response), data);
	
	if (data->dialog_title)
		gtk_window_set_title (GTK_WINDOW (dialog), data->dialog_title);

	if (data->window_icon)
		zenity_util_set_window_icon (dialog, data->window_icon);
	else {
		zenity_util_set_window_icon (dialog, ZENITY_IMAGE_FULLPATH ("zenity-progress.png"));
	}

	text = glade_xml_get_widget (glade_dialog, "zenity_progress_text");
	gtk_label_set_text (GTK_LABEL (text), progress_data->dialog_text);

	progress_bar = glade_xml_get_widget (glade_dialog, "zenity_progress_bar");

	if (progress_data->percentage > -1)
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress_bar), 
					       progress_data->percentage/100.0);
	
	gtk_widget_show (dialog);
	if (progress_data->pulsate != TRUE)
		timer = gtk_timeout_add (100, zenity_progress_timeout, progress_bar);
	else
		timer = gtk_timeout_add (100, zenity_progress_pulsate_timeout, progress_bar);

	gtk_main ();
}

gint 
zenity_progress_timeout (gpointer data)
{
	gchar buffer[256];
	float percentage;

        while(gtk_events_pending()) {
                gtk_main_iteration();

                if (timer == 0)
                        return FALSE;
        }

	if (scanf ("%255s", buffer) == EOF) {
		GtkWidget *button;

		button = glade_xml_get_widget (glade_dialog, "zenity_progress_ok_button");
		gtk_widget_set_sensitive (button, TRUE);
		gtk_widget_grab_focus (button);

		button = glade_xml_get_widget (glade_dialog, "zenity_progress_cancel_button");
		gtk_widget_set_sensitive (button, FALSE);

		if (glade_dialog)
			g_object_unref (glade_dialog);

		return FALSE;
	} else {
		percentage = atoi (buffer);

		if (percentage > 100)
			gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (data), 1.0);
		else
			gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (data), percentage / 100.0);

		return TRUE;
	}
}

gint 
zenity_progress_pulsate_timeout (gpointer data)
{
        
	while(gtk_events_pending()) {
                gtk_main_iteration();

                if (timer == 0)
                        return FALSE;
        }

	if (feof (stdin)) {
		gtk_progress_bar_pulse (GTK_PROGRESS_BAR (data));

		return FALSE;
	} else  {
		GtkWidget *button;

		/* We stop the pulsating and switch the focus on the dialog buttons */

		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (data), 1.0);

		button = glade_xml_get_widget (glade_dialog, "zenity_progress_ok_button");
		gtk_widget_set_sensitive (button, TRUE);
		gtk_widget_grab_focus (button);

		button = glade_xml_get_widget (glade_dialog, "zenity_progress_cancel_button");
		gtk_widget_set_sensitive (button, FALSE);

		return TRUE;
	}
}

static void
zenity_progress_dialog_response (GtkWidget *widget, int response, gpointer data)
{
	ZenityData *zen_data = data;

	switch (response) {
		case GTK_RESPONSE_OK:
			zen_data->exit_code = 0;
			gtk_main_quit ();
			break;

		case GTK_RESPONSE_CANCEL:
			zen_data->exit_code = 1;
			gtk_main_quit ();
			break;

		default:
			zen_data->exit_code = 1;
			break;
	}
}
