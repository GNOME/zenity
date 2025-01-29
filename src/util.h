/* vim: colorcolumn=80 ts=4 sw=4
 */
/*
 * util.h
 *
 * Copyright © 2002 Sun Microsystems, Inc.
 *           © 1999, 2000 Red Hat Inc.
 *           © 1998 James Henstridge
 *           © 1995-2002 Free Software Foundation
 *           © 2021-2024 Logan Rathbone
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
 * Original Authors of util.c (inferring they were the authors of
 * this file as well):
 *
 * 			Glynn Foster <glynn.foster@sun.com>
 *          Havoc Pennington <hp@redhat.com>
 *          James Henstridge <james@daa.com.au>
 *          Tom Tromey <tromey@redhat.com>
 */

#pragma once

#include "zenity.h"
#include "zenity-tree-column-view.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ZENITY_IMAGE_FULLPATH(filename) (PACKAGE_DATADIR "/" filename)

/* FIXME: GTK_IS_DIALOG test below is for backwards compatibility with older
 * dialogs (eg, color selection dialog) only. Once we have a libadwaita or
 * custom equivalent, remove that test here.
 */
#define ZENITY_UTIL_ADD_EXTRA_LABELS(DIALOG) \
	G_GNUC_BEGIN_IGNORE_DEPRECATIONS \
	for (int i = 0; data->extra_label[i] != NULL; ++i) { \
		if (ADW_IS_MESSAGE_DIALOG (DIALOG)) \
			zenity_util_add_button (ADW_MESSAGE_DIALOG (DIALOG), data->extra_label[i], i); \
		else if (GTK_IS_DIALOG (DIALOG)) \
			gtk_dialog_add_button (GTK_DIALOG (DIALOG), data->extra_label[i], i); \
	} \
	G_GNUC_END_IGNORE_DEPRECATIONS

#define ZENITY_UTIL_SETUP_OK_BUTTON_LABEL(DIALOG) \
	G_GNUC_BEGIN_IGNORE_DEPRECATIONS \
	if (adw_message_dialog_has_response (ADW_MESSAGE_DIALOG(DIALOG), "ok")) \
		adw_message_dialog_set_response_label (ADW_MESSAGE_DIALOG(DIALOG), "ok", data->ok_label); \
	else if (adw_message_dialog_has_response (ADW_MESSAGE_DIALOG(DIALOG), "yes")) \
		adw_message_dialog_set_response_label (ADW_MESSAGE_DIALOG(DIALOG), "yes", data->ok_label); \
	G_GNUC_END_IGNORE_DEPRECATIONS


#define ZENITY_UTIL_SETUP_CANCEL_BUTTON_LABEL(DIALOG) \
	G_GNUC_BEGIN_IGNORE_DEPRECATIONS \
	if (adw_message_dialog_has_response (ADW_MESSAGE_DIALOG(DIALOG), "cancel")) \
		adw_message_dialog_set_response_label (ADW_MESSAGE_DIALOG(DIALOG), "cancel", data->cancel_label); \
	else if (adw_message_dialog_has_response (ADW_MESSAGE_DIALOG(DIALOG), "no")) \
		adw_message_dialog_set_response_label (ADW_MESSAGE_DIALOG(DIALOG), "no", data->cancel_label); \
	G_GNUC_END_IGNORE_DEPRECATIONS

#define ZENITY_UTIL_RESPONSE_HANDLE_EXTRA_BUTTONS \
	if (zen_data->extra_label && \
		response < (int)g_strv_length (zen_data->extra_label)) \
	{ \
		printf ("%s\n", zen_data->extra_label[response]); \
		zen_data->exit_code = zenity_util_return_exit_code (ZENITY_EXTRA); \
	} \
	else \
		g_assert_not_reached ();

#define ZENITY_UTIL_SETUP_TIMEOUT(DIALOG) \
	g_timeout_add_seconds (data->timeout_delay, \
		(GSourceFunc) zenity_util_timeout_handle, \
		DIALOG);

GIcon *zenity_util_gicon_from_string (const char *str);
GtkBuilder *zenity_util_load_ui_file (const char *widget_root,
		...) G_GNUC_NULL_TERMINATED;
char *zenity_util_strip_newline (char *string);
gboolean zenity_util_fill_file_buffer (GtkTextBuffer *buffer,
		const char *filename);
void zenity_util_show_help (GError **error);
int zenity_util_return_exit_code (ZenityExitCode value);
void zenity_util_show_dialog (GtkWidget *widget);
gboolean zenity_util_timeout_handle (AdwMessageDialog *dialog);
char *zenity_util_pango_font_description_to_css (PangoFontDescription *desc);
void zenity_util_gapp_main (GtkWindow *window);
void zenity_util_gapp_quit (GtkWindow *window, ZenityData *data);
int zenity_util_parse_dialog_response (const char *response);
GtkWidget *zenity_util_add_button (AdwMessageDialog *dialog, const char
		*button_text, ZenityExitCode response_id);
void zenity_util_setup_dialog_title (gpointer dialog, ZenityData *data);

G_END_DECLS
