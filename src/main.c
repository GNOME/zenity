/* vim: colorcolumn=80 ts=4 sw=4
 */
/*
 * main.c
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
 * Original Author: Glynn Foster <glynn.foster@sun.com>
 */

#include "option.h"
#include "zenity.h"

#include <adwaita.h>
#include <locale.h>
#include <stdlib.h>

#include <config.h>

static int
local_options_cb (GApplication *application,
		GVariantDict *options,
		gpointer user_data)
{
	ZenityParsingOptions *results = user_data;

	if (results->mode == MODE_VERSION)
	{
		g_print ("%s\n", VERSION);
		return 0;
	}

	return -1;
}

static int
command_line_cb (GApplication *app,
               GApplicationCommandLine *command_line,
               gpointer user_data)
{
	g_auto(GStrv) argv = g_application_command_line_get_arguments (command_line, NULL);
	ZenityParsingOptions *results = user_data;

	switch (results->mode)
	{
		case MODE_CALENDAR:
			zenity_calendar (results->data, results->calendar_data);
			break;

		case MODE_ENTRY:
			/* allow for a series of tokens (or even a bash array!) to be
			 * passed as arguments so as to auto-populate the entry with
			 * a list of options as a combo-box.
			 */
			results->entry_data->data = g_strdupv (argv + 1);
			zenity_entry (results->data, results->entry_data);
			break;

		case MODE_ERROR:
		case MODE_QUESTION:
		case MODE_WARNING:
		case MODE_INFO:
			zenity_msg (results->data, results->msg_data);
			break;

		case MODE_SCALE:
			zenity_scale (results->data, results->scale_data);
			break;

		case MODE_FILE:
			zenity_fileselection (results->data, results->file_data);
			break;

		case MODE_LIST:
			results->tree_data->data = g_strdupv (argv + 1);
			zenity_tree (results->data, results->tree_data);
			break;
			
		case MODE_NOTIFICATION:
			zenity_notification (results->data, results->notification_data);
			break;

		case MODE_PROGRESS:
			zenity_progress (results->data, results->progress_data);
			break;

		case MODE_TEXTINFO:
			zenity_text (results->data, results->text_data);
			break;

		case MODE_COLOR:
			zenity_colorselection (results->data, results->color_data);
			break;

		case MODE_PASSWORD:
			zenity_password_dialog (results->data, results->password_data);
			break;

		case MODE_ABOUT:
			zenity_about (results->data);
			break;

		case MODE_FORMS:
			zenity_forms_dialog (results->data, results->forms_data);
			break;

		case MODE_LAST:
			g_printerr (_ ("You must specify a dialog type. See 'zenity "
						   "--help' for details\n"));
			exit (-1);

		default:
			g_assert_not_reached ();
			exit (-1);
	}

	return 0;
}

static void dummy_log_func (void) { }

int
main (int argc, char *argv[])
{
	g_autoptr(AdwApplication) app = NULL;
	int status;
	ZenityParsingOptions *results;

	/* <i18n> */
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
                                                                                
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
	/* </i18n> */

	/* Turn off g_message's from libadwaita - this is to suppress the 'this is
	 * discouraged' message re: mapping dialogs without a transient parent.
	 */
	g_log_set_handler ("Adwaita", G_LOG_LEVEL_MESSAGE, (GLogFunc)dummy_log_func, NULL);

	results = zenity_option_parse (argc, argv);

	app = adw_application_new (APP_ID, G_APPLICATION_HANDLES_COMMAND_LINE | G_APPLICATION_NON_UNIQUE);
	g_signal_connect (app, "handle-local-options", G_CALLBACK(local_options_cb), results);
	g_signal_connect (app, "command-line", G_CALLBACK(command_line_cb), results);

	status = g_application_run (G_APPLICATION(app), argc, argv);

	return status;
}
