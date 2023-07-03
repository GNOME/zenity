/* vim: colorcolumn=80 ts=4 sw=4
 */
/*
 * msg.c
 *
 * Copyright (C) 2002 Sun Microsystems, Inc.
 * Copyright © 2021-2023 Logan Rathbone
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

#include <config.h>

#include "util.h"
#include "zenity.h"

static void zenity_msg_dialog_response (GtkWidget *widget, char *rstr, gpointer data);

/* FIXME - Is this still necessary with gtk4? */
static void
zenity_label_widget_clipboard_selection (GtkWidget *widget)
{
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
zenity_msg (ZenityData *data, ZenityMsgData *msg_data)
{
	g_autoptr(GtkBuilder) builder;
	GtkWidget *dialog;
	GObject *text;
	GObject *image;

	switch (msg_data->mode)
	{
		case ZENITY_MSG_WARNING:
			builder = zenity_util_load_ui_file ("zenity_warning_dialog", "zenity_warning_box", NULL);
			dialog = GTK_WIDGET (
				gtk_builder_get_object (builder, "zenity_warning_dialog"));
			text = gtk_builder_get_object (builder, "zenity_warning_text");
			image = gtk_builder_get_object (builder, "zenity_warning_image");
			break;

		case ZENITY_MSG_QUESTION:
		case ZENITY_MSG_SWITCH:
			builder = zenity_util_load_ui_file ("zenity_question_dialog", "zenity_question_box", NULL);
			dialog = GTK_WIDGET (gtk_builder_get_object (builder,
						"zenity_question_dialog"));
			text = gtk_builder_get_object (builder, "zenity_question_text");
			image = gtk_builder_get_object (builder, "zenity_question_image");
			break;

		case ZENITY_MSG_ERROR:
			builder = zenity_util_load_ui_file ("zenity_error_dialog", "zenity_error_box", NULL);
			dialog = GTK_WIDGET (gtk_builder_get_object (builder,
						"zenity_error_dialog"));
			text = gtk_builder_get_object (builder, "zenity_error_text");
			image = gtk_builder_get_object (builder, "zenity_error_image");
			break;

		case ZENITY_MSG_INFO:
			builder = zenity_util_load_ui_file ("zenity_info_dialog", "zenity_info_box", NULL);
			dialog = GTK_WIDGET (gtk_builder_get_object (builder,
						"zenity_info_dialog"));
			text = gtk_builder_get_object (builder, "zenity_info_text");
			image = gtk_builder_get_object (builder, "zenity_info_image");
			break;

		default:
			builder = NULL;
			dialog = NULL;
			text = NULL;
			image = NULL;
			g_assert_not_reached ();
			break;
	}

	if (msg_data->mode == ZENITY_MSG_QUESTION)
	{
		adw_message_dialog_add_responses (ADW_MESSAGE_DIALOG(dialog),
				"no", _("_No"),
				"yes", _("_Yes"),
				NULL);
	}

	if (data->extra_label)
	{
		ZENITY_UTIL_ADD_EXTRA_LABELS (dialog) 
	}

	if (builder == NULL) {
		data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
		return;
	}

	g_signal_connect (dialog, "response", G_CALLBACK(zenity_msg_dialog_response), data);

	if (data->dialog_title)
		adw_message_dialog_set_heading (ADW_MESSAGE_DIALOG(dialog), data->dialog_title);;

	if (data->ok_label)
	{
		ZENITY_UTIL_SETUP_OK_BUTTON_LABEL (dialog)
	}

	switch (msg_data->mode)
	{
		case ZENITY_MSG_WARNING:
			gtk_window_set_icon_name (GTK_WINDOW(dialog),
					"dialog-warning");
			break;

		case ZENITY_MSG_QUESTION:
			gtk_window_set_icon_name (GTK_WINDOW(dialog),
					"dialog-question");
			break;

		case ZENITY_MSG_SWITCH:
			gtk_window_set_icon_name (GTK_WINDOW(dialog),
					"dialog-question");
			break;

		case ZENITY_MSG_ERROR:
			gtk_window_set_icon_name (GTK_WINDOW(dialog),
					"dialog-error");
			break;

		case ZENITY_MSG_INFO:
			gtk_window_set_icon_name (GTK_WINDOW(dialog),
					"dialog-information");
			break;

		default:
			break;
	}
	if (data->width > -1 || data->height > -1) {
		gtk_window_set_default_size (GTK_WINDOW(dialog),
				data->width, data->height);
	}

	if (data->width > -1) {
		gtk_widget_set_size_request (GTK_WIDGET (text), data->width, -1);
	}
	else if (!msg_data->ellipsize && !msg_data->no_wrap) {
		/* the magic number 60 is picked from gtk+/gtk/ui/gtkmessagedialog.ui
		 */
		gtk_label_set_max_width_chars (GTK_LABEL(text), 60);
	}

	if (data->modal)
		gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

	if (msg_data->dialog_text)
	{
		if (msg_data->no_markup) {
			gtk_label_set_text (GTK_LABEL (text), msg_data->dialog_text);
		}
		else {
			gtk_label_set_markup (GTK_LABEL (text),
					g_strcompress (msg_data->dialog_text));
		}
		zenity_label_widget_clipboard_selection (GTK_WIDGET (text));
	}

	if (msg_data->ellipsize)
		gtk_label_set_ellipsize (GTK_LABEL (text), PANGO_ELLIPSIZE_END);

	if (msg_data->dialog_icon)
	{
		g_autoptr(GIcon) icon = NULL;

		icon = zenity_util_gicon_from_string (msg_data->dialog_icon);
		gtk_image_set_from_gicon (GTK_IMAGE (image), icon);
	}

	if (msg_data->no_wrap)
		gtk_label_set_wrap (GTK_LABEL(text), FALSE);

	zenity_util_show_dialog (dialog);

	if (data->timeout_delay > 0) {
		g_timeout_add_seconds (data->timeout_delay,
			(GSourceFunc) zenity_util_timeout_handle,
			NULL);
	}

	zenity_util_gapp_main (GTK_WINDOW(dialog));
}

static void
zenity_msg_dialog_response (GtkWidget *widget, char *rstr, gpointer data)
{
	ZenityData *zen_data = data;
	ZenityExitCode response = zenity_util_parse_dialog_response (rstr);

	switch (response)
	{	
		case ZENITY_OK:
			zenity_util_exit_code_with_data (ZENITY_OK, zen_data);
			break;

		case ZENITY_CANCEL:
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_CANCEL);
			break;

		default:
			if (zen_data->extra_label &&
				response < (int)g_strv_length (zen_data->extra_label))
				printf ("%s\n", zen_data->extra_label[response]);
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_ESC);
			break;
	}
	zenity_util_gapp_quit (GTK_WINDOW(widget), zen_data);
}
