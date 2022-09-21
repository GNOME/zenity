/* vim: colorcolumn=80 ts=4 sw=4
 */
/*
 * scale.c
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
 * Original Author: Lucas Rocha <lucasr@gnome.org>
 */


#include "util.h"
#include "zenity.h"

#include <config.h>

static GtkWidget *scale;

static void zenity_scale_value_changed (GtkWidget *widget, gpointer data);
static void zenity_scale_dialog_response (GtkWidget *widget, int response,
		gpointer data);

void
zenity_scale (ZenityData *data, ZenityScaleData *scale_data)
{
	g_autoptr(GtkBuilder) builder = NULL;
	GtkWidget *dialog;
	GtkWidget *button;
	GObject *text;

	builder =
		zenity_util_load_ui_file ("zenity_scale_dialog", "adjustment1", NULL);

	if (builder == NULL) {
		data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
		return;
	}

	dialog =
		GTK_WIDGET (gtk_builder_get_object (builder, "zenity_scale_dialog"));
	scale =
		GTK_WIDGET (gtk_builder_get_object (builder, "zenity_scale_hscale"));
	text = gtk_builder_get_object (builder, "zenity_scale_text");

	g_signal_connect (dialog, "response",
		G_CALLBACK(zenity_scale_dialog_response), data);

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

	if (data->dialog_title)
		gtk_window_set_title (GTK_WINDOW (dialog), data->dialog_title);

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
		for (int i = 0; data->extra_label[i] != NULL; ++i)
		{
			gtk_dialog_add_button (GTK_DIALOG (dialog),
					data->extra_label[i], i);
		}
	}

	if (data->ok_label) {
		button = GTK_WIDGET(gtk_builder_get_object (builder,
					"zenity_scale_ok_button"));
		gtk_button_set_label (GTK_BUTTON (button), data->ok_label);
	}

	if (data->cancel_label)
	{
		button = GTK_WIDGET (gtk_builder_get_object (builder,
					"zenity_scale_cancel_button"));
		gtk_button_set_label (GTK_BUTTON (button), data->cancel_label);
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
		g_timeout_add_seconds (data->timeout_delay,
			(GSourceFunc) zenity_util_timeout_handle,
			dialog);
	}

	zenity_util_gapp_main (GTK_WINDOW(dialog));
}

static void
zenity_scale_value_changed (GtkWidget *widget, gpointer data)
{
	g_print ("%.0f\n", gtk_range_get_value (GTK_RANGE(widget)));
}

static void
zenity_scale_dialog_response (GtkWidget *widget, int response, gpointer data)
{
	ZenityData *zen_data = data;

	switch (response)
	{
		case GTK_RESPONSE_OK:
			zenity_util_exit_code_with_data (ZENITY_OK, zen_data);
			g_print ("%.0f\n", gtk_range_get_value (GTK_RANGE (scale)));
			break;

		case GTK_RESPONSE_CANCEL:
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_CANCEL);
			break;

		case ZENITY_TIMEOUT:
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_TIMEOUT);
			g_print ("%.0f\n", gtk_range_get_value (GTK_RANGE (scale)));
			break;

		default:
			if (zen_data->extra_label &&
				response < (int)g_strv_length (zen_data->extra_label))
			{
				printf ("%s\n", zen_data->extra_label[response]);
			}
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_ESC);
			break;
	}

	zenity_util_gapp_quit (GTK_WINDOW(widget));
}
