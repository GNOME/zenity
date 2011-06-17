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
 * Authors: Arx Cruz <arxcruz@gmail.com>
 */

#include "config.h"
#include <string.h>
#include "zenity.h"
#include "util.h"

static ZenityData *zen_data;

static void zenity_forms_dialog_response (GtkWidget *widget, int response, gpointer data);

void zenity_forms_dialog (ZenityData *data, ZenityFormsData *forms_data)
{
  GtkBuilder *builder = NULL;
  GtkWidget *dialog;
  GtkWidget *table;
  GtkWidget *text;

  GSList *tmp;

  gint number_of_widgets = g_slist_length (forms_data->list);

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
