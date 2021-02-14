/* vim: colorcolumn=80 ts=4 sw=4
 */
/*
 * fileselection.c
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
 * Original Author: Glynn Foster <glynn.foster@sun.com>
 */

#include "util.h"
#include "zenity.h"

#include <string.h>

#include <config.h>

static ZenityData *zen_data;

static void zenity_fileselection_dialog_response (GtkDialog *dialog,
		int response, gpointer data);

void
zenity_fileselection (ZenityData *data, ZenityFileData *file_data)
{
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
	GtkFileChooserNative *dialog;

	zen_data = data;

	if (file_data->directory) {
		action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
	}
	else if (file_data->save) {
		action = GTK_FILE_CHOOSER_ACTION_SAVE;
	}

	dialog = gtk_file_chooser_native_new (data->dialog_title,
		NULL,	/* parent */
		action,
		_("_OK"),
		_("_Cancel"));

	if (data->modal)
		gtk_native_dialog_set_modal (GTK_NATIVE_DIALOG(dialog), TRUE);

	if (data->extra_label)
		g_warning ("Cannot add extra labels to GtkFileChooserNative");

	g_signal_connect (dialog, "response",
		G_CALLBACK(zenity_fileselection_dialog_response), file_data);

	if (file_data->uri)
	{
		if (g_path_is_absolute (file_data->uri) == TRUE)
		{
			char *dir = g_path_get_dirname (file_data->uri);
			GFile *dir_gfile = g_file_new_for_path (dir);

			gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(dialog),
					dir_gfile,
					NULL);	/* GError */

			g_free (dir);
			g_object_unref (dir_gfile);
		}

		if (file_data->uri[strlen (file_data->uri) - 1] != '/')
		{
			if (file_data->save)
			{
				char *basename = g_path_get_basename (file_data->uri);

				gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog),
						basename);

				g_free (basename);
			}
			else
			{
				GFile *file = g_file_new_for_uri (file_data->uri);

				gtk_file_chooser_set_file (GTK_FILE_CHOOSER(dialog),
						file,
						NULL);	/* GError */

				g_object_unref (file);
			}
		}
	}

	if (file_data->multi)
		gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER(dialog), TRUE);

	if (file_data->filter)
	{
		/* Filter format: Executables | *.exe *.bat *.com */
		for (int filter_i = 0; file_data->filter[filter_i]; filter_i++)
		{
			GtkFileFilter *filter = gtk_file_filter_new ();
			char *filter_str = file_data->filter[filter_i];
			char **pattern, **patterns;
			char *name = NULL;
			int i;

			/* Set name */
			for (i = 0; filter_str[i] != '\0'; i++)
				if (filter_str[i] == '|')
					break;

			if (filter_str[i] == '|') {
				name = g_strndup (filter_str, i);
				g_strstrip (name);
			}

			if (name) {
				gtk_file_filter_set_name (filter, name);

				/* Point i to the right position for split */
				for (++i; filter_str[i] == ' '; i++)
					;
			} else {
				gtk_file_filter_set_name (filter, filter_str);
				i = 0;
			}

			/* Get patterns */
			patterns = g_strsplit_set (filter_str + i, " ", -1);

			for (pattern = patterns; *pattern; pattern++)
				gtk_file_filter_add_pattern (filter, *pattern);

			if (name)
				g_free (name);

			g_strfreev (patterns);

			gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(dialog), filter);
		}
	}
	gtk_native_dialog_show (GTK_NATIVE_DIALOG (dialog));

	if (data->timeout_delay > 0) {
		g_timeout_add_seconds (data->timeout_delay,
			(GSourceFunc) zenity_util_timeout_handle,
			dialog);
	}

	/* Since a native dialog is not a GtkWindow, we can't use our handy
	 * util function.
	 */
	gtk_native_dialog_show (GTK_NATIVE_DIALOG(dialog));

	zenity_util_gapp_main (NULL);
}

static void
zenity_fileselection_dialog_output (GtkFileChooser *chooser,
		ZenityFileData *file_data)
{
	GListModel *model = gtk_file_chooser_get_files (chooser);
	guint items = g_list_model_get_n_items (model);

	for (guint i = 0; i < items; ++i)
	{
		GFile *file = g_list_model_get_item (model, i);

		g_print ("%s", g_file_get_path (file));

		if (i != items - 1)
			g_print ("%s", file_data->separator);

		g_object_unref (file);
	}
	g_print ("\n");

	g_object_unref (model);
}

static void
zenity_fileselection_dialog_response (GtkDialog *dialog,
		int response, gpointer data)
{
	ZenityFileData *file_data = data;
	GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);

	switch (response)
	{
		case GTK_RESPONSE_ACCEPT:
			zenity_fileselection_dialog_output (chooser, file_data);
			zenity_util_exit_code_with_data (ZENITY_OK, zen_data);
			break;

		case GTK_RESPONSE_CANCEL:
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_CANCEL);
			break;

		case ZENITY_TIMEOUT:
			zenity_fileselection_dialog_output (chooser, file_data);
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_TIMEOUT);
			break;

		default:
			if (zen_data->extra_label &&
					response < (int)g_strv_length (zen_data->extra_label)) {
				printf ("%s\n", zen_data->extra_label[response]);
			}
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_ESC);
			break;
	}
	zenity_util_gapp_quit (NULL);
}
