/*
 * entry.c
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

static void zenity_entry_dialog_response (GtkWidget *widget, int response, gpointer data);

static GtkWidget *entry;

void 
zenity_entry (ZenityData *data, ZenityEntryData *entry_data)
{
  GladeXML *glade_dialog = NULL;
  GtkWidget *dialog;
  GtkWidget *text;

  glade_dialog = zenity_util_load_glade_file ("zenity_entry_dialog");

  if (glade_dialog == NULL) {
    data->exit_code = -1;
    return;
  }
	
  glade_xml_signal_autoconnect (glade_dialog);
	
  dialog = glade_xml_get_widget (glade_dialog, "zenity_entry_dialog");

	
  g_signal_connect (G_OBJECT (dialog), "response",
                    G_CALLBACK (zenity_entry_dialog_response), data);

  if (data->dialog_title)
    gtk_window_set_title (GTK_WINDOW (dialog), data->dialog_title);
	
  if (data->window_icon)
    zenity_util_set_window_icon (dialog, data->window_icon);
  else
    zenity_util_set_window_icon (dialog, ZENITY_IMAGE_FULLPATH ("zenity-entry.png"));

  gtk_window_set_default_size (GTK_WINDOW (dialog), data->width, data->height);

  text = glade_xml_get_widget (glade_dialog, "zenity_entry_text");

  if (entry_data->dialog_text)
    gtk_label_set_text_with_mnemonic (GTK_LABEL (text), entry_data->dialog_text);

  entry = glade_xml_get_widget (glade_dialog, "zenity_entry_input");
	
  if (glade_dialog)
    g_object_unref (glade_dialog);

  if (entry_data->entry_text)
    gtk_entry_set_text (GTK_ENTRY (entry), entry_data->entry_text);

  if (!entry_data->visible)
    g_object_set (G_OBJECT (entry), "visibility", entry_data->visible, NULL);

  gtk_label_set_mnemonic_widget (GTK_LABEL (text), entry);

  gtk_widget_show (dialog);
  gtk_main ();
}

static void
zenity_entry_dialog_response (GtkWidget *widget, int response, gpointer data)
{
  ZenityData *zen_data = data;
  const gchar *text;

  switch (response) {
    case GTK_RESPONSE_OK:
      zen_data->exit_code = 0;
      text = gtk_entry_get_text (GTK_ENTRY (entry));

      if (text != NULL)
        g_printerr ("%s\n", gtk_entry_get_text (GTK_ENTRY (entry)));

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
