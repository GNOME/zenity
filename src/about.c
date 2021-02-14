/* vim: colorcolumn=80 ts=4 sw=4
 */
/*
 * about.c
 *
 * Copyright © 2002 Sun Microsystems, Inc.
 * Copyright © 2001 CodeFactory AB
 * Copyright © 2001, 2002 Anders Carlsson
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
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 * Authors: Glynn Foster <glynn.foster@sun.com>
 *          Anders Carlsson <andersca@gnu.org>
 */

#include "util.h"
#include "zenity.h"

#include <string.h>

#include <config.h>

#define GTK_RESPONSE_CREDITS 0

#define ZENITY_CANVAS_X 400.0
#define ZENITY_CANVAS_Y 280.0

static GtkWidget *dialog;

static void zenity_about_dialog_response (GtkWidget *widget,
		int response, gpointer data);

/* Sync with the people in the THANKS file */
static const gchar *const authors[] = {"Glynn Foster <glynn foster sun com>",
	"Lucas Rocha <lucasr gnome org>",
	"Mike Newman <mikegtn gnome org>",
	NULL};

static const char *documenters[] = {"Glynn Foster <glynn.foster@sun.com>",
	"Lucas Rocha <lucasr@gnome.org>",
	"Java Desktop System Documentation Team",
	"GNOME Documentation Project",
	NULL};

static gchar *translators;

static const char *license[] = {
	N_ ("This program is free software; you can redistribute it and/or modify "
		"it under the terms of the GNU Lesser General Public License as "
		"published by "
		"the Free Software Foundation; either version 2 of the License, or "
		"(at your option) any later version.\n"),
	N_ ("This program is distributed in the hope that it will be useful, "
		"but WITHOUT ANY WARRANTY; without even the implied warranty of "
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
		"GNU Lesser General Public License for more details.\n"),
	N_ ("You should have received a copy of the GNU Lesser General Public "
		"License "
		"along with this program; if not, write to the Free Software "
		"Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA "
		"02110-1301, USA.")};

void
zenity_about (ZenityData *data)
{
	char *license_trans;

	translators = _("translator-credits");

	license_trans = g_strconcat (
		_(license[0]), "\n", _(license[1]), "\n", _(license[2]), "\n", NULL);

	dialog = gtk_about_dialog_new ();

	g_object_set (G_OBJECT (dialog),
		"name",
		"Zenity",
		"version",
		VERSION,
		"copyright",
		"Copyright \xc2\xa9 2003 Sun Microsystems\n"
		"Copyright \xc2\xa9 2021 Logan Rathbone\n",
		"comments",
		_("Display dialog boxes from shell scripts"),
		"authors",
		authors,
		"documenters",
		documenters,
		"translator-credits",
		translators,
		"website",
		"https://gitlab.gnome.org/GNOME/zenity",
		"wrap-license",
		TRUE,
		"license",
		license_trans,
		NULL);

	g_free (license_trans);

	zenity_util_set_window_icon (dialog,
			NULL, ZENITY_IMAGE_FULLPATH ("zenity.png"));

	g_signal_connect (G_OBJECT (dialog),
		"response",
		G_CALLBACK (zenity_about_dialog_response),
		data);

	zenity_util_show_dialog (dialog);
	zenity_util_gapp_main (GTK_WINDOW (dialog));
}

static void
zenity_about_dialog_response (GtkWidget *widget, int response, gpointer data)
{
	ZenityData *zen_data = data;

	g_return_if_fail (GTK_IS_WINDOW (GTK_WINDOW(widget)));

	switch (response) {
		case GTK_RESPONSE_CLOSE:
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
			break;

		default:
			/* Esc dialog */
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_ESC);
			break;
	}
	zenity_util_gapp_quit (GTK_WINDOW(widget));
}
