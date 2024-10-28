/* vim: colorcolumn=80 ts=4 sw=4
 */
/*
 * util.c
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
 * Authors: Glynn Foster <glynn.foster@sun.com>
 *          Havoc Pennington <hp@redhat.com>
 *          James Henstridge <james@daa.com.au>
 *          Tom Tromey <tromey@redhat.com>
 */

#include "util.h"
#include "zenity.h"

#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

#define ZENITY_OK_DEFAULT 0
#define ZENITY_CANCEL_DEFAULT 1
#define ZENITY_ESC_DEFAULT 1
#define ZENITY_ERROR_DEFAULT -1

/* This is an odd exit code for extra labels, but it is maintained
 * for backwards compatibility with zenity <= 3.x.
 */
#define ZENITY_EXTRA_DEFAULT 1

/* This exit code number is arbitrary, but since for the entire 3.x release
 * cycle, zenity would essentially exit(ZENITY_TIMEOUT), which happened to be
 * defined as 5 based on where it was placed in the enum sequence. So
 * hardcoding it as 5 now in case any pre-existing scripts relied upon that
 * being the exit status for timeouts.
 */
#define ZENITY_TIMEOUT_DEFAULT 5

#define ZENITY_UI_RESOURCE_PATH RESOURCE_BASE_PATH "/zenity.ui"

GIcon *
zenity_util_gicon_from_string (const char *str)
{
	GIcon *icon = NULL;
	g_autoptr(GFile) icon_file = NULL;

	if (str)
	{
		icon_file = g_file_new_for_path (str);

		if (g_file_query_exists (icon_file, NULL))
		{
			icon = g_file_icon_new (icon_file);
		}
		else
		{
			g_debug (_("Icon filename %s not found; trying theme icon."), str);
			icon = g_themed_icon_new_with_default_fallbacks (str);
		}
	}

	return icon;
}

GtkBuilder *
zenity_util_load_ui_file (const char *root_widget, ...)
{
	va_list args;
	char *arg = NULL;
	GPtrArray *ptrarray;
	GtkBuilder *builder = gtk_builder_new ();
	g_autoptr(GError) error = NULL;
	char **objects;
	gboolean result = FALSE;

	gtk_builder_set_translation_domain (builder, GETTEXT_PACKAGE);

	/* We have at least the root_widget and a NULL */
	ptrarray = g_ptr_array_sized_new (2);

	g_ptr_array_add (ptrarray, g_strdup (root_widget));

	va_start (args, root_widget);

	arg = va_arg (args, char *);

	while (arg) {
		g_ptr_array_add (ptrarray, g_strdup (arg));
		arg = va_arg (args, char *);
	}
	va_end (args);

	/* Enforce terminating NULL */
	g_ptr_array_add (ptrarray, NULL);
	objects = (char **)g_ptr_array_free (ptrarray, FALSE);

	/* Make sure this custom widget type is registered before this initial
	 * GtkBuilder method call. */
	g_type_ensure (ZENITY_TREE_TYPE_COLUMN_VIEW);

	result = gtk_builder_add_objects_from_resource (builder,
			ZENITY_UI_RESOURCE_PATH,
			(const char **)objects,
			&error);

	g_strfreev (objects);

	if (! result) {
		g_error ("Could not load ui resource %s: %s",
				ZENITY_UI_RESOURCE_PATH,
				error->message);
	}

	/* This should never happen, but if an unexpected error is logged, print
	 * it for debugging purposes. */
	if (error) {
		g_debug ("%s: Error generated: %s",
				__func__, error->message);
	}

	return builder;
}

char *
zenity_util_strip_newline (char *string) {
	gsize len;

	g_return_val_if_fail (string != NULL, NULL);

	len = strlen (string);
	while (len--) {
		if (string[len] == '\n')
			string[len] = '\0';
		else
			break;
	}

	return string;
}

gboolean
zenity_util_fill_file_buffer (GtkTextBuffer *buffer, const char *filename) {
	GtkTextIter iter, end;
	FILE *f;
	char buf[2048];
	int remaining = 0;

	if (filename == NULL)
		return FALSE;

	f = fopen (filename, "r");

	if (f == NULL) {
		g_warning ("Cannot open file '%s': %s", filename, g_strerror (errno));
		return FALSE;
	}

	gtk_text_buffer_get_iter_at_offset (buffer, &iter, 0);

	while (!feof (f)) {
		int count;
		const char *leftover;
		int to_read = 2047 - remaining;

		count = fread (buf + remaining, 1, to_read, f);
		buf[count + remaining] = '\0';

		g_utf8_validate (buf, count + remaining, &leftover);

		g_assert (g_utf8_validate (buf, leftover - buf, NULL));
		gtk_text_buffer_insert (buffer, &iter, buf, leftover - buf);

		remaining = (buf + remaining + count) - leftover;
		memmove(buf, leftover, remaining);

		if (remaining > 6 || count < to_read)
			break;
	}

	if (remaining) {
		g_warning (
			"Invalid UTF-8 data encountered reading file '%s'", filename);
		return FALSE;
	}

	/* We had a newline in the buffer to begin with. (The buffer always contains
	 * a newline, so we delete to the end of the buffer to clean up.
	 */

	gtk_text_buffer_get_end_iter (buffer, &end);
	gtk_text_buffer_delete (buffer, &iter, &end);

	gtk_text_buffer_set_modified (buffer, FALSE);

	return TRUE;
}

void
zenity_util_show_help (GError **error) {
	g_autofree char *tmp = g_find_program_in_path ("yelp");

	if (tmp)
		g_spawn_command_line_async ("yelp help:zenity", error);
}

int
zenity_util_return_exit_code (ZenityExitCode value)
{
	const char *env_var = NULL;
	int retval;

	switch (value) {

		case ZENITY_OK:
			env_var = g_getenv ("ZENITY_OK");
			if (!env_var)
				env_var = g_getenv ("DIALOG_OK");
			if (!env_var)
				retval = ZENITY_OK_DEFAULT;
			break;

		case ZENITY_CANCEL:
			env_var = g_getenv ("ZENITY_CANCEL");
			if (!env_var)
				env_var = g_getenv ("DIALOG_CANCEL");
			if (!env_var)
				retval = ZENITY_CANCEL_DEFAULT;
			break;

		case ZENITY_ESC:
			env_var = g_getenv ("ZENITY_ESC");
			if (!env_var)
				env_var = g_getenv ("DIALOG_ESC");
			if (!env_var)
				retval = ZENITY_ESC_DEFAULT;
			break;

		case ZENITY_EXTRA:
			env_var = g_getenv ("ZENITY_EXTRA");
			if (!env_var)
				env_var = g_getenv ("DIALOG_EXTRA");
			if (!env_var)
				retval = ZENITY_EXTRA_DEFAULT;
			break;

		case ZENITY_ERROR:
			env_var = g_getenv ("ZENITY_ERROR");
			if (!env_var)
				env_var = g_getenv ("DIALOG_ERROR");
			if (!env_var)
				retval = ZENITY_ERROR_DEFAULT;
			break;

		case ZENITY_TIMEOUT:
			env_var = g_getenv ("ZENITY_TIMEOUT");
			if (!env_var)
				env_var = g_getenv ("DIALOG_TIMEOUT");
			if (!env_var)
				retval = ZENITY_TIMEOUT_DEFAULT;
			break;

		default:
			retval = 1;
	}

	if (env_var)
		retval = atoi (env_var);
	return retval;
}

/* This function was written by Matthias Clasen and is included somewhere in
 * the GTK source tree.. I believe it is also included in libdazzle, but I
 * didn't want to include a whole dependency just for one function. LGPL, but
 * credit where credit is due!
 */
char *
zenity_util_pango_font_description_to_css (PangoFontDescription *desc)
{
	GString *s;
	PangoFontMask set;

	s = g_string_new ("* { ");

	set = pango_font_description_get_set_fields (desc);
	if (set & PANGO_FONT_MASK_FAMILY)
	{
		g_string_append (s, "font-family: ");
		g_string_append (s, pango_font_description_get_family (desc));
		g_string_append (s, "; ");
	}
	if (set & PANGO_FONT_MASK_STYLE)
	{
		switch (pango_font_description_get_style (desc))
		{
			case PANGO_STYLE_NORMAL:
				g_string_append (s, "font-style: normal; ");
				break;
			case PANGO_STYLE_OBLIQUE:
				g_string_append (s, "font-style: oblique; ");
				break;
			case PANGO_STYLE_ITALIC:
				g_string_append (s, "font-style: italic; ");
				break;
		}
	}
	if (set & PANGO_FONT_MASK_VARIANT)
	{
		switch (pango_font_description_get_variant (desc))
		{
			case PANGO_VARIANT_NORMAL:
				g_string_append (s, "font-variant: normal; ");
				break;
			case PANGO_VARIANT_SMALL_CAPS:
				g_string_append (s, "font-variant: small-caps; ");
				break;
			default:
				break;
		}
	}
	if (set & PANGO_FONT_MASK_WEIGHT)
	{
		switch (pango_font_description_get_weight (desc))
		{
			case PANGO_WEIGHT_THIN:
				g_string_append (s, "font-weight: 100; ");
				break;
			case PANGO_WEIGHT_ULTRALIGHT:
				g_string_append (s, "font-weight: 200; ");
				break;
			case PANGO_WEIGHT_LIGHT:
			case PANGO_WEIGHT_SEMILIGHT:
				g_string_append (s, "font-weight: 300; ");
				break;
			case PANGO_WEIGHT_BOOK:
			case PANGO_WEIGHT_NORMAL:
				g_string_append (s, "font-weight: 400; ");
				break;
			case PANGO_WEIGHT_MEDIUM:
				g_string_append (s, "font-weight: 500; ");
				break;
			case PANGO_WEIGHT_SEMIBOLD:
				g_string_append (s, "font-weight: 600; ");
				break;
			case PANGO_WEIGHT_BOLD:
				g_string_append (s, "font-weight: 700; ");
				break;
			case PANGO_WEIGHT_ULTRABOLD:
				g_string_append (s, "font-weight: 800; ");
				break;
			case PANGO_WEIGHT_HEAVY:
			case PANGO_WEIGHT_ULTRAHEAVY:
				g_string_append (s, "font-weight: 900; ");
				break;
		}
	}
	if (set & PANGO_FONT_MASK_STRETCH)
	{
		switch (pango_font_description_get_stretch (desc))
		{
			case PANGO_STRETCH_ULTRA_CONDENSED:
				g_string_append (s, "font-stretch: ultra-condensed; ");
				break;
			case PANGO_STRETCH_EXTRA_CONDENSED:
				g_string_append (s, "font-stretch: extra-condensed; ");
				break;
			case PANGO_STRETCH_CONDENSED:
				g_string_append (s, "font-stretch: condensed; ");
				break;
			case PANGO_STRETCH_SEMI_CONDENSED:
				g_string_append (s, "font-stretch: semi-condensed; ");
				break;
			case PANGO_STRETCH_NORMAL:
				g_string_append (s, "font-stretch: normal; ");
				break;
			case PANGO_STRETCH_SEMI_EXPANDED:
				g_string_append (s, "font-stretch: semi-expanded; ");
				break;
			case PANGO_STRETCH_EXPANDED:
				g_string_append (s, "font-stretch: expanded; ");
				break;
			case PANGO_STRETCH_EXTRA_EXPANDED:
				g_string_append (s, "font-stretch: extra-expanded; ");
				break;
			case PANGO_STRETCH_ULTRA_EXPANDED:
				g_string_append (s, "font-stretch: ultra-expanded; ");
				break;
		}
	}
	if (set & PANGO_FONT_MASK_SIZE)
	{
		g_string_append_printf (s, "font-size: %dpt; ",
				pango_font_description_get_size (desc) / PANGO_SCALE);
	}

	g_string_append (s, "}");

	return g_string_free (s, FALSE);
}

void
zenity_util_show_dialog (GtkWidget *dialog)
{
	gtk_window_present (GTK_WINDOW(dialog));
}

gboolean
zenity_util_timeout_handle (AdwMessageDialog *dialog)
{
	if (dialog && ADW_IS_MESSAGE_DIALOG (dialog)) {
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
		adw_message_dialog_response (dialog, "timeout");
G_GNUC_END_IGNORE_DEPRECATIONS
	}
	else {
		exit (zenity_util_return_exit_code (ZENITY_TIMEOUT));
	}
	return FALSE;
}

void
zenity_util_gapp_main (GtkWindow *window)
{
	GApplication *app = g_application_get_default ();

	if (window)
	{
		/* As this behaves quite differently if a window is provided vs. not,
		 * let's ensure any window passed is valid.
		 */
		g_assert (GTK_IS_WINDOW (window));

		gtk_application_add_window (GTK_APPLICATION(app), window);
	}
	else {
		g_application_hold (g_application_get_default ());
	}
}

void
zenity_util_gapp_quit (GtkWindow *window, ZenityData *data)
{
	static gboolean quit_requested = FALSE;

	if (quit_requested)
	{
		g_debug ("%s: Quit request already pending. Ignoring superfluous quit request.", __func__);
		return;
	}

	/* This is a bit hack-ish, but GApplication doesn't really allow for
	 * customized exit statuses within that API.
	 */
	if (data->exit_code != 0)
		exit (data->exit_code);

	if (window)
		gtk_application_remove_window (GTK_APPLICATION(g_application_get_default ()), window);
	else
		g_application_release (g_application_get_default ());

	quit_requested = TRUE;
}

int
zenity_util_parse_dialog_response (const char *response)
{
	if (g_strcmp0 (response, "ok") == 0 || g_strcmp0 (response, "yes") == 0)
	{
		return ZENITY_OK;
	}
	else if (g_strcmp0 (response, "cancel") == 0 || g_strcmp0 (response, "no") == 0)
	{
		return ZENITY_CANCEL;
	}
	else if (g_strcmp0 (response, "timeout") == 0)
	{
		return ZENITY_TIMEOUT;
	}
	else if (response[0] >= '0' && response[0] <= '9')
	{
		/* Since atoi can return 0 upon an error or a valid "0", deal with it
		 * as a special case here.
		 */
		if (g_strcmp0 (response, "0") == 0)
			return 0;
		else
		{
			int retval = atoi (response);

			if (G_UNLIKELY (! retval))
				g_error ("%s: Programmer error: invalid integer string", __func__);

			return retval;
		}
	}
	else
	{
		return ZENITY_ESC;
	}
}

GtkWidget *
zenity_util_add_button (AdwMessageDialog *dialog, const char *button_text,
		ZenityExitCode response_id)
{
	GtkWidget *w = GTK_WIDGET(dialog);
	g_autofree char *response_str = g_strdup_printf ("%d", response_id);

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	adw_message_dialog_add_response (dialog, response_str, button_text);
G_GNUC_END_IGNORE_DEPRECATIONS

	return w;
}

void
zenity_util_setup_dialog_title (gpointer dialog, ZenityData *data)
{
	if (! data->dialog_title)
		return;

	if (ADW_IS_MESSAGE_DIALOG (dialog))
	{
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
		/* This function sets the window title as well */
		adw_message_dialog_set_heading (dialog, data->dialog_title);;
G_GNUC_END_IGNORE_DEPRECATIONS
	}
	else if (GTK_IS_WINDOW (dialog))
	{
		gtk_window_set_title (dialog, data->dialog_title);
	}
}
