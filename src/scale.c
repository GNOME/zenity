/* vim: colorcolumn=80 ts=4 sw=4
 */
/*
 * scale.c
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
 * Original Author: Lucas Rocha <lucasr@gnome.org>
 */


#include "util.h"
#include "zenity.h"

#include <config.h>

static GtkWidget *scale;

static void zenity_scale_value_changed (GtkWidget *widget, gpointer data);
static void zenity_scale_dialog_response (GtkWidget *widget, char *rstr, gpointer data);

void
zenity_scale (ZenityData *data, ZenityScaleData *scale_data)
{
	g_autoptr(GtkBuilder) builder = NULL;
	GtkWidget *dialog;
	GObject *text;

	builder = zenity_util_load_ui_file ("zenity_scale_dialog", "zenity_scale_adjustment", "zenity_scale_box", NULL);

	if (builder == NULL) {
		data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
		return;
	}

	dialog =
		GTK_WIDGET (gtk_builder_get_object (builder, "zenity_scale_dialog"));
	scale =
		GTK_WIDGET (gtk_builder_get_object (builder, "zenity_scale_hscale"));
	text = gtk_builder_get_object (builder, "zenity_scale_text");

	g_signal_connect (dialog, "response", G_CALLBACK(zenity_scale_dialog_response), data);

	if (scale_data->min_value >= scale_data->max_value)
	{
		g_printerr (_("Maximum value must be greater than minimum value.\n"));
		data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
		return;
	}

	if (scale_data->value < scale_data->min_value ||
		scale_data->value > scale_data->max_value)
	{
		g_printerr (_ ("Value out of range.\n"));
		data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
		return;
	}

	zenity_util_setup_dialog_title (dialog, data);

	gtk_window_set_icon_name (GTK_WINDOW(dialog),
			"dialog-question");

	if (data->width > -1 || data->height > -1) {
		gtk_window_set_default_size (GTK_WINDOW(dialog),
				data->width, data->height);
	}

	if (data->modal)
		gtk_window_set_modal (GTK_WINDOW(dialog), TRUE);

	if (data->extra_label)
	{
		ZENITY_UTIL_ADD_EXTRA_LABELS (dialog)
	}

	if (data->ok_label)
	{
		ZENITY_UTIL_SETUP_OK_BUTTON_LABEL (dialog);
	}

	if (data->cancel_label)
	{
		ZENITY_UTIL_SETUP_CANCEL_BUTTON_LABEL (dialog);
	}

	if (scale_data->dialog_text) {
		gtk_label_set_markup (GTK_LABEL (text),
				g_strcompress (scale_data->dialog_text));
	}

	gtk_range_set_range (GTK_RANGE (scale),
			scale_data->min_value, scale_data->max_value);
	gtk_range_set_value (GTK_RANGE (scale), scale_data->value);
	gtk_range_set_increments (
		GTK_RANGE (scale), scale_data->step, scale_data->step);

	if (scale_data->print_partial)
	{
		g_signal_connect (scale, "value-changed",
			G_CALLBACK(zenity_scale_value_changed), data);
	}

	if (scale_data->hide_value)
		gtk_scale_set_draw_value (GTK_SCALE (scale), FALSE);

	zenity_util_show_dialog (dialog);

	if (data->timeout_delay > 0)
	{
		ZENITY_UTIL_SETUP_TIMEOUT (dialog)
	}

	zenity_util_gapp_main (GTK_WINDOW(dialog));
}

static void
zenity_scale_value_changed (GtkWidget *widget, gpointer data)
{
	g_print ("%.0f\n", gtk_range_get_value (GTK_RANGE(widget)));
}

static void
zenity_scale_dialog_response (GtkWidget *widget, char *rstr, gpointer data)
{
	ZenityData *zen_data = data;
	int response = zenity_util_parse_dialog_response (rstr);

	switch (response)
	{
		case ZENITY_OK:
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
			g_print ("%.0f\n", gtk_range_get_value (GTK_RANGE (scale)));
			break;

		case ZENITY_CANCEL:
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_CANCEL);
			break;

		case ZENITY_TIMEOUT:
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_TIMEOUT);
			g_print ("%.0f\n", gtk_range_get_value (GTK_RANGE (scale)));
			break;

		case ZENITY_ESC:
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_ESC);
			break;

		default:
			ZENITY_UTIL_RESPONSE_HANDLE_EXTRA_BUTTONS
			break;
	}

	zenity_util_gapp_quit (GTK_WINDOW(widget), zen_data);
}
