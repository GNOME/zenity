/*
 * forms.c
 *
 * Copyright (C) 2010 Arx Cruz
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
 * Free Software Foundation, Inc., 121 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 * Authors: Arx Cruz <arxcruz@gnome.org>
 */

#include "config.h"
#include <string.h>
#include "zenity.h"
#include "util.h"

static ZenityData *zen_data;
static GSList *selected;
static void zenity_forms_dialog_response (GtkWidget *widget, int response, gpointer data);

static void zenity_forms_dialog_get_selected (GtkTreeModel *model, GtkTreePath *path_buf, GtkTreeIter *iter, GtkTreeView *tree_view) 
{
  gint n_columns = 0;
  gint i = 0;

  n_columns = gtk_tree_model_get_n_columns (model);
  GValue value = {0, };
  for (i = 0; i < n_columns; i++) {
    gtk_tree_model_get_value (model, iter, i, &value);
    selected = g_slist_append (selected, g_value_dup_string (&value));
    g_value_unset (&value);
  }
}

static void zenity_forms_dialog_output (void)
{
  GSList *tmp;

  for (tmp = selected; tmp; tmp = tmp->next) {
    if (tmp->next != NULL) {
        g_print ("%s,", (gchar *) tmp->data);
    }
    else
      g_print ("%s", (gchar *) tmp->data);
  }

  g_slist_foreach (selected, (GFunc) g_free, NULL);
  selected = NULL;
}

static GtkWidget * 
zenity_forms_create_and_fill_list (ZenityFormsData        *forms_data, 
                                           int list_number, gchar *header)
{
  GtkListStore *list_store;
  GtkWidget *tree_view;
  GtkWidget *scrolled_window;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GType *column_types = NULL;
  gchar *list_values;
  gchar *column_values;

  gint i = 0;
  /* If no column names available, default is one */
  gint n_columns = 1;
  gint column_index = 0;

  tree_view = gtk_tree_view_new ();

  if (forms_data->column_values) {
    column_values = g_slist_nth_data (forms_data->column_values, list_number);
    if (column_values) {
      gchar **values = g_strsplit_set (column_values, "|", -1);
      if (values) {
        n_columns = g_strv_length (values);
        column_types = g_new (GType, n_columns);
        for (i = 0; i < n_columns; i++)
          column_types[i] = G_TYPE_STRING;

        for (i = 0; i < n_columns; i++) {  
          gchar *column_name = values[i];
          renderer = gtk_cell_renderer_text_new ();
          column = gtk_tree_view_column_new_with_attributes (column_name,
                                                             renderer,
                                                             "text", column_index,
                                                             NULL);
          gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
          column_index++;
        }
      }
    }
  }

  list_store = g_object_new (GTK_TYPE_LIST_STORE, NULL);

  gtk_list_store_set_column_types (list_store, n_columns, column_types);

  if (forms_data->list_values) {
    list_values = g_slist_nth_data (forms_data->list_values, list_number);
    if (list_values) {
      gchar **row_values = g_strsplit_set (list_values, "|", -1);
      if (row_values) {
        GtkTreeIter iter;
        gchar *row = row_values[0];
        gint position = -1;
        i = 0;
        
        while (row != NULL) {
          if (position >= n_columns || position == -1) {
            position = 0;
            gtk_list_store_append (list_store, &iter);
          }
          gtk_list_store_set (list_store, &iter, position, row, -1);
          position++;
          row = row_values[++i];
        }
        g_strfreev (row_values);
      }
      g_free (list_values);
    }
  }

  gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (list_store));
  g_object_unref (list_store);
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  //gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_window), 
  //                                       GTK_WIDGET (tree_view));
  gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (tree_view));
  gtk_widget_set_size_request (GTK_WIDGET (scrolled_window), -1, 100);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree_view), forms_data->show_header);

  return scrolled_window;  
}

void zenity_forms_dialog (ZenityData *data, ZenityFormsData *forms_data)
{
  GtkBuilder *builder = NULL;
  GtkWidget *dialog;
  GtkWidget *table;
  GtkWidget *text;
  GtkWidget *button;

  GSList *tmp;

  gint number_of_widgets = g_slist_length (forms_data->list);
  int list_count = 0;

  zen_data = data;

  builder = zenity_util_load_ui_file("zenity_forms_dialog", NULL);

  if (builder == NULL) {
    data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
    return;
  }

  gtk_builder_connect_signals(builder, NULL);
  
  dialog = GTK_WIDGET (gtk_builder_get_object (builder, "zenity_forms_dialog"));

  g_signal_connect (G_OBJECT(dialog), "response",
                    G_CALLBACK (zenity_forms_dialog_response), forms_data);
  
  if (data->dialog_title)
    gtk_window_set_title (GTK_WINDOW (dialog), data->dialog_title);

  if (data->width > -1 || data->height > -1)
    gtk_window_set_default_size (GTK_WINDOW (dialog), data->width, data->height);

  if (data->ok_label) {
    button = GTK_WIDGET (gtk_builder_get_object (builder, "zenity_forms_ok_button"));
    gtk_button_set_label (GTK_BUTTON (button), data->ok_label);
    gtk_button_set_image (GTK_BUTTON (button),
                          gtk_image_new_from_stock (GTK_STOCK_OK, GTK_ICON_SIZE_BUTTON));
  }  

  if (data->cancel_label) {
    button = GTK_WIDGET (gtk_builder_get_object (builder, "zenity_forms_cancel_button"));
    gtk_button_set_label (GTK_BUTTON (button), data->cancel_label);
    gtk_button_set_image (GTK_BUTTON (button),
                          gtk_image_new_from_stock (GTK_STOCK_CANCEL, GTK_ICON_SIZE_BUTTON));
  }

  text = GTK_WIDGET (gtk_builder_get_object (builder, "zenity_forms_text"));

  if (forms_data->dialog_text)
    gtk_label_set_markup (GTK_LABEL (text), g_strcompress (forms_data->dialog_text));
  
  table = GTK_WIDGET (gtk_builder_get_object (builder, "zenity_forms_table"));

  gtk_table_resize (GTK_TABLE (table), number_of_widgets, 2);

  int i = 0;

  for (tmp = forms_data->list; tmp; tmp = tmp->next) {
    ZenityFormsValue *zenity_value = (ZenityFormsValue *) tmp->data;
    GtkWidget *label;
    GtkWidget *align;
    
    label = gtk_label_new(zenity_value->option_value);
    
    align = gtk_alignment_new (0.0, 0.5, 0.0, 0.0);
    gtk_container_add (GTK_CONTAINER (align), label);

    gtk_table_attach (GTK_TABLE (table), 
                      GTK_WIDGET (align),
                      0,
                      1,
                      i,
                      i+1,
                      GTK_FILL,
                      GTK_FILL,
                      0,
                      0);
                      
    switch(zenity_value->type)
    {
      case ZENITY_FORMS_ENTRY:
        zenity_value->forms_widget = gtk_entry_new();
        gtk_table_attach (GTK_TABLE (table),
                      GTK_WIDGET (zenity_value->forms_widget),
                      1,
                      2,
                      i,
                      i+1,
                      GTK_EXPAND | GTK_FILL,
                      GTK_EXPAND | GTK_FILL,
                      0,
                      0);
        break;
      case ZENITY_FORMS_PASSWORD:
        zenity_value->forms_widget = gtk_entry_new();
        gtk_entry_set_visibility(GTK_ENTRY(zenity_value->forms_widget), 
                                 FALSE);
        gtk_table_attach (GTK_TABLE (table),
                      GTK_WIDGET (zenity_value->forms_widget),
                      1,
                      2,
                      i,
                      i+1,
                      GTK_EXPAND | GTK_FILL,
                      GTK_EXPAND | GTK_FILL,
                      0,
                      0);
        break;
      case ZENITY_FORMS_CALENDAR:
        zenity_value->forms_widget = gtk_calendar_new();
        gtk_alignment_set (GTK_ALIGNMENT (align), 0.0, 0.02, 0.0, 0.0);
        align = gtk_alignment_new (0.0, 0.5, 0.0, 0.0);
        gtk_container_add (GTK_CONTAINER (align), zenity_value->forms_widget);
        gtk_table_attach (GTK_TABLE (table),
                      GTK_WIDGET (align),
                      1,
                      2,
                      i,
                      i+1,
                      GTK_FILL,
                      GTK_FILL,
                      0,
                      0);
        break;
      case ZENITY_FORMS_LIST:
          zenity_value->forms_widget = zenity_forms_create_and_fill_list (forms_data, list_count,
                                                                          zenity_value->option_value);
          gtk_alignment_set (GTK_ALIGNMENT (align), 0.0, 0.02, 0.0, 0.0);
          gtk_table_attach (GTK_TABLE (table),
                      GTK_WIDGET (zenity_value->forms_widget),
                      1,
                      2,
                      i,
                      i+1,
                      GTK_EXPAND | GTK_FILL,
                      GTK_EXPAND | GTK_FILL,
                      0,
                      0);
          list_count++;                                                                           
        break;
      default:
        zenity_value->forms_widget = gtk_entry_new();
        gtk_table_attach (GTK_TABLE (table),
                      GTK_WIDGET (zenity_value->forms_widget),
                      1,
                      2,
                      i,
                      i+1,
                      GTK_EXPAND | GTK_FILL,
                      GTK_EXPAND | GTK_FILL,
                      0,
                      0);
        break;
    }
    
    i++;
  }

  gtk_widget_show_all (GTK_WIDGET (dialog));
  
  g_object_unref (builder);

  if (data->timeout_delay > 0) {
    g_timeout_add_seconds (data->timeout_delay, (GSourceFunc) zenity_util_timeout_handle, dialog);
  }

  gtk_main();
}

static void
zenity_forms_dialog_response (GtkWidget *widget, int response, gpointer data)
{
  ZenityFormsData *forms_data = (ZenityFormsData *) data;
  GSList *tmp;
  guint day, year, month;
  GDate *date = NULL;
  gchar time_string[128]; 
  GtkTreeSelection *selection;

  switch (response) {
    case GTK_RESPONSE_OK:
      zenity_util_exit_code_with_data(ZENITY_OK, zen_data);      
      for (tmp = forms_data->list; tmp; tmp = tmp->next) {
        ZenityFormsValue *zenity_value = (ZenityFormsValue *) tmp->data;
        switch (zenity_value->type) {
          case ZENITY_FORMS_PASSWORD:
          case ZENITY_FORMS_ENTRY:
            g_print("%s", gtk_entry_get_text (GTK_ENTRY (zenity_value->forms_widget)));
            break;
          case ZENITY_FORMS_LIST:
            selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (gtk_bin_get_child (GTK_BIN (zenity_value->forms_widget))));
            gtk_tree_selection_selected_foreach (selection,
                                             (GtkTreeSelectionForeachFunc) zenity_forms_dialog_get_selected,
                                             GTK_TREE_VIEW (gtk_bin_get_child (GTK_BIN (zenity_value->forms_widget))));
            zenity_forms_dialog_output ();
            break;
          case ZENITY_FORMS_CALENDAR:
            gtk_calendar_get_date (GTK_CALENDAR (zenity_value->forms_widget), &day, &month, &year);
            date = g_date_new_dmy (year, month + 1, day);
            g_date_strftime (time_string, 127, forms_data->date_format, date);
            g_print ("%s", time_string);
            break;
        }
        if (tmp->next != NULL)
          g_print("%s", forms_data->separator);
      }
      g_print("\n");

      break;
    case GTK_RESPONSE_CANCEL:
      zen_data->exit_code = zenity_util_return_exit_code (ZENITY_CANCEL);
      break;

    default:
      zen_data->exit_code = zenity_util_return_exit_code (ZENITY_ESC);
      break;
  }

  gtk_main_quit ();
}
