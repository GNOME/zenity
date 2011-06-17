/*
 * fileselection.c
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

#include <string.h>
#include "zenity.h"
#include "util.h"

static ZenityData	*zen_data;

static void zenity_fileselection_dialog_response (GtkWidget *widget, int response, gpointer data);

void zenity_fileselection (ZenityData *data, ZenityFileData *file_data)
{
  GtkWidget *dialog;
  gchar *dir;
  gchar *basename;
  GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;

  zen_data = data;

  if (file_data->directory) {
    if (file_data->save)
      action = GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER;
    else
      action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
  } else {
    if (file_data->save)
      action = GTK_FILE_CHOOSER_ACTION_SAVE;
  }

  dialog = gtk_file_chooser_dialog_new (NULL, NULL,
		  			action,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OK, GTK_RESPONSE_OK,
					NULL);

  gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), 
                                                  file_data->confirm_overwrite);

  g_signal_connect (G_OBJECT (dialog), "response", 
                    G_CALLBACK (zenity_fileselection_dialog_response), file_data);

  if (data->dialog_title)
    gtk_window_set_title (GTK_WINDOW (dialog), data->dialog_title);
	
  zenity_util_set_window_icon (dialog, data->window_icon, ZENITY_IMAGE_FULLPATH ("zenity-file.png"));

  if (file_data->uri) {
    dir = g_path_get_dirname (file_data->uri);

    if (g_path_is_absolute (file_data->uri) == TRUE)
      gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), dir);

    if (file_data->uri[strlen (file_data->uri) - 1] != '/') {
      basename = g_path_get_basename (file_data->uri);
      if (file_data->save)
        gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), basename);
      else
        (void) gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), file_data->uri);
      g_free (basename);
    }
    g_free (dir);
  }

  if (file_data->multi)
    gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (dialog), TRUE);

  if (file_data->filter) {
    /* Filter format: Executables | *.exe *.bat *.com */
    gint filter_i;

    for (filter_i = 0; file_data->filter [filter_i]; filter_i++) {
      GtkFileFilter *filter = gtk_file_filter_new();
      gchar *filter_str = file_data->filter [filter_i];
      gchar **pattern, **patterns;
      gchar *name = NULL;
      gint i;

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
        for (++i; filter_str[i] == ' '; i++);
      } else {
        gtk_file_filter_set_name (filter, filter_str);
        i = 0;
      }

      /* Get patterns */
      patterns = g_strsplit_set (filter_str + i, " ", -1);

      for (pattern = patterns; *pattern; pattern++)
        gtk_file_filter_add_pattern (filter, *pattern);

      if (name)
        g_free (name);

      g_strfreev (patterns);

      gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
    }
  }

  zenity_util_show_dialog (dialog);

  if(data->timeout_delay > 0) {
    g_timeout_add_seconds (data->timeout_delay, (GSourceFunc) zenity_util_timeout_handle, dialog);
  }

  gtk_main ();
}

static void
zenity_fileselection_dialog_response (GtkWidget *widget, int response, gpointer data)
{
  ZenityFileData *file_data = data;
  GSList *selections, *iter;
	  
  switch (response) {
    case GTK_RESPONSE_OK:
      zenity_util_exit_code_with_data(ZENITY_OK, zen_data);
      selections = gtk_file_chooser_get_filenames (GTK_FILE_CHOOSER (widget));
      for (iter = selections;iter != NULL; iter = iter->next) {
        g_print ("%s", g_filename_to_utf8 ((gchar*)iter->data, -1, NULL, NULL, NULL));
	g_free (iter->data);
	if (iter->next != NULL)
	    g_print ("%s",file_data->separator);
      }
      g_print("\n");
      g_slist_free(selections);
      break;

    case GTK_RESPONSE_CANCEL:
      zen_data->exit_code = zenity_util_return_exit_code (ZENITY_CANCEL);
      break;

    default:
      /* Esc dialog */
      zen_data->exit_code = zenity_util_return_exit_code (ZENITY_ESC);
      break;
  }
  gtk_main_quit ();
}
