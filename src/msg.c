/*
 * msg.c
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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 * Authors: Glynn Foster <glynn.foster@sun.com>
 */

#include "config.h"

#include "zenity.h"
#include "util.h"

static void zenity_msg_dialog_response (GtkWidget *widget, int response, gpointer data);

static void
zenity_msg_construct_question_dialog (GtkWidget *dialog, ZenityMsgData *msg_data, ZenityData *data)
{
  GtkWidget *cancel_button, *ok_button;
  
  cancel_button = gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_NO, GTK_RESPONSE_CANCEL);
  ok_button = gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_YES, GTK_RESPONSE_OK);

  gtk_widget_grab_focus (ok_button);

  if (data->cancel_label) {
    gtk_button_set_label (GTK_BUTTON (cancel_button), data->cancel_label);
    gtk_button_set_image (GTK_BUTTON (cancel_button), 
                          gtk_image_new_from_stock (GTK_STOCK_CANCEL, GTK_ICON_SIZE_BUTTON));
  }

  if (data->ok_label) {
    gtk_button_set_label (GTK_BUTTON (ok_button), data->ok_label);
    gtk_button_set_image (GTK_BUTTON (ok_button), 
                          gtk_image_new_from_stock (GTK_STOCK_OK, GTK_ICON_SIZE_BUTTON));
  }
}

void 
zenity_msg (ZenityData *data, ZenityMsgData *msg_data)
{
  GtkBuilder *builder;
  GtkWidget *dialog;
  GtkWidget *ok_button;
  GObject *text;

  switch (msg_data->mode) {
    case ZENITY_MSG_WARNING:
      builder = zenity_util_load_ui_file ("zenity_warning_dialog", NULL);
      dialog = GTK_WIDGET (gtk_builder_get_object (builder, "zenity_warning_dialog"));
      text = gtk_builder_get_object (builder, "zenity_warning_text");
      ok_button = GTK_WIDGET (gtk_builder_get_object (builder, "zenity_warning_ok_button"));
      break;

    case ZENITY_MSG_QUESTION:
      builder = zenity_util_load_ui_file ("zenity_question_dialog", NULL);
      dialog = GTK_WIDGET (gtk_builder_get_object (builder, "zenity_question_dialog"));
      text = gtk_builder_get_object (builder, "zenity_question_text");
      ok_button = NULL;
      break;

    case ZENITY_MSG_ERROR:
      builder = zenity_util_load_ui_file ("zenity_error_dialog", NULL);
      dialog = GTK_WIDGET (gtk_builder_get_object (builder, "zenity_error_dialog"));
      text = gtk_builder_get_object (builder, "zenity_error_text");
      ok_button = GTK_WIDGET (gtk_builder_get_object (builder, "zenity_error_ok_button"));
      break;

    case ZENITY_MSG_INFO:
      builder = zenity_util_load_ui_file ("zenity_info_dialog", NULL);
      dialog = GTK_WIDGET (gtk_builder_get_object (builder, "zenity_info_dialog"));
      text = gtk_builder_get_object (builder, "zenity_info_text");
      ok_button = GTK_WIDGET (gtk_builder_get_object (builder, "zenity_info_ok_button"));
      break;
		
    default:
      builder = NULL;
      dialog = NULL;
      text = NULL;
      ok_button = NULL;
      g_assert_not_reached ();
      break;	
  }

  if (builder == NULL) {
    data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
    return;
  }

  g_signal_connect (G_OBJECT (dialog), "response",
                    G_CALLBACK (zenity_msg_dialog_response), data);

  gtk_builder_connect_signals (builder, NULL);
        
  if (data->dialog_title)
    gtk_window_set_title (GTK_WINDOW (dialog), data->dialog_title);

  if (ok_button) {
    if (data->ok_label) {
      gtk_button_set_label (GTK_BUTTON (ok_button), data->ok_label);
      gtk_button_set_image (GTK_BUTTON (ok_button),
                            gtk_image_new_from_stock (GTK_STOCK_OK, GTK_ICON_SIZE_BUTTON));
    }
  }

  switch (msg_data->mode) {
    case ZENITY_MSG_WARNING:
      zenity_util_set_window_icon_from_stock (dialog, data->window_icon, GTK_STOCK_DIALOG_WARNING);
      break;

    case ZENITY_MSG_QUESTION:
      zenity_util_set_window_icon_from_stock (dialog, data->window_icon, GTK_STOCK_DIALOG_QUESTION);
      zenity_msg_construct_question_dialog (dialog, msg_data, data);
      break;
      
    case ZENITY_MSG_ERROR:
      zenity_util_set_window_icon_from_stock (dialog, data->window_icon, GTK_STOCK_DIALOG_ERROR);
      break;
      
    case ZENITY_MSG_INFO:
      zenity_util_set_window_icon_from_stock (dialog, data->window_icon, GTK_STOCK_DIALOG_INFO);
      break;
  
    default:
      break;
  }
  
  if (data->width > -1 || data->height > -1)
    gtk_window_set_default_size (GTK_WINDOW (dialog), data->width, data->height);
        
  if (msg_data->dialog_text) {
    if (msg_data->no_markup)
      gtk_label_set_text (GTK_LABEL (text), msg_data->dialog_text);
    else 
      gtk_label_set_markup (GTK_LABEL (text), g_strcompress (msg_data->dialog_text));
  }
 
  if (msg_data->no_wrap)
    gtk_label_set_line_wrap (GTK_LABEL (text), FALSE);

  zenity_util_show_dialog (dialog);

  if(data->timeout_delay > 0) {
    g_timeout_add_seconds (data->timeout_delay, (GSourceFunc) zenity_util_timeout_handle, NULL);
  }

  g_object_unref (builder);

  gtk_main ();
}

static void
zenity_msg_dialog_response (GtkWidget *widget, int response, gpointer data)
{
  ZenityData *zen_data = data;

  switch (response) {
    case GTK_RESPONSE_OK:
      zenity_util_exit_code_with_data(ZENITY_OK, zen_data);      
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
