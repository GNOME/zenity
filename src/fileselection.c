/* vim: colorcolumn=80 ts=4 sw=4
 */
/*
 * fileselection.c
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
 * Original Author: Glynn Foster <glynn.foster@sun.com>
 */

#include "util.h"
#include "zenity.h"

#include <string.h>

#include <config.h>

static ZenityData *zen_data;

static void zenity_fileselection_dialog_response (GObject *widget, GAsyncResult *response, gpointer data);

static void
show_extra_button_deprecation_warning (void)
{
	g_printerr (_("Warning: the --extra-button option for --file-selection "
			"is deprecated and will be removed in a future version of zenity. "
			"Ignoring.\n"));
}

void
zenity_fileselection (ZenityData *data, ZenityFileData *file_data)
{
	GtkFileDialog *dialog = gtk_file_dialog_new ();

	zen_data = data;

	if (data->dialog_title)
		gtk_file_dialog_set_title (GTK_FILE_DIALOG(dialog), data->dialog_title);

	if (data->modal)
		gtk_file_dialog_set_modal (GTK_FILE_DIALOG(dialog), TRUE);

	if (data->ok_label)
		gtk_file_dialog_set_accept_label (GTK_FILE_DIALOG(dialog), data->ok_label);

	if (data->extra_label)
		show_extra_button_deprecation_warning ();

	if (file_data->uri)
	{
		g_autoptr(GFile) file = g_file_new_for_commandline_arg (file_data->uri);
		g_autofree char *basename = g_file_get_basename (file);

		/* If provided filename is not just a base name, then switch to its provided directory */
		if (g_strcmp0(basename, file_data->uri) != 0)
		{
			g_autoptr(GFile) dir_gfile =
				file_data->uri[strlen (file_data->uri) - 1] == G_DIR_SEPARATOR
				? g_object_ref (file)
				: g_file_get_parent (file);

			gtk_file_dialog_set_initial_folder (GTK_FILE_DIALOG(dialog), dir_gfile);
		}

		if (file_data->uri[strlen (file_data->uri) - 1] != G_DIR_SEPARATOR)
			gtk_file_dialog_set_initial_name (GTK_FILE_DIALOG(dialog), basename);
	}

	if (file_data->filter)
	{
		GListStore *filters = g_list_store_new(GTK_TYPE_FILE_FILTER);

		/* Filter format: Executables | *.exe *.bat *.com */
		for (int filter_i = 0; file_data->filter[filter_i]; filter_i++)
		{
			GtkFileFilter *filter = gtk_file_filter_new ();
			char *filter_str = file_data->filter[filter_i];
			GStrv pattern;
			g_auto(GStrv) patterns = NULL;
			g_autofree char *name = NULL;
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

			g_list_store_append (filters, filter);
		}

		gtk_file_dialog_set_filters (GTK_FILE_DIALOG(dialog), G_LIST_MODEL(filters));
	}

	if (data->timeout_delay > 0)
	{
		ZENITY_UTIL_SETUP_TIMEOUT (dialog)
	}

	if (file_data->multi)
	{
		if (file_data->directory)
			gtk_file_dialog_select_multiple_folders (dialog, NULL, NULL, zenity_fileselection_dialog_response, file_data);
		else
			gtk_file_dialog_open_multiple (dialog, NULL, NULL, zenity_fileselection_dialog_response, file_data);
	}
	else
	{
		if (file_data->save)
			gtk_file_dialog_save (dialog, NULL, NULL, zenity_fileselection_dialog_response, file_data);
		else if (file_data->directory)
			gtk_file_dialog_select_folder (dialog, NULL, NULL, zenity_fileselection_dialog_response, file_data);
		else
			gtk_file_dialog_open (dialog, NULL, NULL, zenity_fileselection_dialog_response, file_data);
	}

	zenity_util_gapp_main (NULL);
}

static void
zenity_fileselection_dialog_output (GListModel *model,
		ZenityFileData *file_data)
{
	guint items = g_list_model_get_n_items (model);

	for (guint i = 0; i < items; ++i)
	{
		g_autoptr(GFile) file = g_list_model_get_item (model, i);

		g_print ("%s", g_file_get_path (file));

		if (i != items - 1)
			g_print ("%s", file_data->separator);
	}
	g_print ("\n");
}

static void
zenity_fileselection_dialog_response (GObject *widget, GAsyncResult *response, gpointer data)
{
	ZenityFileData *file_data = data;
	GtkFileDialog *dialog = GTK_FILE_DIALOG (widget);
	g_autoptr(GError) local_error = NULL;

	if (file_data->multi)
	{
		g_autoptr(GListModel) model = NULL;

		if (file_data->directory)
			model = gtk_file_dialog_select_multiple_folders_finish (dialog, response, &local_error);
		else
			model = gtk_file_dialog_open_multiple_finish (dialog, response, &local_error);
		
		if (local_error && local_error->code != GTK_DIALOG_ERROR_DISMISSED)
			g_warning ("%s", local_error->message);

		if (local_error && local_error->code == GTK_DIALOG_ERROR_DISMISSED)
		{
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_CANCEL);
		}
		else if (model == NULL)
		{
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_ESC);
		}
		else
		{
			zenity_fileselection_dialog_output (model, file_data);
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
		}
	}
	else
	{
		g_autoptr(GFile) file = NULL;

		if (file_data->save)
			file = gtk_file_dialog_save_finish (dialog, response, &local_error);
		else if (file_data->directory)
			file = gtk_file_dialog_select_folder_finish (dialog, response, &local_error);
		else
			file = gtk_file_dialog_open_finish (dialog, response, &local_error);

		if (local_error && local_error->code != GTK_DIALOG_ERROR_DISMISSED)
			g_warning ("%s", local_error->message);

		if (local_error && local_error->code == GTK_DIALOG_ERROR_DISMISSED)
		{
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_CANCEL);
		}
		else if (file == NULL)
		{
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_ESC);
		}
		else
		{
			g_print ("%s\n", g_file_get_path (file));
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
		}
	}

	zenity_util_gapp_quit (NULL, zen_data);
}
