/* vim: colorcolumn=80 ts=4 sw=4
 */
/*
 * main.c
 *
 * Copyright © 2002 Sun Microsystems, Inc.
 * Copyright © 2021 Logan Rathbone
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

#include <gtk/gtk.h>
#include <locale.h>
#include <stdlib.h>

#include <config.h>

typedef struct {
	int argc;
	char **argv;
} ZenityArgs;

static void
activate_cb (GtkApplication *app, gpointer user_data)
{
	ZenityArgs *args = user_data;
	ZenityParsingOptions *results;
	int retval;

	results = zenity_option_parse (args->argc, args->argv);
	g_free (args);

	switch (results->mode)
	{
		case MODE_CALENDAR:
			zenity_calendar (results->data, results->calendar_data);
			break;

		case MODE_ENTRY:
			results->entry_data->data = (const char **) args->argv + 1;
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
			results->tree_data->data = (const char **) args->argv + 1;
			zenity_tree (results->data, results->tree_data);
			break;
			
#ifdef HAVE_LIBNOTIFY
		case MODE_NOTIFICATION:
			zenity_notification (results->data, results->notification_data);
			break;
#endif

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

		case MODE_VERSION:
			g_print ("%s\n", VERSION);
			break;

		case MODE_LAST:
			g_printerr (_ ("You must specify a dialog type. See 'zenity "
						   "--help' for details\n"));
			zenity_option_free ();
			exit (-1);

		default:
			g_assert_not_reached ();
			zenity_option_free ();
			exit (-1);
	}

	retval = results->data->exit_code;

	zenity_option_free ();

	// FIXME - pass retval to gapplication properly.
//	exit (retval);
}

int
main (int argc, char *argv[])
{
	ZenityArgs *args;
	GtkApplication *app;
	int status;

	/* boilerplate i18n stuff */                                                
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
                                                                                
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
	/* </i18n> */

	args = g_new (ZenityArgs, 1);
	args->argc = argc;
	args->argv = argv;

	app = gtk_application_new ("org.gnome.Zenity", G_APPLICATION_FLAGS_NONE);
	g_signal_connect (app, "activate",
			G_CALLBACK(activate_cb), args);
	status = g_application_run (G_APPLICATION(app), 0, NULL);
	g_object_unref (app);

	return status;
}
