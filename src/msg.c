/*
 * msg.c
 *
 * Copyright (C) 2002 Sun Microsystems, Inc.
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
 */

#include "config.h"

#include "util.h"
#include "zenity.h"

static void zenity_msg_dialog_response (
	GtkWidget *widget, int response, gpointer data);
static void
zenity_msg_construct_question_dialog (
	GtkWidget *dialog, ZenityMsgData *msg_data, ZenityData *data) {

	GtkWidget *cancel_button, *ok_button;

	cancel_button = gtk_dialog_add_button (
		GTK_DIALOG (dialog), _ ("_No"), GTK_RESPONSE_CANCEL);
	ok_button = gtk_dialog_add_button (
		GTK_DIALOG (dialog), _ ("_Yes"), GTK_RESPONSE_OK);

	gtk_widget_grab_focus (
		msg_data->default_cancel ? cancel_button : ok_button);

	if (data->cancel_label) {
		gtk_button_set_label (GTK_BUTTON (cancel_button), data->cancel_label);
	}

	if (data->ok_label) {
		gtk_button_set_label (GTK_BUTTON (ok_button), data->ok_label);
	}
}

static void
zenity_label_widget_clipboard_selection (GtkWidget *widget) {
	/* Workaround hotfix for suspected toolkit issue:
	   since focus change of the dialog's focussed widget (text)
	   somehow currently chooses to destroy
	   a pre-existing (read: foreign, user-initiated) X11 primary selection
	   (via gtk_label_select_region() -> ...
	   -> gtk_clipboard_set_contents()/gtk_clipboard_clear()),
	   we need to ensure
	   that the widget does have its gtk-label-select-on-focus property off,
	   in order to avoid having the label become selected automatically
	   and thereby having pre-existing clipboard content nullified.
	   Side note: this selection issue only applies to widgets
	   which have both
					<property name="can_focus">True</property>
					<property name="selectable">True</property>
	   .
	 */
	g_object_set (gtk_widget_get_settings (widget),
		"gtk-label-select-on-focus",
		FALSE,
		NULL);
}

void
zenity_msg (ZenityData *data, ZenityMsgData *msg_data) {
	GtkBuilder *builder;
	GtkWidget *dialog;
	GtkWidget *ok_button;
	GObject *text;
	GObject *image;

	switch (msg_data->mode) {
		case ZENITY_MSG_WARNING:
			builder = zenity_util_load_ui_file ("zenity_warning_dialog", NULL);
			dialog = GTK_WIDGET (
				gtk_builder_get_object (builder, "zenity_warning_dialog"));
			text = gtk_builder_get_object (builder, "zenity_warning_text");
			image = gtk_builder_get_object (builder, "zenity_warning_image");
			ok_button = GTK_WIDGET (
				gtk_builder_get_object (builder, "zenity_warning_ok_button"));
			break;

		case ZENITY_MSG_QUESTION:
		case ZENITY_MSG_SWITCH:
			builder = zenity_util_load_ui_file ("zenity_question_dialog", NULL);
			dialog = GTK_WIDGET (
				gtk_builder_get_object (builder, "zenity_question_dialog"));
			text = gtk_builder_get_object (builder, "zenity_question_text");
			image = gtk_builder_get_object (builder, "zenity_question_image");
			ok_button = NULL;
			break;

		case ZENITY_MSG_ERROR:
			builder = zenity_util_load_ui_file ("zenity_error_dialog", NULL);
			dialog = GTK_WIDGET (
				gtk_builder_get_object (builder, "zenity_error_dialog"));
			text = gtk_builder_get_object (builder, "zenity_error_text");
			image = gtk_builder_get_object (builder, "zenity_error_image");
			ok_button = GTK_WIDGET (
				gtk_builder_get_object (builder, "zenity_error_ok_button"));
			break;

		case ZENITY_MSG_INFO:
			builder = zenity_util_load_ui_file ("zenity_info_dialog", NULL);
			dialog = GTK_WIDGET (
				gtk_builder_get_object (builder, "zenity_info_dialog"));
			text = gtk_builder_get_object (builder, "zenity_info_text");
			image = gtk_builder_get_object (builder, "zenity_info_image");
			ok_button = GTK_WIDGET (
				gtk_builder_get_object (builder, "zenity_info_ok_button"));
			break;

		default:
			builder = NULL;
			dialog = NULL;
			text = NULL;
			image = NULL;
			ok_button = NULL;
			g_assert_not_reached ();
			break;
	}

	if (data->extra_label) {
		gint i = 0;
		while (data->extra_label[i] != NULL) {
			gtk_dialog_add_button (
				GTK_DIALOG (dialog), data->extra_label[i], i);
			i++;
		}
	}

	if (builder == NULL) {
		data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
		return;
	}

	g_signal_connect (G_OBJECT (dialog),
		"response",
		G_CALLBACK (zenity_msg_dialog_response),
		data);

	gtk_builder_connect_signals (builder, NULL);

	if (data->dialog_title)
		gtk_window_set_title (GTK_WINDOW (dialog), data->dialog_title);

	if (ok_button) {
		if (data->ok_label) {
			gtk_button_set_label (GTK_BUTTON (ok_button), data->ok_label);
		}
	}

	switch (msg_data->mode) {
		case ZENITY_MSG_WARNING:
			zenity_util_set_window_icon_from_icon_name (
				dialog, data->window_icon, "dialog-warning");
			break;

		case ZENITY_MSG_QUESTION:
			zenity_util_set_window_icon_from_icon_name (
				dialog, data->window_icon, "dialog-question");
			zenity_msg_construct_question_dialog (dialog, msg_data, data);
			break;

		case ZENITY_MSG_SWITCH:
			zenity_util_set_window_icon_from_icon_name (
				dialog, data->window_icon, "dialog-question");
			break;

		case ZENITY_MSG_ERROR:
			zenity_util_set_window_icon_from_icon_name (
				dialog, data->window_icon, "dialog-error");
			break;

		case ZENITY_MSG_INFO:
			zenity_util_set_window_icon_from_icon_name (
				dialog, data->window_icon, "dialog-information");
			break;

		default:
			break;
	}

	if (data->width > -1 || data->height > -1)
		gtk_window_set_default_size (
			GTK_WINDOW (dialog), data->width, data->height);

	if (data->width > -1)
		gtk_widget_set_size_request (GTK_WIDGET (text), data->width, -1);
	else if (!msg_data->ellipsize && !msg_data->no_wrap) {
		// the magic number 60 is picked from gtk+/gtk/ui/gtkmessagedialog.ui
		// however, 60 would increase the distance between the icon and the
		// text,
		// decreasing to 10 fix it.
		gtk_label_set_width_chars (text, 10);
		gtk_label_set_max_width_chars (text, 10);
	}

	if (data->modal)
		gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

	if (msg_data->dialog_text) {
		if (msg_data->no_markup)
			gtk_label_set_text (GTK_LABEL (text), msg_data->dialog_text);
		else
			gtk_label_set_markup (
				GTK_LABEL (text), g_strcompress (msg_data->dialog_text));
		zenity_label_widget_clipboard_selection (GTK_WIDGET (text));
	}

	if (msg_data->ellipsize)
		gtk_label_set_ellipsize (GTK_LABEL (text), PANGO_ALIGN_RIGHT);

	if (msg_data->dialog_icon)
		gtk_image_set_from_icon_name (
			GTK_IMAGE (image), msg_data->dialog_icon, GTK_ICON_SIZE_DIALOG);

	if (msg_data->no_wrap)
		gtk_label_set_line_wrap (GTK_LABEL (text), FALSE);

	zenity_util_show_dialog (dialog, data->attach);

	if (data->timeout_delay > 0) {
		g_timeout_add_seconds (data->timeout_delay,
			(GSourceFunc) zenity_util_timeout_handle,
			NULL);
	}

	g_object_unref (builder);

	gtk_main ();
}

static void
zenity_msg_dialog_response (GtkWidget *widget, int response, gpointer data) {
	ZenityData *zen_data = data;

	switch (response) {
		case GTK_RESPONSE_OK:
			zenity_util_exit_code_with_data (ZENITY_OK, zen_data);
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
