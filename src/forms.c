/* vim: colorcolumn=80 ts=4 sw=4
 */
/*
 * forms.c
 *
 * Copyright © 2010 Arx Cruz
 * Copyright © 2021-2026 Logan Rathbone
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
 * Free Software Foundation, Inc., 121 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 * Original Author: Arx Cruz <arxcruz@gnome.org>
 */

#include "util.h"
#include "zenity.h"

#include <string.h>

#include <config.h>

static ZenityData *zen_data;
static void zenity_forms_dialog_response (GtkWidget *widget, char *rstr, gpointer data);

static GtkWidget *
zenity_forms_create_and_fill_combo (ZenityFormsData *forms_data,
		int combo_number)
{
	GtkWidget *dropdown = NULL;

	if (forms_data->combo_values)
	{
		char *combo_values =
			g_slist_nth_data (forms_data->combo_values, combo_number);

		if (combo_values)
		{
			g_auto(GStrv) row_values = g_strsplit (combo_values, "|", -1);

			if (row_values)
				dropdown = gtk_drop_down_new_from_strings ((const char* const*)row_values);
		}
	}

	return dropdown;
}

static GtkWidget *
zenity_forms_create_and_fill_list (ZenityFormsData *forms_data,
		int list_number, char *header)
{
	GListStore *store = g_list_store_new (ZENITY_TREE_TYPE_ROW);
	ZenityTreeColumnView *col_view = g_object_new (ZENITY_TREE_TYPE_COLUMN_VIEW,
			"model", store,
			"hide-header", !forms_data->show_header,
			NULL);

	if (forms_data->column_values)
	{
		char *column_values = NULL;
		int columns_values_count = g_slist_length (forms_data->column_values);
		int column_number = 0;

		if (list_number < columns_values_count) {
			column_number = list_number;
		}

		column_values = g_slist_nth_data (forms_data->column_values,
				column_number);

		if (column_values)
		{
			g_auto(GStrv) values = g_strsplit (column_values, "|", -1);

			if (values)
			{
				const int n_columns = g_strv_length (values);

				for (int i = 0; i < n_columns; ++i)
				{
					const char *column_name = values[i];

					zenity_tree_column_view_add_column (col_view, column_name);
				}
			}
		}
	}

	if (forms_data->list_values)
	{
		char *list_values =
			g_slist_nth_data (forms_data->list_values, list_number);

		if (list_values)
		{
			g_auto(GStrv) row_values = g_strsplit (list_values, "|", -1);

			if (row_values)
			{
				const int n_columns = zenity_tree_column_view_get_n_columns (col_view);
				const guint n_row_values = g_strv_length (row_values);
				ZenityTreeRow *row = NULL;

				for (int i = 0; i < n_row_values; ++i)
				{
					ZenityTreeItem *item;
					gboolean at_new_row = i % n_columns == 0;

					if (at_new_row)
					{
						row = zenity_tree_row_new ();
						g_list_store_append (store, row);
					}

					g_assert (row != NULL);

					item = zenity_tree_item_new (row_values[i], gtk_label_new (NULL));
					zenity_tree_row_add (row, item);
				}
			}
		}
	}

	return GTK_WIDGET(col_view);
}

void
zenity_forms_dialog (ZenityData *data, ZenityFormsData *forms_data)
{
	g_autoptr(GtkBuilder) builder = NULL;
	GtkWidget *dialog;
	GtkWidget *grid;
	GtkWidget *text;
	GtkWidget *frame;

	int list_count = 0;
	int combo_count = 0;
	int i = 0;

	zen_data = data;

	builder = zenity_util_load_ui_file ("zenity_forms_dialog", "zenity_forms_box", NULL);

	if (builder == NULL) {
		data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
		return;
	}

	dialog = GTK_WIDGET(gtk_builder_get_object (builder,
				"zenity_forms_dialog"));

	frame = GTK_WIDGET(gtk_builder_get_object (builder, "frame1"));

	gtk_widget_set_hexpand(frame, TRUE);

	g_signal_connect (dialog, "response",
		G_CALLBACK (zenity_forms_dialog_response), forms_data);

	zenity_util_setup_dialog_title (dialog, data);

	if (data->width > -1 || data->height > -1) {
		gtk_window_set_default_size (GTK_WINDOW(dialog),
				data->width, data->height);
	}

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

	text = GTK_WIDGET (gtk_builder_get_object (builder, "zenity_forms_text"));

	if (forms_data->dialog_text)
		gtk_label_set_markup (
			GTK_LABEL (text), g_strcompress (forms_data->dialog_text));

	grid = GTK_WIDGET (gtk_builder_get_object (builder, "zenity_forms_grid"));

	i = 0;
	for (GSList *tmp = forms_data->list;
			tmp != NULL;
			tmp = tmp->next)
	{
		ZenityFormsValue *zenity_value = tmp->data;
		GtkWidget *label;

		label = gtk_label_new (zenity_value->option_value);
		gtk_widget_set_halign (label, GTK_ALIGN_START);
		gtk_grid_attach (GTK_GRID (grid), label, 0, i, 1, 1);

		switch (zenity_value->type)
		{
			case ZENITY_FORMS_ENTRY:
				zenity_value->forms_widget = gtk_entry_new ();
				break;

			case ZENITY_FORMS_PASSWORD:
				zenity_value->forms_widget = gtk_entry_new ();
				gtk_entry_set_visibility (GTK_ENTRY(zenity_value->forms_widget),
						FALSE);
				break;

			case ZENITY_FORMS_CALENDAR:
				zenity_value->forms_widget = gtk_calendar_new ();
				break;

			case ZENITY_FORMS_LIST:
				zenity_value->forms_widget =
					zenity_forms_create_and_fill_list (forms_data,
							list_count, zenity_value->option_value);
				list_count++;
				break;

			case ZENITY_FORMS_COMBO:
				zenity_value->forms_widget =
					zenity_forms_create_and_fill_combo (forms_data,
							combo_count);
				combo_count++;
				break;

			case ZENITY_FORMS_MULTLINE_ENTRY:
			{
				GtkWidget *tv = gtk_text_view_new ();

				zenity_value->forms_widget = gtk_scrolled_window_new ();
				gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW(zenity_value->forms_widget), tv);

				/* Try to keep consistent with the properties set for
				 * zenity_text_view in the .ui file (used with --text-info)
				 */
				gtk_widget_set_vexpand (zenity_value->forms_widget, TRUE);
				gtk_text_view_set_editable (GTK_TEXT_VIEW(tv), TRUE);
				gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW(tv), GTK_WRAP_WORD);

				/* Being able to insert tabs in a textview is nice, but NOT
				 * being able to tab between widgets in forms is just annoying.
				 */
				gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW(tv), FALSE);
			}
				break;

			default:
				zenity_value->forms_widget = gtk_entry_new ();
				break;
		}

		gtk_widget_set_hexpand(zenity_value->forms_widget, TRUE);

		gtk_grid_attach_next_to (GTK_GRID (grid),
			GTK_WIDGET (zenity_value->forms_widget),
			label,
			GTK_POS_RIGHT,
			1,
			1);

		gtk_accessible_update_relation (GTK_ACCESSIBLE (zenity_value->forms_widget),
			GTK_ACCESSIBLE_RELATION_LABELLED_BY,
			label,
			NULL,
			-1);

		++i;
	}

	if (data->timeout_delay > 0)
	{
		ZENITY_UTIL_SETUP_TIMEOUT (dialog)
	}

	zenity_util_gapp_main (GTK_WINDOW(dialog));
	zenity_util_show_dialog (dialog);
}

static void
zenity_forms_dialog_output (ZenityFormsData *forms_data)
{
	GSList *tmp;

	for (tmp = forms_data->list; tmp; tmp = tmp->next)
	{
		ZenityFormsValue *zenity_value = tmp->data;

		switch (zenity_value->type)
		{
			case ZENITY_FORMS_PASSWORD:
			case ZENITY_FORMS_ENTRY:
				g_print ("%s",
						gtk_entry_buffer_get_text (gtk_entry_get_buffer
							(GTK_ENTRY(zenity_value->forms_widget))));
				break;

			case ZENITY_FORMS_LIST:
			{
				g_autoptr(GPtrArray) arr = g_ptr_array_new_with_free_func (g_free);
				GtkSelectionModel *selection = zenity_tree_column_view_get_selection_model (ZENITY_TREE_COLUMN_VIEW(zenity_value->forms_widget));

				for (guint i = 0; i < g_list_model_get_n_items (G_LIST_MODEL(selection)); ++i)
				{
					g_autoptr(ZenityTreeRow) row = g_list_model_get_item (G_LIST_MODEL(selection), i);

					if (! gtk_selection_model_is_selected (selection, i))
						continue;

					for (guint j = 0; j < zenity_tree_row_get_n_items (row); ++j)
					{
						ZenityTreeItem *item = zenity_tree_row_get_item (row, j);
						const char *text = zenity_tree_item_get_text (item);

						g_ptr_array_add (arr, g_strdup (text));
					}
				}

				for (guint i = 0; i < arr->len; ++i)
				{
					const char *text = g_ptr_array_index (arr, i);

					if (i == arr->len - 1)
						g_print ("%s", text);
					else
						g_print ("%s,", text);
				}
			}
				break;

			case ZENITY_FORMS_CALENDAR:
			{
				g_autoptr(GDateTime) date = gtk_calendar_get_date (GTK_CALENDAR(zenity_value->forms_widget));
				g_autofree char *date_string = NULL;

				date_string = g_date_time_format (date, forms_data->date_format);

				g_print ("%s", date_string);
			}
				break;

			case ZENITY_FORMS_COMBO:
			{
				GtkStringObject *strobj = gtk_drop_down_get_selected_item (GTK_DROP_DOWN(zenity_value->forms_widget));

				if (strobj)
					g_print ("%s", gtk_string_object_get_string (strobj));
				else
					g_print (" ");
			}
				break;

			case ZENITY_FORMS_MULTLINE_ENTRY:
			{
				g_autofree char *text = NULL;
				GtkWidget *tv = gtk_scrolled_window_get_child (GTK_SCROLLED_WINDOW(zenity_value->forms_widget));
				GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(tv));
				GtkTextIter start, end;

				gtk_text_buffer_get_bounds (buffer, &start, &end);
				text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
				g_print ("%s", text);
			}
				break;

		}
		if (tmp->next != NULL)
			g_print ("%s", forms_data->separator);
	}
	g_print ("\n");
}

static void
zenity_forms_dialog_response (GtkWidget *widget, char *rstr, gpointer data)
{
	ZenityFormsData *forms_data = data;
	int response = zenity_util_parse_dialog_response (rstr);

	switch (response)
	{
		case ZENITY_OK:
			zenity_forms_dialog_output (forms_data);
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
			break;

		case ZENITY_CANCEL:
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_CANCEL);
			break;

		case ZENITY_TIMEOUT:
			zenity_forms_dialog_output (forms_data);
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
