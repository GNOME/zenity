/*
 * tree.c
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
 *          Jonathan Blanford <jrb@redhat.com>
 *          Kristian Rietveld <kris@gtk.org>
 */

#include <glade/glade.h>
#include "zenity.h"
#include "util.h"

#define MAX_ELEMENTS_BEFORE_SCROLLING 8

static GladeXML *glade_dialog;

enum
{
	NAME_COLUMN,
	DESCRIPTION_COLUMN,
	N_COLUMNS
};

void zenity_tree_dialog_response (GtkWindow *window, int button, gpointer data);

static void
zenity_tree_toggled_callback (GtkCellRendererToggle *cell, gchar *path_string, gpointer data)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *path;
	gboolean value;

	model = GTK_TREE_MODEL (data);
	path = gtk_tree_path_new_from_string (path_string);

	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get (model, &iter, NAME_COLUMN, &value, -1);

	value = !value;
	gtk_list_store_set (GTK_LIST_STORE (model), &iter, NAME_COLUMN, value, -1);

	gtk_tree_path_free (path);
}

static gboolean
count_rows_foreach (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
	gint *rows = data;

	(*rows)++;
	return FALSE;
}

static void
zenity_tree_fill_entries (GtkTreeView *tree_view, const **argv)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gint i = 0;

	model = gtk_tree_view_get_model (tree_view);
	gtk_tree_model_foreach (model, count_rows_foreach, &i);

	while (i<10) {	
	gtk_list_store_append (GTK_LIST_STORE (model), &iter);
	gtk_list_store_set (GTK_LIST_STORE (model), &iter, 
			    NAME_COLUMN, "TRUE", 
			    DESCRIPTION_COLUMN, "This is the bar above foobar", 
			    -1);

	if (i == MAX_ELEMENTS_BEFORE_SCROLLING) {
		GtkWidget *scrolled_window;
		GtkRequisition rectangle;

		gtk_widget_size_request (GTK_WIDGET (tree_view), &rectangle);
		scrolled_window = glade_xml_get_widget (glade_dialog, "zenity_tree_window");
		gtk_widget_set_size_request (scrolled_window, -1, rectangle.height);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
						GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	}
	i++;   
	}
}

int 
zenity_tree (ZenityData *data, ZenityTreeData *tree_data)
{
	GtkWidget *dialog;
	GtkWidget *tree_view;
	GtkTreeViewColumn *column;
	GtkListStore *model;

	glade_dialog = zenity_util_load_glade_file ("zenity_tree_dialog");

	if (glade_dialog == NULL)
		return FALSE;
	
	glade_xml_signal_autoconnect (glade_dialog);

	dialog = glade_xml_get_widget (glade_dialog, "zenity_tree_dialog");

	if (data->dialog_title)
		gtk_window_set_title (GTK_WINDOW (dialog), data->dialog_title);

	if (data->window_icon)
		zenity_util_set_window_icon (dialog, data->window_icon);
	else
		zenity_util_set_window_icon (dialog, ZENITY_IMAGE_FULLPATH ("zenity-list.png"));

	tree_view = glade_xml_get_widget (glade_dialog, "zenity_tree_view");

	if (tree_data->checkbox || tree_data->radiobox)
		model = gtk_list_store_new (N_COLUMNS, G_TYPE_BOOLEAN, G_TYPE_STRING);
	else 
		model = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);

	gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (model));

	if (tree_data->checkbox) {
		GtkCellRenderer *cell_renderer;

		cell_renderer = gtk_cell_renderer_toggle_new ();
		g_signal_connect (cell_renderer, "toggled", 
				  G_CALLBACK (zenity_tree_toggled_callback), model);

		column = gtk_tree_view_column_new_with_attributes (NULL,
							   	   cell_renderer,
							           "active", NAME_COLUMN, NULL);
	}
	else if (tree_data->radiobox) {
		GtkCellRenderer *cell_renderer;

		cell_renderer = gtk_cell_renderer_toggle_new ();
		g_object_set (G_OBJECT (cell_renderer), "radio", TRUE, NULL);
		g_signal_connect (cell_renderer, "toggled", 
				  G_CALLBACK (zenity_tree_toggled_callback), model);

		column = gtk_tree_view_column_new_with_attributes (NULL,
							   	   cell_renderer,
							           "active", NAME_COLUMN, NULL);

	}
	else {
		column = gtk_tree_view_column_new_with_attributes (tree_data->column_one_header, 
							   	   gtk_cell_renderer_text_new (),
							           "text", NAME_COLUMN, NULL);
		gtk_tree_view_column_set_sort_column_id (column, NAME_COLUMN);
	}

	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
	
	column = gtk_tree_view_column_new_with_attributes (tree_data->column_two_header,
							   gtk_cell_renderer_text_new (),
							   "text", DESCRIPTION_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id (column, DESCRIPTION_COLUMN);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (tree_view), TRUE);

	zenity_tree_fill_entries (GTK_TREE_VIEW (tree_view), NULL);

	gtk_widget_show (dialog);
	gtk_main ();

	if (glade_dialog)
		g_object_unref (glade_dialog);

	return TRUE;
}

void
zenity_tree_dialog_response (GtkWindow *window, int button, gpointer data)
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
