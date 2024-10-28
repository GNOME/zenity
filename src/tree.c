/* vim: colorcolumn=80 ts=4 sw=4
 */
/*
 * tree.c
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
 * Original Authors: Glynn Foster <glynn.foster@sun.com>
 *          Jonathan Blanford <jrb@redhat.com>
 *          Kristian Rietveld <kris@gtk.org>
 */


#include "util.h"
#include "zenity.h"
#include "zenity-tree-column-view.h"

#include <stdlib.h>
#include <string.h>

#include <config.h>

#define PRINT_HIDE_COLUMN_SEPARATOR ","

static ZenityTreeColumnView *col_view;
static gboolean editable;
static GSList *selected;
static char *separator;
static gboolean print_all_columns = FALSE;
static int *print_columns = NULL;
static int *hide_columns = NULL;
static GIOChannel *channel;

static int *zenity_tree_extract_column_indexes (char *indexes, int n_columns);
static void zenity_tree_dialog_response (GtkWidget *widget, char *rstr, gpointer data);
static void zenity_tree_dialog_output (void);

static void
show_mid_search_deprecation_warning (void)
{
	g_printerr (_("Warning: --mid-search is deprecated and will be removed in a "
			"future version of zenity. Ignoring.\n"));
}
static gboolean
zenity_tree_handle_stdin (GIOChannel *channel, GIOCondition condition, gpointer data)
{
	static int column_count = 0;
	static int row_count = 0;
	GIOStatus status = G_IO_STATUS_NORMAL;
	int n_columns = zenity_tree_column_view_get_n_columns (col_view);
	gboolean toggles = FALSE;
	ZenityTreeListType list_type = zenity_tree_column_view_get_list_type (col_view);
	GListStore *store = G_LIST_STORE(zenity_tree_column_view_get_model (col_view));

	if (list_type == ZENITY_TREE_LIST_RADIO || list_type == ZENITY_TREE_LIST_CHECK)
		toggles = TRUE;

	if ((condition & G_IO_IN) == G_IO_IN)
	{
		static ZenityTreeRow *row = NULL;
		g_autoptr(GString) gstring = g_string_new (NULL);
		g_autoptr(GError) error = NULL;

		while ((g_io_channel_get_flags (channel) & G_IO_FLAG_IS_READABLE) !=
			G_IO_FLAG_IS_READABLE)
			;

		if (!row)
			row = zenity_tree_row_new ();
		do {
			ZenityTreeItem *item;

			do {
				if (g_io_channel_get_flags (channel) & G_IO_FLAG_IS_READABLE) {
					status = g_io_channel_read_line_string (channel, gstring, NULL, &error);
					g_strchomp (gstring->str);
				}
				else {
					return FALSE;
				}

				while (g_main_context_pending (NULL)) {
					g_main_context_iteration (NULL, FALSE);
				}
				/* FIXME: Find a better way to avoid 100% cpu utilization */
				g_usleep (10000);

			} while (status == G_IO_STATUS_AGAIN);

			if (status != G_IO_STATUS_NORMAL)
			{
				if (error) {
					g_warning ("%s: %s", __func__, error->message);
					g_clear_error (&error);
				}
				continue;
			}

			if (column_count == n_columns)
			{
				/* We're starting a new row */
				column_count = 0;
				row_count++;
				if (row)
					g_list_store_append (store, row);
				row = zenity_tree_row_new ();
			}

			if (toggles && column_count == 0)
			{
				item = zenity_tree_item_new (gstring->str, gtk_check_button_new ());
			}
			else if (list_type == ZENITY_TREE_LIST_IMAGE && column_count == 0)
			{
				item = zenity_tree_item_new (gstring->str, gtk_image_new ());
			}
			else
			{
				if (editable)
					item = zenity_tree_item_new (gstring->str, gtk_editable_label_new (NULL));
				else
					item = zenity_tree_item_new (gstring->str, gtk_label_new (NULL));
			}

			zenity_tree_row_add (row, item);
			column_count++;

		} while ((g_io_channel_get_buffer_condition (channel) & G_IO_IN) == G_IO_IN &&
			status != G_IO_STATUS_EOF);	/* !do while */

		if (row)
			g_list_store_append (store, row);
	}

	if ((condition & G_IO_IN) != G_IO_IN || status == G_IO_STATUS_EOF)
	{
		g_io_channel_shutdown (channel, TRUE, NULL);
		return FALSE;
	}
	return TRUE;
}

static void
zenity_tree_fill_entries_from_stdin (void)
{
	channel = g_io_channel_unix_new (0);
	g_io_channel_set_encoding (channel, NULL, NULL);
	g_io_channel_set_flags (channel, G_IO_FLAG_NONBLOCK, NULL);
	g_io_add_watch (channel, G_IO_IN | G_IO_HUP, zenity_tree_handle_stdin, NULL);
}

static void
zenity_tree_fill_entries (char **args)
{
	guint num_args = g_strv_length (args);
	int n_columns = zenity_tree_column_view_get_n_columns (col_view);
	ZenityTreeListType list_type = zenity_tree_column_view_get_list_type (col_view);
	GListStore *store = G_LIST_STORE(zenity_tree_column_view_get_model (col_view));
	gboolean toggles = FALSE;
	ZenityTreeRow *row = NULL;

	if (list_type == ZENITY_TREE_LIST_RADIO || list_type == ZENITY_TREE_LIST_CHECK)
		toggles = TRUE;

	for (guint i = 0; i < num_args; ++i)
	{
		ZenityTreeItem *item = NULL;
		gboolean beginning_of_column = i % n_columns == 0;
		gboolean end_of_column = (i + 1) % n_columns == 0;
		const char *str = args[i];

		if (beginning_of_column)
			row = zenity_tree_row_new ();

		if (beginning_of_column && toggles)
		{
			item = zenity_tree_item_new (str, gtk_check_button_new ());
		}
		else if (beginning_of_column && list_type == ZENITY_TREE_LIST_IMAGE)
		{
			item = zenity_tree_item_new (str, gtk_image_new ());
		}
		else
		{
			if (editable)
				item = zenity_tree_item_new (str, gtk_editable_label_new (""));
			else
				item = zenity_tree_item_new (str, gtk_label_new (NULL));
		}

		zenity_tree_row_add (row, item);

		if (end_of_column)
			g_list_store_append (store, row);
	}
}

static void
cv_activated_cb (ZenityTreeColumnView *cv, gpointer data)
{
	GtkWindow *parent;
	ZenityData *zen_data = data;

	zenity_tree_dialog_output ();
	zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);

	parent = GTK_WINDOW(gtk_widget_get_native (GTK_WIDGET(col_view)));

	zenity_util_gapp_quit (parent, zen_data);
}

static void
search_changed_cb (GtkSearchEntry *entry, gpointer data)
{
	zenity_tree_column_view_set_search (col_view,
			gtk_editable_get_text (GTK_EDITABLE(entry)));
}

void
zenity_tree (ZenityData *data, ZenityTreeData *tree_data)
{
	g_autoptr(GtkBuilder) builder = NULL;
	GtkWidget *dialog;
	GtkWidget *search_entry;
	GObject *text;
	GListStore *store;
	ZenityTreeListType list_type;
	GSList *tmp;
	int n_columns;

	if (tree_data->mid_search_DEPRECATED)
		show_mid_search_deprecation_warning ();

	builder = zenity_util_load_ui_file ("zenity_tree_dialog", "zenity_tree_box", NULL);

	if (builder == NULL) {
		data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
		return;
	}

	separator = g_strcompress (tree_data->separator);
	n_columns = g_slist_length (tree_data->columns);

	/* Sanity checks */

	if (n_columns == 0) {
		g_printerr (_("No column titles specified for List dialog.\n"));
		data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
		return;
	}

	if ((tree_data->radiobox || tree_data->checkbox) && n_columns < 2)
	{
		/* Translators: --checklist and --radiolist should not be translated
		 * (command-line options). */
		g_printerr (_("Insufficient columns specified for List dialog (at least "
			"2 are required for --checklist or --radiolist).\n"));
		data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
		return;
	}

	if (tree_data->checkbox + tree_data->radiobox + tree_data->imagebox > 1)
	{
		g_printerr (_ ("You should use only one List dialog type.\n"));
		data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
		return;
	}

	if (tree_data->print_column)
	{
		if (strcmp (g_ascii_strdown (tree_data->print_column, -1), "all") == 0)
			print_all_columns = TRUE;
		else
			print_columns = zenity_tree_extract_column_indexes (
				tree_data->print_column, n_columns);
	}
	else
	{
		print_columns = g_new (gint, 2);
		print_columns[0] = (tree_data->radiobox || tree_data->checkbox ? 2 : 1);
		print_columns[1] = 0;
	}

	if (tree_data->hide_column) {
		hide_columns =
			zenity_tree_extract_column_indexes (tree_data->hide_column,
					n_columns);
	}

	dialog = GTK_WIDGET(gtk_builder_get_object (builder, "zenity_tree_dialog"));
	g_signal_connect (dialog, "response", G_CALLBACK(zenity_tree_dialog_response), data);

	zenity_util_setup_dialog_title (dialog, data);

	if (data->modal)
		gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

	if (data->extra_label)
	{
		ZENITY_UTIL_ADD_EXTRA_LABELS (dialog)
	}

	if (data->ok_label) {
		ZENITY_UTIL_SETUP_OK_BUTTON_LABEL (dialog)
	}

	if (data->cancel_label) {
		ZENITY_UTIL_SETUP_CANCEL_BUTTON_LABEL (dialog)
	}

	text = gtk_builder_get_object (builder, "zenity_tree_text");

	if (tree_data->dialog_text)
		gtk_label_set_markup (GTK_LABEL(text),
				g_strcompress (tree_data->dialog_text));

	gtk_window_set_icon_name (GTK_WINDOW(dialog),
			"view-sort-ascending");

	if (data->width > -1 || data->height > -1) {
		gtk_window_set_default_size (GTK_WINDOW(dialog),
				data->width, data->height);
	}

	/* Create a list store */
	store = g_list_store_new (ZENITY_TREE_TYPE_ROW);

	/* Define global col_view */
	col_view = ZENITY_TREE_COLUMN_VIEW(gtk_builder_get_object (builder, "zenity_tree_cv"));

	/* This signal will only get emitted for certain kinds of lists, but if
	 * applicable, it should finish the job.
	 */
	g_signal_connect (col_view, "activated", G_CALLBACK(cv_activated_cb), data);

	search_entry = GTK_WIDGET(gtk_builder_get_object (builder, "zenity_tree_search_entry"));
	g_signal_connect (search_entry, "search-changed", G_CALLBACK(search_changed_cb), NULL);

	if (tree_data->radiobox) {
		list_type = ZENITY_TREE_LIST_RADIO;
	}
	else if (tree_data->checkbox) {
		list_type = ZENITY_TREE_LIST_CHECK;
	}
	else if (tree_data->imagebox) {
		list_type = ZENITY_TREE_LIST_IMAGE;
	}
	else {
		list_type = ZENITY_TREE_LIST_NONE;
	}

	editable = tree_data->editable;
	g_object_set (col_view,
			"list-type", list_type,
			"multi", tree_data->multi,
			"model", G_LIST_MODEL(store),
			"hide-header", tree_data->hide_header,
			NULL);

	for (tmp = tree_data->columns; tmp; tmp = tmp->next)
	{
		zenity_tree_column_view_add_column (col_view, tmp->data);
	}

	if (hide_columns)
	{
		for (guint i = 0; hide_columns[i] != 0; i++)
		{
			zenity_tree_column_view_show_column (col_view, hide_columns[i]-1, FALSE);
		}
	}

	if (tree_data->data && *tree_data->data)	/* we have argv after opts */
	{
		zenity_tree_fill_entries (tree_data->data);
	}
	else
	{
		zenity_tree_fill_entries_from_stdin ();
	}

	zenity_util_show_dialog (dialog);

	if (data->timeout_delay > 0)
	{
		ZENITY_UTIL_SETUP_TIMEOUT (dialog)
	}

	zenity_util_gapp_main (GTK_WINDOW(dialog));
}

static gboolean
get_row_checkbtn_toggled (ZenityTreeRow *row)
{
	ZenityTreeItem *item = zenity_tree_row_get_item (row, 0);
	GtkWidget *child = zenity_tree_item_get_child (item);

	if (GTK_IS_CHECK_BUTTON (child))
	{
		return gtk_check_button_get_active (GTK_CHECK_BUTTON(child));
	}

	return FALSE;
}

static void
zenity_tree_dialog_get_selected (void)
{
	int n_columns = zenity_tree_column_view_get_n_columns (col_view);
	GListModel *model = G_LIST_MODEL(zenity_tree_column_view_get_selection_model (col_view));
	ZenityTreeItem *item;

	for (guint i = 0; i < g_list_model_get_n_items (model); ++i)
	{
		ZenityTreeRow *row;

		if (! zenity_tree_column_view_is_selected (col_view, i))
			continue;

		row = g_list_model_get_item (model, i);

		if (print_all_columns)
		{
			for (int j = 0; j < n_columns; j++)
			{
				item = zenity_tree_row_get_item (row, j);

				if (!item) continue;

				selected = g_slist_append (selected,
						g_strdup (zenity_tree_item_get_text (item)));
			}
		}
		else
		{
			for (int j = 0; print_columns[j] != 0; j++)
			{
				/* columns in CLI count from 1 and are allocated as 0-terminated
				 * arrays, so account for that here with -1. */
				item = zenity_tree_row_get_item (row, print_columns[j] - 1);

				if (!item) continue;

				selected = g_slist_append (selected,
						g_strdup (zenity_tree_item_get_text (item)));
			}
		}
	}
}

static void
zenity_tree_dialog_toggle_get_selected (ZenityTreeRow *row, gpointer unused)
{
	int n_columns;
	ZenityTreeItem *item;

	if (! get_row_checkbtn_toggled (row))
		return;

	n_columns = zenity_tree_column_view_get_n_columns (col_view);

	if (print_all_columns)
	{
		/* start at 1 because we're not printing the checklist column string
		 */
		for (int i = 1; i < n_columns; i++)
		{
			item = zenity_tree_row_get_item (row, i);

			if (!item) continue;

			selected = g_slist_append (selected,
					g_strdup (zenity_tree_item_get_text (item)));
		}
	}
	else
	{
		for (int i = 0; print_columns[i] != 0; i++)
		{
			/* columns in CLI count from 1 and are allocated as 0-terminated
			 * arrays, so account for that here with -1. */
			item = zenity_tree_row_get_item (row, print_columns[i] - 1);

			if (!item) continue;
			
			selected = g_slist_append (selected,
					g_strdup (zenity_tree_item_get_text (item)));
		}
	}
}

static void
zenity_tree_dialog_output (void)
{
	ZenityTreeListType list_type = zenity_tree_column_view_get_list_type (col_view);

	if (list_type == ZENITY_TREE_LIST_RADIO || list_type == ZENITY_TREE_LIST_CHECK)
	{
		zenity_tree_column_view_foreach_row (col_view,
				(GFunc)zenity_tree_dialog_toggle_get_selected, NULL);
	}
	else
	{
		zenity_tree_dialog_get_selected ();
	}

	for (GSList *tmp = selected; tmp != NULL; tmp = tmp->next)
	{
		if (tmp->next != NULL) {
			g_print ("%s%s", (char *)tmp->data, separator);
		} else {
			g_print ("%s\n", (char *)tmp->data);
		}
	}

	g_free (print_columns);
	g_free (hide_columns);
	g_free (separator);
	g_slist_free_full (g_steal_pointer (&selected), g_free);
}

static void
zenity_tree_dialog_response (GtkWidget *widget, char *rstr, gpointer data)
{
	ZenityData *zen_data = data;
	int response = zenity_util_parse_dialog_response (rstr);

	switch (response)
	{
		case ZENITY_OK:
			zenity_tree_dialog_output ();
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
			break;

		case ZENITY_CANCEL:
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_CANCEL);
			break;

		case ZENITY_ESC:
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_ESC);
			break;

		case ZENITY_TIMEOUT:
			zenity_tree_dialog_output ();
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_TIMEOUT);
			break;

		default:
			ZENITY_UTIL_RESPONSE_HANDLE_EXTRA_BUTTONS
			break;
	}
	if (channel != NULL &&
		g_io_channel_get_flags (channel) & G_IO_FLAG_IS_READABLE)
	{
		g_io_channel_shutdown (channel, TRUE, NULL);
	}

	zenity_util_gapp_quit (GTK_WINDOW(widget), zen_data);
}

static int *
zenity_tree_extract_column_indexes (char *indexes, int n_columns)
{
	g_auto(GStrv) tmp;
	int *result;
	int i, j, index;

	tmp = g_strsplit (indexes, PRINT_HIDE_COLUMN_SEPARATOR, 0);

	result = g_new (int, 1);

	for (j = i = 0; tmp[i] != NULL; i++)
	{
		index = atoi (tmp[i]);

		if (index > 0 && index <= n_columns)
		{
			result[j] = index;
			j++;
			result = g_renew (int, result, j + 1);
		}
	}
	result[j] = 0;

	return result;
}
