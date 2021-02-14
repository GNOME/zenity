/* vim: colorcolumn=80 ts=4 sw=4
 */
/*
 * util.c
 *
 * Copyright © 2002 Sun Microsystems, Inc.
 *           © 1999, 2000 Red Hat Inc.
 *           © 1998 James Henstridge
 *           © 1995-2002 Free Software Foundation
 *           © 2021 Logan Rathbone
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

#include "config.h"

#include "config.h"
#include "util.h"
#include "zenity.h"
#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ZENITY_OK_DEFAULT 0
#define ZENITY_CANCEL_DEFAULT 1
#define ZENITY_ESC_DEFAULT 1
#define ZENITY_ERROR_DEFAULT -1
#define ZENITY_EXTRA_DEFAULT 127

GtkBuilder *
zenity_util_load_ui_file (const char *root_widget, ...)
{
	va_list args;
	char *arg = NULL;
	GPtrArray *ptrarray;
	GtkBuilder *builder = gtk_builder_new ();
	GError *error = NULL;
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
		g_error_free (error);
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

const char *
zenity_util_icon_name_from_filename (const char *filename) {
	if (!filename || !filename[0])
		return "dialog-warning"; /* default */

	if (!g_ascii_strcasecmp (filename, "warning"))
		return "dialog-warning";
	if (!g_ascii_strcasecmp (filename, "info"))
		return "dialog-information";
	if (!g_ascii_strcasecmp (filename, "question"))
		return "dialog-question";
	if (!g_ascii_strcasecmp (filename, "error"))
		return "dialog-error";
	return NULL;
}

void
zenity_util_set_window_icon_from_file (
	GtkWidget *widget, const char *filename) {
	const char *icon_name;

	icon_name = zenity_util_icon_name_from_filename (filename);
	if (icon_name) {
		gtk_window_set_icon_name (GTK_WINDOW (widget), icon_name);
	} else {
		g_debug ("%s: NOT IMPLEMENTED with icon name not existing.",
				__func__);
	}
}

void
zenity_util_set_window_icon (GtkWidget *widget,
		const char *filename, const char *default_file)
{
	if (filename != NULL) {
		zenity_util_set_window_icon_from_file (widget, filename);
	} else {
		g_debug ("%s: setting icon where no filename: NOT IMPLEMENTED",
				__func__);
	}
}

void
zenity_util_set_window_icon_from_icon_name (
	GtkWidget *widget, const char *filename, const char *default_icon_name) {
	if (filename != NULL)
		zenity_util_set_window_icon_from_file (widget, filename);
	else
		gtk_window_set_icon_name (GTK_WINDOW (widget), default_icon_name);
}

void
zenity_util_show_help (GError **error) {
	char *tmp;
	tmp = g_find_program_in_path ("yelp");

	if (tmp) {
		g_free (tmp);
		g_spawn_command_line_async ("yelp help:zenity", error);
	}
}

int
zenity_util_return_exit_code (ZenityExitCode value) {

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
				retval = ZENITY_TIMEOUT;
			break;

		default:
			retval = 1;
	}

	if (env_var)
		retval = atoi (env_var);
	return retval;
}

void
zenity_util_exit_code_with_data (ZenityExitCode value, ZenityData *zen_data)
{
	zen_data->exit_code = zenity_util_return_exit_code (value);
}

#if 0
// FIXME - ???
//#ifdef GDK_WINDOWING_X11

static Window
transient_get_xterm (void) {
	const char *wid_str = g_getenv ("WINDOWID");
	if (wid_str) {
		char *wid_str_end;
		int ret;
		Window wid = strtoul (wid_str, &wid_str_end, 10);
		if (*wid_str != '\0' && *wid_str_end == '\0' && wid != 0) {
			XWindowAttributes attrs;
			gdk_error_trap_push ();
			ret = XGetWindowAttributes (
				GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()), wid, &attrs);
			gdk_flush ();
			if (gdk_error_trap_pop () != 0 || ret == 0) {
				return None;
			}
			return wid;
		}
	}
	return None;
}

static void
transient_x_free (void *ptr) {
	if (ptr)
		XFree (ptr);
}

static gboolean
transient_is_toplevel (Window wid) {
	XTextProperty prop;
	Display *dpy = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());
	if (!XGetWMName (dpy, wid, &prop))
		return FALSE;
	transient_x_free (prop.value);
	return !!prop.value;
}

/*
 * GNOME Terminal doesn't give us its toplevel window, but the WM needs a
 * toplevel XID for proper stacking.  Other terminals work fine without this
 * magic.  We can't use GDK here since "xterm" is a foreign window.
 */

static Window
transient_get_xterm_toplevel (void) {
	Window xterm = transient_get_xterm ();
	Display *dpy = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());
	while (xterm != None && !transient_is_toplevel (xterm)) {
		Window root, parent, *children;
		unsigned nchildren;
		XQueryTree (dpy, xterm, &root, &parent, &children, &nchildren);
		transient_x_free (children);
		if (parent == root)
			xterm = None;
		else
			xterm = parent;
	}
	return xterm;
}

static void
zenity_util_make_transient (GdkWindow *window, Window parent) {
	Window parent_window = parent;
	if (parent_window == 0)
		parent_window = transient_get_xterm_toplevel ();
	if (parent_window != None) {
		XSetTransientForHint (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()),
			GDK_WINDOW_XID (window),
			parent_window);
	}
}

#endif /* GDK_WINDOWING_X11 */

/* helper for common_set_gtkhex_font_from_settings.
 * 
 * This function was written by Matthias Clasen and is included somewhere in
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
	gtk_widget_realize (dialog);	// FIXME - doubt this is necessary.
	gtk_widget_show (dialog);
}

gboolean
zenity_util_timeout_handle (gpointer data)
{
	GtkDialog *dialog = GTK_DIALOG (data);
	
	if (dialog != NULL) {
		gtk_dialog_response (dialog, ZENITY_TIMEOUT);
	}
	else {
		zenity_util_gapp_quit (GTK_WINDOW(dialog));
		exit (ZENITY_TIMEOUT);
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
zenity_util_gapp_quit (GtkWindow *window)
{
	if (window)
	{
		g_assert (GTK_IS_WINDOW (window));
		gtk_window_set_application (window, NULL);
	}
	else {
		g_application_release (g_application_get_default ());
	}
}
