/*
 * msg.c
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

void zenity_msg_dialog_response (GtkWindow *window, int button, gpointer data);

int zenity_msg (ZenityData *data, ZenityMsgData *msg_data)
{
	GladeXML *glade_dialog;
	GtkWidget *dialog;
	GtkWidget *text;

	switch (msg_data->mode) {

		case ZENITY_MSG_WARNING:
			glade_dialog = zenity_util_load_glade_file ("zenity_warning_dialog");
			dialog = glade_xml_get_widget (glade_dialog, "zenity_warning_dialog");
			text = glade_xml_get_widget (glade_dialog, "zenity_warning_text");
			break;

		case ZENITY_MSG_QUESTION:
			glade_dialog = zenity_util_load_glade_file ("zenity_question_dialog");
			dialog = glade_xml_get_widget (glade_dialog, "zenity_question_dialog");
			text = glade_xml_get_widget (glade_dialog, "zenity_question_text");
			break;

		case ZENITY_MSG_ERROR:
			glade_dialog = zenity_util_load_glade_file ("zenity_error_dialog");
			dialog = glade_xml_get_widget (glade_dialog, "zenity_error_dialog");
			text = glade_xml_get_widget (glade_dialog, "zenity_error_text");
			break;

		case ZENITY_MSG_INFO:
			glade_dialog = zenity_util_load_glade_file ("zenity_info_dialog");
			dialog = glade_xml_get_widget (glade_dialog, "zenity_info_dialog");
			text = glade_xml_get_widget (glade_dialog, "zenity_info_text");
			break;
		
		default:
			g_assert_not_reached ();
			break;	
	}

	if (glade_dialog == NULL)
		return FALSE;
	
	glade_xml_signal_autoconnect (glade_dialog);

	if (data->dialog_title)
		gtk_window_set_title (GTK_WINDOW (dialog), data->dialog_title);

	if (data->window_icon)
		zenity_util_set_window_icon (dialog, data->window_icon);
	else {
		GdkPixbuf	*pixbuf = NULL;
		switch (msg_data->mode) {
	
			case ZENITY_MSG_WARNING:
				zenity_util_set_window_icon_from_stock (dialog, GTK_STOCK_DIALOG_WARNING);
				break;
			case ZENITY_MSG_QUESTION:
				zenity_util_set_window_icon_from_stock (dialog, GTK_STOCK_DIALOG_QUESTION);
				break;
			case ZENITY_MSG_ERROR:
				zenity_util_set_window_icon_from_stock (dialog, GTK_STOCK_DIALOG_ERROR);
				break;
			case ZENITY_MSG_INFO:
				zenity_util_set_window_icon_from_stock (dialog, GTK_STOCK_DIALOG_INFO);
				break;
			default:
				break;
		}
	}

	gtk_label_set_text (GTK_LABEL (text), msg_data->dialog_text);

	gtk_widget_show (dialog);
	gtk_main ();

	if (glade_dialog)
		g_object_unref (glade_dialog);

	return TRUE;
}

void
zenity_msg_dialog_response (GtkWindow *window, int button, gpointer data)
{
	GError *error = NULL;

	switch (button) {
		case GTK_RESPONSE_OK:
			gtk_main_quit ();
			break;

		case GTK_RESPONSE_CANCEL:
			gtk_main_quit ();
			break;

		default:
			break;
	}
}
