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
#include <string.h>
#include "zenity.h"
#include "util.h"

#define MAX_ELEMENTS_BEFORE_SCROLLING 8

static GladeXML *glade_dialog;
static GSList *selected;
static gchar *separator;

static void zenity_tree_dialog_response (GtkWidget *widget, int response, gpointer data);

static gboolean
zenity_tree_dialog_untoggle (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
  GValue toggle_value = {0, };

  gtk_tree_model_get_value (model, iter, 0, &toggle_value);

  if (g_value_get_boolean (&toggle_value))
    gtk_list_store_set (GTK_LIST_STORE (model), iter, 0, FALSE, -1);
}

static void
zenity_tree_toggled_callback (GtkCellRendererToggle *cell, gchar *path_string, gpointer data)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  GtkTreePath *path;
  gboolean value;

  model = GTK_TREE_MODEL (data);

  /* Because this is a radio list, we should untoggle the previous toggle so that 
   * we only have one selection at any given time
   */

  if (GPOINTER_TO_INT (g_object_get_data (G_OBJECT (model), "radio")) == 1) {
    gtk_tree_model_foreach (model, zenity_tree_dialog_untoggle, NULL);
  }

  path = gtk_tree_path_new_from_string (path_string);
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, 0, &value, -1);

  value = !value;
  gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, value, -1);

  gtk_tree_path_free (path);
}

static gboolean
zenity_tree_handle_stdin (GIOChannel  *channel,
                          GIOCondition condition,
                          gpointer     data)
{
  static GtkTreeView *tree_view;
  GtkTreeModel *model;
  GtkTreeIter iter;
  static gint column_count = 0;
  static gint row_count = 0;
  static gint n_columns;
  static gboolean editable;
  static gboolean toggles;

  tree_view = GTK_TREE_VIEW (data);
  n_columns = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (tree_view), "n_columns"));
  editable = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (tree_view), "editable"));
  toggles = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (tree_view), "toggles"));

  model = gtk_tree_view_get_model (tree_view);

  if ((condition == G_IO_IN) || (condition == G_IO_IN + G_IO_HUP)) {
    GString *string;
    GError *error = NULL;

    string = g_string_new (NULL);

    while (channel->is_readable != TRUE)
      ;
    do {
      gint status;

      do {
        status = g_io_channel_read_line_string (channel, string, NULL, &error);

        while (gtk_events_pending ())
          gtk_main_iteration ();

      } while (status == G_IO_STATUS_AGAIN);

      if (status != G_IO_STATUS_NORMAL) {
        if (error) {
          g_warning ("zenity_tree_handle_stdin () : %s", error->message);
          g_error_free (error);
          error = NULL;
        }
        continue;
      }
    
      if (column_count == n_columns) {
        /* We're starting a new row */
        column_count = 0;
        row_count++;
        gtk_list_store_append (GTK_LIST_STORE (model), &iter);
      }

      if (toggles && column_count == 0) {
        if (strcmp (string->str, "TRUE") == 0)
          gtk_list_store_set (GTK_LIST_STORE (model), &iter, column_count, TRUE, -1);
	else 
          gtk_list_store_set (GTK_LIST_STORE (model), &iter, column_count, FALSE, -1);
      }
      else {
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column_count, zenity_util_strip_newline (string->str), -1);	
      }

    if (editable) {
      g_print ("Shouldn't be going here");
      gtk_list_store_set (GTK_LIST_STORE (model), &iter, n_columns, TRUE, -1);
    }

    if (row_count == MAX_ELEMENTS_BEFORE_SCROLLING) {
      GtkWidget *scrolled_window;
      GtkRequisition rectangle;

      gtk_widget_size_request (GTK_WIDGET (tree_view), &rectangle);
      scrolled_window = glade_xml_get_widget (glade_dialog, "zenity_tree_window");
      gtk_widget_set_size_request (scrolled_window, -1, rectangle.height);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                      GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    }

    column_count ++;

    } while (g_io_channel_get_buffer_condition (channel) == G_IO_IN); 
    g_string_free (string, TRUE);
  }

  if (condition != G_IO_IN) {
    g_io_channel_shutdown (channel, TRUE, NULL);
    return FALSE;
  }
  return TRUE;
}

static void
zenity_tree_fill_entries_from_stdin (GtkTreeView  *tree_view,
                                     gint          n_columns,
                                     gboolean      toggles,
                                     gboolean      editable)
{
  GIOChannel *channel;

  g_object_set_data (G_OBJECT (tree_view), "n_columns", (gint *) n_columns);
  g_object_set_data (G_OBJECT (tree_view), "toggles", (gint *) toggles);
  g_object_set_data (G_OBJECT (tree_view), "editable", (gint *) editable); 

  channel = g_io_channel_unix_new (0);
  g_io_channel_set_encoding (channel, NULL, NULL);
  g_io_channel_set_flags (channel, G_IO_FLAG_NONBLOCK, NULL);
  g_io_add_watch (channel, G_IO_IN | G_IO_HUP, zenity_tree_handle_stdin, tree_view);
}

static void
zenity_tree_fill_entries (GtkTreeView  *tree_view, 
                          const gchar **args, 
                          gint          n_columns, 
                          gboolean      toggles, 
                          gboolean      editable)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  gint i = 0;

  model = gtk_tree_view_get_model (tree_view);

  while (args[i] != NULL) {
    gint j;

    gtk_list_store_append (GTK_LIST_STORE (model), &iter);

    for (j = 0; j < n_columns; j++) {
	
      if (toggles && j == 0) {
        if (strcmp (args[i+j], "TRUE") == 0)
          gtk_list_store_set (GTK_LIST_STORE (model), &iter, j, TRUE, -1);
	else 
          gtk_list_store_set (GTK_LIST_STORE (model), &iter, j, FALSE, -1);
      }
      else
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, j, args[i+j], -1);	
    }

    if (editable)
      gtk_list_store_set (GTK_LIST_STORE (model), &iter, n_columns, TRUE, -1);

    if (i == MAX_ELEMENTS_BEFORE_SCROLLING) {
      GtkWidget *scrolled_window;
      GtkRequisition rectangle;

      gtk_widget_size_request (GTK_WIDGET (tree_view), &rectangle);
      scrolled_window = glade_xml_get_widget (glade_dialog, "zenity_tree_window");
      gtk_widget_set_size_request (scrolled_window, -1, rectangle.height);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                      GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    }

  i += n_columns;
  }
}

static void
zenity_cell_edited_callback (GtkCellRendererText *cell, 
                             const gchar         *path_string, 
                             const gchar         *new_text, 
                             gpointer             data) 
{
  GtkTreeModel *model;
  GtkTreePath *path;
  GtkTreeIter iter;
  gint column;

  model = GTK_TREE_MODEL (data);
  path = gtk_tree_path_new_from_string (path_string);

  column = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (cell), "column"));
  gtk_tree_model_get_iter (model, &iter, path);
        
  gtk_list_store_set (GTK_LIST_STORE (model), &iter, column, new_text, -1);

  gtk_tree_path_free (path);
}

void
zenity_tree (ZenityData *data, ZenityTreeData *tree_data)
{
  GtkWidget *dialog;
  GtkWidget *tree_view;
  GtkTreeViewColumn *column;
  GtkListStore *model;
  GType *column_types;
  GSList *tmp;
  gboolean first_column = FALSE;
  gint i, column_index, n_columns;

  glade_dialog = zenity_util_load_glade_file ("zenity_tree_dialog");

  if (glade_dialog == NULL) {
    data->exit_code = -1;
    return;
  }
        
  separator = g_strdup (tree_data->separator);

  n_columns = g_slist_length (tree_data->columns);

  if (n_columns == 0) {
    g_printerr (_("No column titles specified for --list\n")); 
    data->exit_code = -1;
    return;
  }

  /*
  if (tree_data->data == NULL) {
    g_printerr (_("No contents specified for --list\n"));
    data->exit_code = -1;
    return;
  }
  */

  glade_xml_signal_autoconnect (glade_dialog);

  dialog = glade_xml_get_widget (glade_dialog, "zenity_tree_dialog");

  g_signal_connect (G_OBJECT (dialog), "response",
                    G_CALLBACK (zenity_tree_dialog_response), data);

  if (data->dialog_title)
    gtk_window_set_title (GTK_WINDOW (dialog), data->dialog_title);

  if (data->window_icon)
    zenity_util_set_window_icon (dialog, data->window_icon);
  else
    zenity_util_set_window_icon (dialog, ZENITY_IMAGE_FULLPATH ("zenity-list.png"));

  gtk_window_set_default_size (GTK_WINDOW (dialog), data->width, data->height);

  tree_view = glade_xml_get_widget (glade_dialog, "zenity_tree_view");

  /* Create an empty list store */
  model = g_object_new (GTK_TYPE_LIST_STORE, NULL);

  if (tree_data->editable)
    column_types = g_new (GType, n_columns + 1);
  else
    column_types = g_new (GType, n_columns);

  for (i = 0; i < n_columns; i++) {
    /* Have the limitation that the radioboxes and checkboxes are in the first column */
    if (i == 0 && (tree_data->checkbox || tree_data->radiobox))
      column_types[i] = G_TYPE_BOOLEAN;
    else
      column_types[i] = G_TYPE_STRING;
  }

  if (tree_data->editable)
    column_types[n_columns] = G_TYPE_BOOLEAN;

  if (tree_data->editable)
    gtk_list_store_set_column_types (model, n_columns + 1, column_types);
  else
    gtk_list_store_set_column_types (model, n_columns, column_types);

  gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (model));

  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view)),
                               GTK_SELECTION_MULTIPLE);

  column_index = 0;

  for (tmp = tree_data->columns; tmp; tmp = tmp->next) {
    if (!first_column) {
      if (tree_data->checkbox || tree_data->radiobox) {
        GtkCellRenderer *cell_renderer;
    
        cell_renderer = gtk_cell_renderer_toggle_new ();
				
        if (tree_data->radiobox) {
          g_object_set (G_OBJECT (cell_renderer), "radio", TRUE, NULL);
          g_object_set_data (G_OBJECT (model), "radio", (gint *) 1);
        }

	g_signal_connect (cell_renderer, "toggled",
                          G_CALLBACK (zenity_tree_toggled_callback), model);

        column = gtk_tree_view_column_new_with_attributes (tmp->data,
                                                           cell_renderer, 
                                                           "active", column_index, NULL);
      }
      else  {
        if (tree_data->editable) {
          GtkCellRenderer *cell_renderer;

          cell_renderer = gtk_cell_renderer_text_new ();
          g_signal_connect (G_OBJECT (cell_renderer), "edited",
                            G_CALLBACK (zenity_cell_edited_callback),
          gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view)));
          g_object_set_data (G_OBJECT (cell_renderer), "column", (gint *) column_index);

          column = gtk_tree_view_column_new_with_attributes (tmp->data, 
                                                             cell_renderer, 
                                                             "text", column_index, 
                                                             "editable", n_columns, 
                                                             NULL);
          } 
          else  {
            column = gtk_tree_view_column_new_with_attributes (tmp->data, 
                                                               gtk_cell_renderer_text_new (), 
                                                               "text", column_index, 
                                                               NULL);
          }

        gtk_tree_view_column_set_sort_column_id (column, column_index);
        gtk_tree_view_column_set_resizable (column, TRUE);
      }
			
      first_column = TRUE;
    }
    else {
      if (tree_data->editable) {
        GtkCellRenderer *cell_renderer;

        cell_renderer = gtk_cell_renderer_text_new ();
        g_signal_connect (G_OBJECT (cell_renderer), "edited",
                          G_CALLBACK (zenity_cell_edited_callback),
        gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view)));
        g_object_set_data (G_OBJECT (cell_renderer), "column", (gint *) column_index);

        column = gtk_tree_view_column_new_with_attributes (tmp->data, 
                                                           cell_renderer, 
                                                           "text", column_index, 
                                                           "editable", n_columns, 
                                                           NULL);
      }
      else {
        column = gtk_tree_view_column_new_with_attributes (tmp->data,
                                                           gtk_cell_renderer_text_new (), 
                                                           "text", column_index, NULL);
      }

      gtk_tree_view_column_set_sort_column_id (column, column_index);
      gtk_tree_view_column_set_resizable (column, TRUE);
    }
	
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
    column_index++;
  }

  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (tree_view), TRUE);

  if (tree_data->radiobox || tree_data->checkbox) {
    if (tree_data->data)
      zenity_tree_fill_entries (GTK_TREE_VIEW (tree_view), tree_data->data, n_columns, TRUE, tree_data->editable);
    else
      zenity_tree_fill_entries_from_stdin (GTK_TREE_VIEW (tree_view), n_columns, TRUE, tree_data->editable);
  }
  else {
    if (tree_data->data)
      zenity_tree_fill_entries (GTK_TREE_VIEW (tree_view), tree_data->data, n_columns, FALSE, tree_data->editable);
    else
      zenity_tree_fill_entries_from_stdin (GTK_TREE_VIEW (tree_view), n_columns, FALSE, tree_data->editable);
  }

  gtk_widget_show (dialog);
  gtk_main ();

  if (glade_dialog)
    g_object_unref (glade_dialog);
}

static void 
zenity_tree_dialog_get_selected (GtkTreeModel *model, GtkTreePath *path_buf, GtkTreeIter *iter, GtkTreeView *tree_view)
{
  GValue value = {0, };

  gtk_tree_model_get_value (model, iter, 0, &value);

  selected = g_slist_append (selected, g_strdup (g_value_get_string (&value)));
  g_value_unset (&value);
}

static gboolean
zenity_tree_dialog_toggle_get_selected (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
  GValue toggle_value = {0, };

  gtk_tree_model_get_value (model, iter, 0, &toggle_value);

  if (g_value_get_boolean (&toggle_value)) {
    GValue value = {0, };
    gtk_tree_model_get_value (model, iter, 1, &value);
    selected = g_slist_append (selected, g_strdup (g_value_get_string (&value)));
    g_value_unset (&value);
  }
  g_value_unset (&toggle_value);
  return FALSE;
}

static void
zenity_tree_dialog_output (void)
{
  GSList *tmp;

  for (tmp = selected; tmp; tmp = tmp->next) {
    if (tmp->next != NULL) {
      /* FIXME: There must be a nicer way to do this. This is just arse */
      if (strstr ((const gchar *) separator, (const gchar *) "\\n") != NULL)
        g_printerr ("%s\n", tmp->data);
      else if (strstr ((const gchar *) separator, (const gchar *) "\\t") != NULL)
        g_printerr ("%s\t", tmp->data);
      else
        g_printerr ("%s%s", tmp->data, separator);
    }
    else
      g_printerr ("%s\n", tmp->data);
  }

  g_free (separator);
  g_slist_foreach (selected, (GFunc) g_free, NULL);
  selected = NULL;
}

static void
zenity_tree_dialog_response (GtkWidget *widget, int response, gpointer data)
{
  ZenityData *zen_data = data;
  GtkWidget *tree_view;
  GtkTreeSelection *selection; 
  GtkTreeModel *model;

  switch (response) {
    case GTK_RESPONSE_OK:
      tree_view = glade_xml_get_widget (glade_dialog, "zenity_tree_view");
      model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

      if (gtk_tree_model_get_column_type (model, 0) == G_TYPE_BOOLEAN)
        gtk_tree_model_foreach (model, (GtkTreeModelForeachFunc) zenity_tree_dialog_toggle_get_selected, NULL);
      else {
        selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
        gtk_tree_selection_selected_foreach (selection, 
                                             (GtkTreeSelectionForeachFunc) zenity_tree_dialog_get_selected, 
                                             GTK_TREE_VIEW (tree_view));
      }
      zenity_tree_dialog_output ();
      zen_data->exit_code = 0;
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
