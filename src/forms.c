/* vim: colorcolumn=80 ts=4 sw=4
 */
/*
 * forms.c
 *
 * Copyright © 2010 Arx Cruz
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
 * Free Software Foundation, Inc., 121 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 * Original Author: Arx Cruz <arxcruz@gnome.org>
 */

#include "util.h"
#include "zenity.h"

#include <string.h>

#include <config.h>

/* TODO: Many items in this source file need to be ported away from deprecated
 * API in GTK 4.10+. Suppress the whirlwind of warnings for now since we're
 * well-aware of this.
 */
G_GNUC_BEGIN_IGNORE_DEPRECATIONS

static ZenityData *zen_data;
static GSList *selected;
static void zenity_forms_dialog_response (GtkWidget *widget, char *rstr, gpointer data);

static void
zenity_forms_dialog_get_selected (GtkTreeModel *model, GtkTreePath *path_buf,
	GtkTreeIter *iter, gpointer data)
{
	int n_columns = 0;
	GValue value = G_VALUE_INIT;
	GtkTreeView *tree_view = GTK_TREE_VIEW(data);

	g_return_if_fail (GTK_IS_TREE_VIEW (tree_view));

	n_columns = gtk_tree_model_get_n_columns (model);

	for (int i = 0; i < n_columns; ++i)
	{
		gtk_tree_model_get_value (model, iter, i, &value);
		selected = g_slist_append (selected, g_value_dup_string (&value));
		g_value_unset (&value);
	}
}

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
			g_auto(GStrv) row_values = g_strsplit_set (combo_values, "|", -1);

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
	g_autoptr(GtkListStore) list_store = NULL;
	GtkWidget *tree_view;
	GtkWidget *scrolled_window;
	GType *column_types = NULL;

	int i = 0;
	/* If no column names available, default is one */
	int n_columns = 1;
	int column_index = 0;

	tree_view = gtk_tree_view_new ();

	if (forms_data->column_values)
	{
		char *column_values;
		int columns_values_count = g_slist_length (forms_data->column_values);
		int column_number = 0;

		if (list_number < columns_values_count) {
			column_number = list_number;
		}

		column_values = g_slist_nth_data (forms_data->column_values,
				column_number);

		if (column_values)
		{
			char **values = g_strsplit_set (column_values, "|", -1);

			if (values)
			{
				n_columns = g_strv_length (values);
				column_types = g_new (GType, n_columns);

				for (i = 0; i < n_columns; i++)
					column_types[i] = G_TYPE_STRING;

				for (i = 0; i < n_columns; i++)
				{
					GtkCellRenderer *renderer;
					GtkTreeViewColumn *column;
					char *column_name = values[i];

					renderer = gtk_cell_renderer_text_new ();
					column =
						gtk_tree_view_column_new_with_attributes (column_name,
								renderer,
								"text", column_index,
								NULL);
					gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view),
							column);
					column_index++;
				}
			}
		}
		else
		{
			/* If no values available, add one with string type*/
			column_types = g_new (GType, n_columns);
			column_types[0] = G_TYPE_STRING;
		}
	}

	list_store = g_object_new (GTK_TYPE_LIST_STORE, NULL);

	gtk_list_store_set_column_types (list_store, n_columns, column_types);

	if (forms_data->list_values)
	{
		char *list_values =
			g_slist_nth_data (forms_data->list_values, list_number);

		if (list_values)
		{
			char **row_values = g_strsplit_set (list_values, "|", -1);

			if (row_values)
			{
				GtkTreeIter iter;
				char *row = row_values[0];
				int position = -1;
				i = 0;

				while (row != NULL)
				{
					if (position >= n_columns || position == -1)
					{
						position = 0;
						gtk_list_store_append (list_store, &iter);
					}
					gtk_list_store_set (list_store, &iter, position, row, -1);
					position++;
					row = row_values[++i];
				}
				g_strfreev (row_values);
			}
		}
	}

	gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view),
			GTK_TREE_MODEL (list_store));
	gtk_tree_view_set_headers_visible (
		GTK_TREE_VIEW (tree_view), forms_data->show_header);

	scrolled_window = gtk_scrolled_window_new ();
	gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW(scrolled_window),
			tree_view);
	gtk_widget_set_size_request (scrolled_window, -1, 100);

	return scrolled_window;
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

	zenity_util_show_dialog (dialog);

	if (data->timeout_delay > 0)
	{
		ZENITY_UTIL_SETUP_TIMEOUT (dialog)
	}

	zenity_util_gapp_main (GTK_WINDOW(dialog));
}

static void
zenity_forms_dialog_output (ZenityFormsData *forms_data)
{
	GSList *tmp, *tmp2;
	guint day, year, month;
	g_autoptr(GDate) date = NULL;
	char time_string[128];
	GtkTreeSelection *selection;

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
				selection =
					gtk_tree_view_get_selection
						(GTK_TREE_VIEW(gtk_scrolled_window_get_child
						   (GTK_SCROLLED_WINDOW(zenity_value->forms_widget))));

				gtk_tree_selection_selected_foreach (selection,
					zenity_forms_dialog_get_selected,
					GTK_TREE_VIEW(gtk_scrolled_window_get_child
						(GTK_SCROLLED_WINDOW(zenity_value->forms_widget))));

				for (tmp2 = selected; tmp2; tmp2 = tmp2->next)
				{
					if (tmp->next != NULL) {
						g_print ("%s,", (char *)tmp2->data);
					} else
						g_print ("%s", (char *)tmp2->data);
				}

				g_slist_free_full (g_steal_pointer (&selected), g_free);
				break;

			case ZENITY_FORMS_CALENDAR:
				g_object_get (zenity_value->forms_widget,
						"day", &day,
						"month", &month,
						"year", &year,
						NULL);
				date = g_date_new_dmy (day, month + 1, year);
				g_date_strftime (time_string, sizeof time_string, forms_data->date_format, date);
				g_print ("%s", time_string);
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

G_GNUC_END_IGNORE_DEPRECATIONS
