/*
 * about.c
 *
 * Copyright (C) 2002 Sun Microsystems, Inc.
 * Copyright (C) 2001 CodeFactory AB
 * Copyright (C) 2001, 2002 Anders Carlsson
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
 *          Anders Carlsson <andersca@gnu.org>
 */

#include "config.h"
#include "util.h"
#include "zenity.h"
#include <gdk/gdkkeysyms.h>
#include <string.h>

#define GTK_RESPONSE_CREDITS 0
#define ZENITY_HELP_PATH ZENITY_DATADIR "/help/"

static GtkWidget *dialog;

static void zenity_about_dialog_response (
	GtkWidget *widget, int response, gpointer data);

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
zenity_about (ZenityData *data) {
	GdkPixbuf *logo;
	char *license_trans;

	translators = _ ("translator-credits");
	logo =
		gdk_pixbuf_new_from_file (ZENITY_IMAGE_FULLPATH ("zenity.png"), NULL);

	license_trans = g_strconcat (
		_ (license[0]), "\n", _ (license[1]), "\n", _ (license[2]), "\n", NULL);

	dialog = gtk_about_dialog_new ();

	g_object_set (G_OBJECT (dialog),
		"name",
		"Zenity",
		"version",
		VERSION,
		"copyright",
		"Copyright \xc2\xa9 2003 Sun Microsystems",
		"comments",
		_ ("Display dialog boxes from shell scripts"),
		"authors",
		authors,
		"documenters",
		documenters,
		"translator-credits",
		translators,
		"website",
		"http://live.gnome.org/Zenity",
		"logo",
		logo,
		"wrap-license",
		TRUE,
		"license",
		license_trans,
		NULL);

	g_free (license_trans);

	zenity_util_set_window_icon (
		dialog, NULL, ZENITY_IMAGE_FULLPATH ("zenity.png"));

	g_signal_connect (G_OBJECT (dialog),
		"response",
		G_CALLBACK (zenity_about_dialog_response),
		data);

	zenity_util_show_dialog (dialog, data->attach);
	gtk_main ();
}

static void
zenity_about_dialog_response (GtkWidget *widget, int response, gpointer data) {
	ZenityData *zen_data = data;

	switch (response) {
		case GTK_RESPONSE_CLOSE:
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
			break;

		default:
			/* Esc dialog */
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_ESC);
			break;
	}

	gtk_main_quit ();
}
