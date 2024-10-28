/* vim: colorcolumn=80 ts=4 sw=4
 */
/*
 * password.c
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
 * Authors: Arx Cruz <arxcruz@gmail.com>
 */

#include "util.h"
#include "zenity.h"

#include <string.h>

#include <config.h>

static ZenityData *zen_data;

static void zenity_password_dialog_response (GtkWidget *widget, char *rstr, gpointer data);

void
zenity_password_dialog (ZenityData *data, ZenityPasswordData *password_data)
{
	GtkBuilder *builder;
	GtkWidget *dialog;
	GtkWidget *grid;
	GtkWidget *label;
	int pass_row = 0;

	/* Set global */
	zen_data = data;

	builder = zenity_util_load_ui_file ("zenity_password_dialog", "zenity_password_box", NULL);

	if (builder == NULL)
	{
		data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
		return;
	}

	dialog = GTK_WIDGET(gtk_builder_get_object (builder,
				"zenity_password_dialog"));

	if (data->extra_label)
	{
		ZENITY_UTIL_ADD_EXTRA_LABELS (dialog)
	}

	if (data->ok_label) {
		ZENITY_UTIL_SETUP_OK_BUTTON_LABEL (dialog);
	}

	if (data->cancel_label)
	{
		ZENITY_UTIL_SETUP_CANCEL_BUTTON_LABEL (dialog);
	}

	grid = GTK_WIDGET(gtk_builder_get_object (builder,
				"zenity_password_grid"));

	/* Checks if username has been passed as a parameter */
	if (password_data->username)
	{
		/* Change the password label to ask for both username and password */
		label = GTK_WIDGET(gtk_builder_get_object (builder,
					"zenity_password_title"));
		gtk_label_set_text (GTK_LABEL(label),
				_("Type your username and password"));

		/* Add the username label and entry and increment the row for the
		 * password entry so it will be added below the username.
		 */
		label = gtk_label_new (_("Username:"));
		gtk_grid_attach (GTK_GRID(grid), label,
				0,		/* col */
				0,		/* row */
				1, 1);	/* width/height by cell. */	

		password_data->entry_username = gtk_entry_new ();
		gtk_grid_attach (GTK_GRID(grid), password_data->entry_username,
				1,
				0,
				1, 1);

		++pass_row;
	}

	label = gtk_label_new (_("Password:"));
	gtk_grid_attach (GTK_GRID(grid), label,
			0,		/* col */
			pass_row,		/* row */
			1, 1);	/* width/height by cell. */	

	password_data->entry_password = gtk_entry_new ();
	gtk_entry_set_visibility (GTK_ENTRY(password_data->entry_password), FALSE);
	gtk_entry_set_input_purpose (GTK_ENTRY(password_data->entry_password),
			GTK_INPUT_PURPOSE_PASSWORD);
	gtk_entry_set_activates_default (GTK_ENTRY(password_data->entry_password),
			TRUE);
	gtk_grid_attach (GTK_GRID(grid), password_data->entry_password,
			1,
			pass_row,
			1, 1);

	zenity_util_setup_dialog_title (dialog, data);

	if (data->modal)
		gtk_window_set_modal (GTK_WINDOW(dialog), TRUE);

	g_signal_connect (dialog, "response", G_CALLBACK(zenity_password_dialog_response), password_data);

	zenity_util_show_dialog (dialog);

	if (data->timeout_delay > 0)
	{
		ZENITY_UTIL_SETUP_TIMEOUT (dialog)
	}

	zenity_util_gapp_main (GTK_WINDOW(dialog));
}

static void
zenity_password_dialog_response (GtkWidget *widget, char *rstr, gpointer data)
{
	ZenityPasswordData *password_data = data;
	GtkEntryBuffer *user_buff = NULL, *pass_buff = NULL;
	int response = zenity_util_parse_dialog_response (rstr);

	if (password_data->username)
		user_buff = gtk_entry_get_buffer (GTK_ENTRY(password_data->entry_username));

	pass_buff = gtk_entry_get_buffer (GTK_ENTRY(password_data->entry_password));

	switch (response)
	{
		case ZENITY_OK:
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
			if (password_data->username) {
				g_print ("%s|%s\n",
					gtk_entry_buffer_get_text (user_buff),
					gtk_entry_buffer_get_text (pass_buff));
			}
			else {
				g_print ("%s\n",
					gtk_entry_buffer_get_text (pass_buff));
			}
			break;

		case ZENITY_CANCEL:
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_CANCEL);
			break;

		case ZENITY_ESC:
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_ESC);
			break;

		case ZENITY_TIMEOUT:
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_TIMEOUT);
			break;

		default:
			ZENITY_UTIL_RESPONSE_HANDLE_EXTRA_BUTTONS
			break;
	}
	zenity_util_gapp_quit (GTK_WINDOW(widget), zen_data);
}
