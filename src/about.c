/*
 * about.c
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
#include "config.h"
#include "zenity.h"
#include "util.h"

#define GTK_RESPONSE_CREDITS 1

static void zenity_about_dialog_response (GtkWidget *widget, int response, gpointer data);

void 
zenity_about (ZenityData *data)
{
	GladeXML *glade_dialog = NULL;
	GtkWidget *dialog;
	GtkWidget *label;
        gchar *text;

	glade_dialog = zenity_util_load_glade_file ("zenity_about_dialog");

	if (glade_dialog == NULL) {
		data->exit_code = -1;
		return;
	}
	
	glade_xml_signal_autoconnect (glade_dialog);

	dialog = glade_xml_get_widget (glade_dialog, "zenity_about_dialog");

	g_signal_connect (G_OBJECT (dialog), "response",
			  G_CALLBACK (zenity_about_dialog_response), data);

        /* FIXME: Set an appropriate window icon for the dialog
         * zenity_util_set_window_icon (dialog, ZENITY_IMAGE_FULLPATH (""));
         */

	label = glade_xml_get_widget (glade_dialog, "zenity_about_version");
        text = g_strdup_printf ("<span size=\"xx-large\" weight=\"bold\">Zenity %s</span>", VERSION);
	gtk_label_set_markup (GTK_LABEL (label), text);
        g_free (text);

        label = glade_xml_get_widget (glade_dialog, "zenity_about_description");
        gtk_label_set_text (GTK_LABEL (label), _("Display dialog boxes from shell scripts"));

        label = glade_xml_get_widget (glade_dialog, "zenity_about_copyright");
        text = g_strdup_printf ("<span size=\"small\">%s</span>", _("(C) 2003 Sun Microsystems"));
        gtk_label_set_markup (GTK_LABEL (label), text);
        g_free (text);

	if (glade_dialog)
		g_object_unref (glade_dialog);

	gtk_widget_show (dialog);
	gtk_main ();
}

static void
zenity_about_dialog_response (GtkWidget *widget, int response, gpointer data)
{
        ZenityData *zen_data = data;

	switch (response) {
		case GTK_RESPONSE_OK:
			zen_data->exit_code = 0;
			gtk_main_quit ();
			break;

		case GTK_RESPONSE_HELP:
			zen_data->exit_code = 1;
			gtk_main_quit ();
			break;

                case GTK_RESPONSE_CREDITS:
                        zen_data->exit_code = 1;
                        gtk_main_quit ();
                        break;
		default:
			/* Esc dialog */
			zen_data->exit_code = 1;
			break;
	}
}
