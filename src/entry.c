/* vim: colorcolumn=80 ts=4 sw=4
 */
/*
 * entry.c
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

#include <config.h>

static void zenity_entry_dialog_response (GtkWidget *widget, char *rstr, gpointer data);

static GtkWidget *entry;
static guint n_entries = 0;

static void
zenity_entry_combo_activate_default (GtkEntry *entry, gpointer window)
{
	g_signal_emit_by_name (window, "activate-default");
}

void
zenity_entry (ZenityData *data, ZenityEntryData *entry_data)
{
	g_autoptr(GtkBuilder) builder = NULL;
	GtkWidget *dialog;
	GObject *text;
	GObject *vbox;

	builder = zenity_util_load_ui_file ("zenity_entry_dialog", "zenity_entry_box", NULL);

	if (builder == NULL)
	{
		data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
		return;
	}

	dialog = GTK_WIDGET(gtk_builder_get_object (builder,
				"zenity_entry_dialog"));

	g_signal_connect (dialog, "response", G_CALLBACK(zenity_entry_dialog_response), data);

	zenity_util_setup_dialog_title (dialog, data);

	gtk_window_set_icon_name (GTK_WINDOW(dialog),
			"insert-text");

	if (data->width > -1 || data->height > -1)
		gtk_window_set_default_size (GTK_WINDOW (dialog),
				data->width, data->height);

	if (data->modal)
		gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

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

	text = gtk_builder_get_object (builder, "zenity_entry_text");

	if (entry_data->dialog_text) {
		gtk_label_set_text_with_mnemonic (GTK_LABEL (text),
				g_strcompress (entry_data->dialog_text));
	}

	vbox = gtk_builder_get_object (builder, "vbox4");

	/* The argv strings we have left will NOT include the first entry_text,
	 * which is defined at entry_data->entry_text. So we need to increment
	 * n_entries to take that into account.
	 */
	n_entries = g_strv_length (entry_data->data);
	if (entry_data->entry_text) ++n_entries;

	if (n_entries > 1)
	{
		G_GNUC_BEGIN_IGNORE_DEPRECATIONS

		GtkWidget *child;

		entry = gtk_combo_box_text_new_with_entry ();
		child = gtk_combo_box_get_child (GTK_COMBO_BOX(entry));

		for (int i = 0; entry_data->data[i]; ++i)
		{
			gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(entry),
					entry_data->data[i]);
		}

		if (entry_data->entry_text)
		{
			gtk_combo_box_text_prepend_text (GTK_COMBO_BOX_TEXT(entry),
					entry_data->entry_text);
			gtk_combo_box_set_active (GTK_COMBO_BOX(entry), 0);
		}

		g_signal_connect (child, "activate",
			G_CALLBACK (zenity_entry_combo_activate_default),
			GTK_WINDOW (dialog));

		G_GNUC_END_IGNORE_DEPRECATIONS
	}
	else
	{
		GtkEntryBuffer *buffer;

		entry = gtk_entry_new ();
		buffer = gtk_entry_get_buffer (GTK_ENTRY(entry));

		gtk_entry_set_activates_default (GTK_ENTRY(entry), TRUE);

		if (entry_data->entry_text) {
			gtk_entry_buffer_set_text (buffer, entry_data->entry_text, -1);
		}

		if (entry_data->hide_text) {
			gtk_entry_set_visibility (GTK_ENTRY(entry), FALSE);
		}
	}

	gtk_box_append (GTK_BOX(vbox), entry);

	gtk_label_set_mnemonic_widget (GTK_LABEL (text), entry);

	zenity_util_show_dialog (dialog);

	if (data->timeout_delay > 0)
	{
		ZENITY_UTIL_SETUP_TIMEOUT (dialog)
	}

	zenity_util_gapp_main (GTK_WINDOW(dialog));
}

static void
zenity_entry_dialog_output (void)
{
	const char *text;

	if (n_entries > 1)
	{
		G_GNUC_BEGIN_IGNORE_DEPRECATIONS
		text = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT(entry));
		G_GNUC_END_IGNORE_DEPRECATIONS
	}
	else
	{
		GtkEntryBuffer *buffer;

		buffer = gtk_entry_get_buffer (GTK_ENTRY(entry));
		text = gtk_entry_buffer_get_text (buffer);
	}

	if (text != NULL)
		g_print ("%s\n", text);
}

static void
zenity_entry_dialog_response (GtkWidget *widget, char *rstr, gpointer data)
{
	ZenityData *zen_data = data;
	int response = zenity_util_parse_dialog_response (rstr);

	switch (response)
	{
		case ZENITY_OK:
			zenity_entry_dialog_output ();
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
			break;

		case ZENITY_CANCEL:
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_CANCEL);
			break;

		case ZENITY_TIMEOUT:
			zenity_entry_dialog_output ();
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
