/* vim: colorcolumn=80 ts=4 sw=4
 */
/*
 * color.c
 *
 * Copyright © 2010 Berislav Kovacki
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
 * Authors: Berislav Kovacki <pantokrator@pantokrator.net>
 */

#include "config.h"

#include "util.h"
#include "zenity.h"
#include <string.h>

G_GNUC_BEGIN_IGNORE_DEPRECATIONS

static ZenityData *zen_data;

static void zenity_colorselection_dialog_response (GtkWidget *widget, int response, gpointer data);

static void
setup_custom_button (GtkDialog *dialog, int response, const char *label)
{
	GtkButton *button = GTK_BUTTON (gtk_dialog_get_widget_for_response (dialog, response));

	gtk_button_set_label (button, label);
}

void
zenity_colorselection (ZenityData *data, ZenityColorData *color_data)
{
	GtkWidget *dialog;
	GdkRGBA color;

	zen_data = data;

	dialog = gtk_color_chooser_dialog_new (data->dialog_title, NULL);

	g_signal_connect (dialog, "response", G_CALLBACK (zenity_colorselection_dialog_response), color_data);

	if (color_data->color &&
			gdk_rgba_parse (&color, color_data->color))
	{
		gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER(dialog), &color);
	}

	if (data->ok_label)
		setup_custom_button (GTK_DIALOG(dialog), GTK_RESPONSE_OK, data->ok_label);

	if (data->cancel_label)
		setup_custom_button (GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL, data->cancel_label);

	if (data->extra_label)
	{
		ZENITY_UTIL_ADD_EXTRA_LABELS (dialog)
	}

	if (data->modal)
		gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

	g_object_set (dialog, "show-editor", !color_data->show_palette, NULL);

	zenity_util_show_dialog (dialog);

	if (data->timeout_delay > 0)
	{
		ZENITY_UTIL_SETUP_TIMEOUT (dialog)
	}

	zenity_util_gapp_main (GTK_WINDOW(dialog));
}

static void
zenity_colorselection_dialog_response (GtkWidget *widget, int response, gpointer data)
{
	GdkRGBA color;

	switch (response)
	{
		case GTK_RESPONSE_OK:
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
			gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (widget), &color);
			g_print ("%s\n", gdk_rgba_to_string (&color));
			break;

		case GTK_RESPONSE_CANCEL:
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_CANCEL);
			break;

		case GTK_RESPONSE_DELETE_EVENT:
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_ESC);
			break;

		default:
			ZENITY_UTIL_RESPONSE_HANDLE_EXTRA_BUTTONS
			break;
	}
	zenity_util_gapp_quit (GTK_WINDOW(widget), zen_data);
}

G_GNUC_END_IGNORE_DEPRECATIONS
