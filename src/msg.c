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

#include <glade/glade.h>
#include "zenity.h"
#include "util.h"

static void zenity_msg_dialog_response (GtkWidget *widget, int response, gpointer data);

void 
zenity_msg (ZenityData *data, ZenityMsgData *msg_data)
{
  GladeXML *glade_dialog;
  GtkWidget *dialog;
  GtkWidget *text;

  switch (msg_data->mode) {
    case ZENITY_MSG_WARNING:
      glade_dialog = zenity_util_load_glade_file ("zenity_warning_dialog");
      dialog = glade_xml_get_widget (glade_dialog, "zenity_warning_dialog");
      text = glade_xml_get_widget (glade_dialog, "zenity_warning_text");
      break;

    case ZENITY_MSG_QUESTION:
      glade_dialog = zenity_util_load_glade_file ("zenity_question_dialog");
      dialog = glade_xml_get_widget (glade_dialog, "zenity_question_dialog");
      text = glade_xml_get_widget (glade_dialog, "zenity_question_text");
      break;

    case ZENITY_MSG_ERROR:
      glade_dialog = zenity_util_load_glade_file ("zenity_error_dialog");
      dialog = glade_xml_get_widget (glade_dialog, "zenity_error_dialog");
      text = glade_xml_get_widget (glade_dialog, "zenity_error_text");
      break;

    case ZENITY_MSG_INFO:
      glade_dialog = zenity_util_load_glade_file ("zenity_info_dialog");
      dialog= glade_xml_get_widget (glade_dialog, "zenity_info_dialog");
      text = glade_xml_get_widget (glade_dialog, "zenity_info_text");
      break;
		
    default:
      glade_dialog = NULL;
      dialog = NULL;
      text = NULL;
      g_assert_not_reached ();
      break;	
  }

  g_signal_connect (G_OBJECT (dialog), "response",
                    G_CALLBACK (zenity_msg_dialog_response), data);
	
  if (glade_dialog == NULL) {
    data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
    return;
  }
	
  glade_xml_signal_autoconnect (glade_dialog);
        
  if (glade_dialog)
    g_object_unref (glade_dialog);

  if (data->dialog_title)
    gtk_window_set_title (GTK_WINDOW (dialog), data->dialog_title);

  switch (msg_data->mode) {
    case ZENITY_MSG_WARNING:
      zenity_util_set_window_icon_from_stock (dialog, data->window_icon, GTK_STOCK_DIALOG_WARNING);
      break;

    case ZENITY_MSG_QUESTION:
      zenity_util_set_window_icon_from_stock (dialog, data->window_icon, GTK_STOCK_DIALOG_QUESTION);
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
        
  if (msg_data->dialog_text) 
    gtk_label_set_markup (GTK_LABEL (text), g_strcompress (msg_data->dialog_text));
    
  if (msg_data->no_wrap)
    gtk_label_set_line_wrap (GTK_LABEL (text), FALSE);

  zenity_util_show_dialog (dialog);

  if(data->timeout_delay > 0) {
    g_timeout_add (data->timeout_delay * 1000, (GSourceFunc) zenity_util_timeout_handle, NULL);
  }

  gtk_main ();
}

static void
zenity_msg_dialog_response (GtkWidget *widget, int response, gpointer data)
{
  ZenityData *zen_data = data;

  switch (response) {
    case GTK_RESPONSE_OK:
      zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
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
