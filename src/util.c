/*
 * util.c
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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors: Glynn Foster <glynn.foster@sun.com>
 *          Havoc Pennington  <hp@redhat.com>
 */

#include <stdio.h>
#include <errno.h>
#include "config.h"
#include "util.h"

GladeXML*
zenity_util_load_glade_file (const gchar *widget_root) 
{
	GladeXML *xml = NULL;

	if (g_file_test (ZENITY_GLADE_FILE_RELATIVEPATH,
			 G_FILE_TEST_EXISTS)) {
		/* Try current dir, for debugging */
		xml = glade_xml_new (ZENITY_GLADE_FILE_RELATIVEPATH, widget_root, GETTEXT_PACKAGE);
	}
 
	if (xml == NULL)
      		xml = glade_xml_new (ZENITY_GLADE_FILE_FULLPATH, widget_root, GETTEXT_PACKAGE);

	if (xml == NULL) {
		g_warning ("Could not load glade file : %s", ZENITY_GLADE_FILE_FULLPATH);
		return NULL;
	}

	return xml;
}

gboolean
zenity_util_fill_file_buffer (GtkTextBuffer *buffer, const gchar *filename) 
{
	GtkTextIter iter, end;
	FILE* f;
	gchar buf[2048];
	gint remaining = 0;

	if (filename == NULL)
		return FALSE;

	f = fopen (filename, "r");

	if (f == NULL) {
		g_warning ("Cannot open file '%s': %s", filename, g_strerror (errno));
		return FALSE;
	}

	gtk_text_buffer_get_iter_at_offset (buffer, &iter, 0);

	while (!feof (f)) {
		gint count;
		const char *leftover;
		int to_read = 2047  - remaining;

		count = fread (buf + remaining, 1, to_read, f);
		buf[count + remaining] = '\0';

		g_utf8_validate (buf, count + remaining, &leftover);

		g_assert (g_utf8_validate (buf, leftover - buf, NULL));
		gtk_text_buffer_insert (buffer, &iter, buf, leftover - buf);

		remaining = (buf + remaining + count) - leftover;
		g_memmove (buf, leftover, remaining);

		if (remaining > 6 || count < to_read)
			break;
	}

	if (remaining) {
		g_warning ("Invalid UTF-8 data encountered reading file '%s'", filename);
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

static GList *
zenity_util_list_from_char_array (const char **s)
{
	GList *list = NULL;
	gint i;

	for (i = 0; s[i]; i++) {
		GdkPixbuf *pixbuf;

		pixbuf = gdk_pixbuf_new_from_file (s[i], NULL);
		if (pixbuf)
			list = g_list_prepend (list, pixbuf);
	}

	return list;
}

static void
zenity_util_free_list (GList *list)
{
	g_list_foreach (list, (GFunc) g_object_unref, NULL);
	g_list_free (list);
}

void
zenity_util_set_window_icon (GtkWidget *widget, const gchar *filename)
{
	const gchar *filenames[2] = { NULL};
	GList *list;

        g_return_if_fail (widget != NULL);
        g_return_if_fail (GTK_IS_WINDOW (widget));

	if (filename == NULL)
		return;

	filenames[0] = filename;
	list = zenity_util_list_from_char_array (filenames);
	gtk_window_set_icon_list (GTK_WINDOW (widget), list);
	zenity_util_free_list (list);
}

void 
zenity_util_set_window_icon_from_stock (GtkWidget *widget, const gchar *stock_id)
{
	GdkPixbuf *pixbuf;
	
	pixbuf = gtk_widget_render_icon (widget, stock_id, (GtkIconSize) -1, NULL);
	gtk_window_set_icon (GTK_WINDOW (widget), pixbuf);
	g_object_unref (pixbuf);
}
