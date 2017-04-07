/*
 * color.c
 *
 * Copyright (C) 2010 Berislav Kovacki
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
 * Authors: Berislav Kovacki <pantokrator@pantokrator.net>
 */

#include "config.h"

#include "util.h"
#include "zenity.h"
#include <string.h>

static ZenityData *zen_data;

static void zenity_colorselection_dialog_response (
	GtkWidget *widget, int response, gpointer data);

void
zenity_colorselection (ZenityData *data, ZenityColorData *color_data) {
	GtkWidget *dialog;
	GtkWidget *button;
	GdkRGBA color;

	zen_data = data;

	dialog = gtk_color_chooser_dialog_new (data->dialog_title, NULL);

	g_signal_connect (G_OBJECT (dialog),
		"response",
		G_CALLBACK (zenity_colorselection_dialog_response),
		color_data);

	if (color_data->color) {
		if (gdk_rgba_parse (&color, color_data->color)) {
			gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (dialog), &color);
		}
	}

	if (data->extra_label) {
		gint i = 0;
		while (data->extra_label[i] != NULL) {
			gtk_dialog_add_button (
				GTK_DIALOG (dialog), data->extra_label[i], i);
			i++;
		}
	}

	if (data->ok_label) {
		g_object_get (G_OBJECT (dialog), "ok-button", &button, NULL);
		gtk_button_set_label (GTK_BUTTON (button), data->ok_label);
		g_object_unref (G_OBJECT (button));
	}

	if (data->cancel_label) {
		g_object_get (G_OBJECT (dialog), "cancel-button", &button, NULL);
		gtk_button_set_label (GTK_BUTTON (button), data->cancel_label);
		g_object_unref (G_OBJECT (button));
	}

	if (data->modal)
		gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

	g_object_set (dialog, "show-editor", !color_data->show_palette, NULL);

	zenity_util_show_dialog (dialog, data->attach);

	if (data->timeout_delay > 0) {
		g_timeout_add_seconds (data->timeout_delay,
			(GSourceFunc) zenity_util_timeout_handle,
			dialog);
	}

	gtk_main ();
}

static void
zenity_colorselection_dialog_response (
	GtkWidget *widget, int response, gpointer data) {
	GdkRGBA color;

	switch (response) {
		case GTK_RESPONSE_OK:
			zenity_util_exit_code_with_data (ZENITY_OK, zen_data);
			gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (widget), &color);
			g_print ("%s\n", gdk_rgba_to_string (&color));
			break;

		case GTK_RESPONSE_CANCEL:
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_CANCEL);
			break;

		default:
			if (zen_data->extra_label &&
				response < g_strv_length (zen_data->extra_label))
				printf ("%s\n", zen_data->extra_label[response]);
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_ESC);
			break;
	}

	gtk_main_quit ();
}
