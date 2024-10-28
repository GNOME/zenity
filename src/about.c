/* vim: colorcolumn=80 ts=4 sw=4
 */
/*
 * about.c
 *
 * Copyright © 2002 Sun Microsystems, Inc.
 * Copyright © 2001 CodeFactory AB
 * Copyright © 2001, 2002 Anders Carlsson
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
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 * Original authors: Glynn Foster <glynn.foster@sun.com>
 *          Anders Carlsson <andersca@gnu.org>
 */

#include "util.h"
#include "zenity.h"

#include <string.h>

#include <config.h>

static GtkWidget *about_window;

static void zenity_about_close_cb (GtkWindow *window, gpointer data);

static const char *const developers[] = {"Glynn Foster",
	"Lucas Rocha",
	"Mike Newman",
	"Logan Rathbone <poprocks@gmail com>",
	NULL};

static const char *documenters[] = {"Glynn Foster",
	"Lucas Rocha",
	"Java Desktop System Documentation Team",
	"GNOME Documentation Project",
	"Logan Rathbone <poprocks@gmail.com>",
	NULL};

void
zenity_about (ZenityData *data)
{
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	about_window = adw_about_window_new ();
G_GNUC_END_IGNORE_DEPRECATIONS

	g_object_set (G_OBJECT (about_window),
		"application-name", "Zenity",
		"version", VERSION,
		"copyright",
			"Copyright \xc2\xa9 2003 Sun Microsystems\n"
			"Copyright \xc2\xa9 2021-2023 Logan Rathbone\n",
		"comments", _("Display dialog boxes from shell scripts"),
		"developers", developers,
		"documenters", documenters,
		"website", "https://gitlab.gnome.org/GNOME/zenity",
		"license-type", GTK_LICENSE_LGPL_2_1,
		"application-icon", "zenity",
		NULL);

	g_signal_connect (about_window, "close-request",
			G_CALLBACK(zenity_about_close_cb), data);

	zenity_util_show_dialog (about_window);
	zenity_util_gapp_main (GTK_WINDOW (about_window));
}

static void
zenity_about_close_cb (GtkWindow *window, gpointer data)
{
	ZenityData *zen_data = data;

	zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
	zenity_util_gapp_quit (window, zen_data);
}
