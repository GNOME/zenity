/*
 * about.c
 *
 * Copyright (C) 2002 Sun Microsystems, Inc.
 * Copyright (C) 2001 CodeFactory AB
 * Copyright (C) 2001, 2002 Anders Carlsson
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
 *          Anders Carlsson <andersca@gnu.org>
 */

#include "config.h"
#include "zenity.h"
#include "util.h"
#include <glade/glade.h>

#define GTK_RESPONSE_CREDITS 0
#define ZENITY_HELP_PATH ZENITY_DATADIR "/help/"

static GtkWidget *dialog;
static GtkWidget *cred_dialog;

static void zenity_about_dialog_response (GtkWidget *widget, int response, gpointer data);

/* Sync with the people in the THANKS file */
static const gchar *author_credits[] = {
        "Jonathan Blanford <jrb@redhat.com>",
        "Anders Carlsson <andersca@gnu.org>",
        "Glynn Foster <glynn.foster@sun.com>",
        "James Henstridge <james@daa.com.au>",
        "Mike Newman <mike@gtnorthern.demon.co.uk>",
        "Havoc Pennington <hp@redhat.com>",
        "Kristian Rietveld <kris@gtk.org>",
        "Jakub Steiner <jimmac@ximian.com>",
        "Tom Tromey <tromey@redhat.com>",
        NULL
};

gchar *translator_credits;

void 
zenity_about (ZenityData *data)
{
	GladeXML *glade_dialog = NULL;
        GdkPixbuf *pixbuf;
	GtkWidget *label;
        GtkWidget *image;
        gchar *text;

	glade_dialog = zenity_util_load_glade_file ("zenity_about_dialog");

	if (glade_dialog == NULL) {
		data->exit_code = -1;
		return;
        }

	translator_credits = _("translator_credits");


	glade_xml_signal_autoconnect (glade_dialog);

	dialog = glade_xml_get_widget (glade_dialog, "zenity_about_dialog");

	g_signal_connect (G_OBJECT (dialog), "response",
			  G_CALLBACK (zenity_about_dialog_response), data);

        zenity_util_set_window_icon (dialog, ZENITY_IMAGE_FULLPATH ("zenity.png"));

        image = glade_xml_get_widget (glade_dialog, "zenity_about_image");

        pixbuf = gdk_pixbuf_new_from_file (ZENITY_IMAGE_FULLPATH ("zenity.png"), NULL);
    
        if (pixbuf != NULL) {
                gtk_image_set_from_pixbuf (GTK_IMAGE (image), pixbuf);
                g_object_unref (pixbuf);
        }

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

static GtkWidget *
zenity_about_create_label (void)
{
        GtkWidget *label;

        label = gtk_label_new ("");
        gtk_label_set_selectable (GTK_LABEL (label), TRUE);
        gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
        gtk_misc_set_padding (GTK_MISC (label), 8, 8);

        gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

        return label;
}

static void
zenity_about_update_author_label (GtkWidget *label)
{
        GString *string;
        gchar *tmp;
        gint i = 0;

        gtk_widget_show (label);

        string = g_string_new ("");

        for (i = 0; author_credits[i] != NULL; i++) {
                tmp = g_markup_escape_text (author_credits[i], -1);
                g_string_append (string, tmp);

                if (author_credits[i+1] != NULL)
                          g_string_append (string, "\n");

                g_free (tmp);
        }
        gtk_label_set_markup (GTK_LABEL (label), string->str);
        g_string_free (string, TRUE);
}

static void
zenity_about_update_translator_label (GtkWidget *label)
{
        GString *string;
        gchar *tmp;

        if (strcmp (translator_credits, "translator_credits") == 0) {
                gtk_widget_hide (label);
                return;
        } else {
                gtk_widget_show (label);
        }

        string = g_string_new ("");

        tmp = g_markup_escape_text (translator_credits, -1);
        g_string_append (string, tmp);
        g_free (tmp);

        gtk_label_set_markup (GTK_LABEL (label), string->str);
        g_string_free (string, TRUE);
}

static void
zenity_about_dialog_credits_response (GtkWidget *widget, int response, gpointer data)
{
        gtk_widget_destroy (widget);
        widget = NULL;
}

static void
zenity_about_display_credits_dialog (void)
{
        GtkWidget *credits_dialog;
        GtkWidget *label, *notebook, *sw;

        if (cred_dialog != NULL) {
                gtk_window_present (GTK_WINDOW (cred_dialog));
                return;
        }

        credits_dialog = gtk_dialog_new_with_buttons (_("Credits"), 
                                                      GTK_WINDOW (dialog), GTK_DIALOG_DESTROY_WITH_PARENT,
                                                      GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);

	g_signal_connect (G_OBJECT (credits_dialog), "response",
			  G_CALLBACK (gtk_widget_destroy), credits_dialog);
	g_signal_connect (G_OBJECT (credits_dialog), "destroy",
			  G_CALLBACK (gtk_widget_destroyed), &cred_dialog);

        cred_dialog = credits_dialog;

        gtk_window_set_default_size (GTK_WINDOW (credits_dialog), 360, 260);
        gtk_dialog_set_default_response (GTK_DIALOG (credits_dialog), GTK_RESPONSE_OK);

        notebook = gtk_notebook_new ();
        gtk_container_set_border_width (GTK_CONTAINER (notebook), 8);
        gtk_box_pack_start (GTK_BOX (GTK_DIALOG (credits_dialog)->vbox), notebook, TRUE, TRUE, 0);

        if (author_credits != NULL) {
                label = zenity_about_create_label ();
                sw = gtk_scrolled_window_new (NULL, NULL);
                gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), 
                                                GTK_POLICY_AUTOMATIC,
                                                GTK_POLICY_AUTOMATIC);
                gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sw), label);
                gtk_viewport_set_shadow_type (GTK_VIEWPORT (GTK_BIN (sw)->child), GTK_SHADOW_NONE);
                gtk_notebook_append_page (GTK_NOTEBOOK (notebook), sw,
                                          gtk_label_new (_("Written by")));
                zenity_about_update_author_label (label);
        }

        if (translator_credits != NULL) {
                label = zenity_about_create_label ();
                sw = gtk_scrolled_window_new (NULL, NULL);
                gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), 
                                                GTK_POLICY_AUTOMATIC,
                                                GTK_POLICY_AUTOMATIC);
                gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sw), label);
                gtk_viewport_set_shadow_type (GTK_VIEWPORT (GTK_BIN (sw)->child), GTK_SHADOW_NONE);
                gtk_notebook_append_page (GTK_NOTEBOOK (notebook), sw,
                                          gtk_label_new (_("Translated by")));
                zenity_about_update_translator_label (label);
        }

        gtk_widget_show_all (credits_dialog);
}

static void
zenity_about_dialog_response (GtkWidget *widget, int response, gpointer data)
{
        ZenityData *zen_data = data;
        GError *error = NULL;

	switch (response) {
		case GTK_RESPONSE_OK:
			zen_data->exit_code = 0;
			gtk_main_quit ();
			break;

		case GTK_RESPONSE_HELP:
                        zenity_util_show_help (ZENITY_HELP_PATH, "zenity.xml", NULL);
			break;

                case GTK_RESPONSE_CREDITS:
                        zenity_about_display_credits_dialog ();
                        break;
		default:
			/* Esc dialog */
			zen_data->exit_code = 1;
			break;
	}
}
