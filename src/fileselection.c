/*
 * fileselection.c
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

void zenity_fileselection_dialog_response (GtkWindow *window, int button, gpointer data);

int zenity_fileselection (ZenityData *data, ZenityFileData *file_data)
{
	GladeXML *glade_dialog;
	GtkWidget *dialog;

	glade_dialog = zenity_util_load_glade_file ("zenity_fileselection_dialog");

	if (glade_dialog == NULL)
		return FALSE;
	
	glade_xml_signal_autoconnect (glade_dialog);

	dialog = glade_xml_get_widget (glade_dialog, "zenity_fileselection_dialog");
	if (data->dialog_title)
		gtk_window_set_title (GTK_WINDOW (dialog), data->dialog_title);
	
	if (data->window_icon)
		zenity_util_set_window_icon (dialog, data->window_icon);
	else
		zenity_util_set_window_icon (dialog, ZENITY_IMAGE_FULLPATH ("zenity-file.png"));

	if (file_data->uri)
		gtk_file_selection_set_filename (GTK_FILE_SELECTION (dialog), file_data->uri);

	gtk_widget_show (dialog);
	gtk_main ();

	if (glade_dialog)
		g_object_unref (glade_dialog);

	return TRUE;
}

void
zenity_fileselection_dialog_response (GtkWindow *window, int button, gpointer data)
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
