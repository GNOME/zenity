/* vim: colorcolumn=80 ts=4 sw=4
 */
/*
 * text.c
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
 * Original Author: Glynn Foster <glynn.foster@sun.com>
 */


#include "util.h"
#include "zenity.h"

#include <gio/gio.h>

#ifdef HAVE_WEBKITGTK
#include <webkit2/webkit2.h>
#endif

#include <config.h>

static ZenityTextData *zen_text_data;

static void zenity_text_dialog_response (GtkWidget *widget, int response,
		gpointer data);
static void zenity_text_toggle_button (GtkToggleButton *button, gpointer data);

// TODO - I don't think gtk4 support for webkit is fully "there" yet.
#ifdef HAVE_WEBKITGTK
static void
zenity_configure_webkit (WebKitWebView *web_view) {
	WebKitSettings *settings;
	settings = webkit_web_view_get_settings (web_view);
	g_object_set (G_OBJECT (settings), "auto-load-images", TRUE, NULL);
	/*
	  Stick to the defaults
	  "cursive-font-family"      gchar*                : Read / Write /
	  Construct
	  "default-encoding"         gchar*                : Read / Write /
	  Construct
	  "default-font-family"      gchar*                : Read / Write /
	  Construct
	  "default-font-size"        gint                  : Read / Write /
	  Construct
	  "default-monospace-font-size" gint               : Read / Write /
	  Construct
	  "editing-behavior"         WebKitEditingBehavior : Read / Write /
	  Construct
	*/
	g_object_set (G_OBJECT (settings), "enable-caret-browsing", FALSE, NULL);
	g_object_set (G_OBJECT (settings), "enable-developer-extras", FALSE, NULL);
	g_object_set (G_OBJECT (settings), "enable-fullscreen", FALSE, NULL);
	g_object_set (G_OBJECT (settings), "enable-html5-database", FALSE, NULL);
	g_object_set (
		G_OBJECT (settings), "enable-html5-local-storage", FALSE, NULL);
	g_object_set (G_OBJECT (settings), "enable-java", FALSE, NULL);
	g_object_set (G_OBJECT (settings), "enable-javascript", FALSE, NULL);
	g_object_set (G_OBJECT (settings),
		"enable-offline-web-application-cache",
		FALSE,
		NULL);
	g_object_set (G_OBJECT (settings), "enable-page-cache", FALSE, NULL);
	g_object_set (G_OBJECT (settings), "enable-plugins", FALSE, NULL);
	g_object_set (G_OBJECT (settings), "enable-private-browsing", TRUE, NULL);
	/*
	  Stick to defaults
	  "enforce-96-dpi"           gboolean              : Read / Write /
	  Construct
	  "fantasy-font-family"      gchar*                : Read / Write /
	  Construct
	*/
	/*
	  Stick to defaults
	  "minimum-font-size"        gint                  : Read / Write /
	  Construct
	  "minimum-logical-font-size" gint                 : Read / Write /
	  Construct
	  "monospace-font-family"    gchar*                : Read / Write /
	  Construct
	  "print-backgrounds"        gboolean              : Read / Write /
	  Construct
	  "resizable-text-areas"     gboolean              : Read / Write /
	  Construct
	  "sans-serif-font-family"   gchar*                : Read / Write /
	  Construct
	  "serif-font-family"        gchar*                : Read / Write /
	  Construct
	  "spell-checking-languages" gchar*                : Read / Write /
	  Construct
	*/
	g_object_set (G_OBJECT (settings), "enable-tabs-to-links", FALSE, NULL);
	g_object_set (G_OBJECT (settings),
		"user-agent",
		"Zenity with WebKit (KHTML, like Gecko) support",
		NULL);
	/*
	  Stick to defaults
	  "user-stylesheet-uri"      gchar*                : Read / Write /
	  Construct
	  "zoom-step"                gfloat                : Read / Write /
	  Construct
	*/
}

static gboolean
zenity_text_webview_decision_request (WebKitWebView *web_view,
	WebKitPolicyDecision *decision, WebKitPolicyDecisionType type) {
	if (type == WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION) {
		WebKitNavigationPolicyDecision *navigation_decision =
			WEBKIT_NAVIGATION_POLICY_DECISION (decision);
		WebKitNavigationAction *navigation_action =
			webkit_navigation_policy_decision_get_navigation_action (
				navigation_decision);
		webkit_policy_decision_ignore (decision);
		if (!zen_text_data->no_interaction &&
			webkit_navigation_action_get_navigation_type (navigation_action) ==
				WEBKIT_NAVIGATION_TYPE_LINK_CLICKED) {
			WebKitURIRequest *request =
				webkit_navigation_action_get_request (navigation_action);
			g_app_info_launch_default_for_uri (
				webkit_uri_request_get_uri (request), NULL, NULL);
		}
	}
	return TRUE;
}

static void
zenity_text_webview_load_changed (
	WebKitWebView *webkitwebview, WebKitLoadEvent event, gpointer user_data) {
	if (event == WEBKIT_LOAD_FINISHED) {
		g_signal_connect (G_OBJECT (webkitwebview),
			"decide-policy",
			G_CALLBACK (zenity_text_webview_decision_request),
			NULL);
	}
}

#endif	/* HAVE_WEBKITGTK */

static gboolean
zenity_text_handle_stdin (GIOChannel *channel, GIOCondition condition,
		gpointer data)
{
#define BUF_SIZE 1024
	static GtkTextBuffer *buffer;
	static GtkTextView *text_view;
	char buf[BUF_SIZE];

	gsize len;

	text_view = GTK_TEXT_VIEW (data);
	buffer = gtk_text_view_get_buffer (text_view);

	if ((condition & G_IO_IN) || (condition & (G_IO_IN | G_IO_HUP)))
	{
		g_autoptr(GError) error = NULL;
		int status;

		while (channel->is_readable != TRUE)
			;

		do {
			status = g_io_channel_read_chars (channel, buf, BUF_SIZE,
					&len, &error);

			while (g_main_context_pending (NULL)) {
				g_main_context_iteration (NULL, FALSE);
			}
		} while (status == G_IO_STATUS_AGAIN);

		if (status != G_IO_STATUS_NORMAL)
		{
			if (error) {
				g_warning ("%s: %s",
						__func__, error->message);
				error = NULL;
			}
			return FALSE;
		}

		if (len > 0)
		{
			GtkTextIter end;
			g_autofree char *utftext = NULL;
			gsize localelen;
			gsize utflen;

			gtk_text_buffer_get_end_iter (buffer, &end);

			if (! g_utf8_validate (buf, len, NULL))
			{
				utftext = g_convert_with_fallback (buf,
					len,
					"UTF-8",
					"ISO-8859-1",
					NULL,
					&localelen,
					&utflen,
					NULL);
				gtk_text_buffer_insert (buffer, &end, utftext, utflen);
			}
			else
			{
				gtk_text_buffer_insert (buffer, &end, buf, len);
			}

			if (zen_text_data->auto_scroll)
			{
				GtkTextMark *mark = gtk_text_buffer_get_insert (buffer);

				if (mark != NULL)
				{
					gtk_text_view_scroll_to_mark (text_view, mark,
							0.0, FALSE, 0, 0);
				}
			}
		}
	}
	return TRUE;
}

static void
zenity_text_fill_entries_from_stdin (GtkTextView *text_view)
{
	GIOChannel *channel;

	channel = g_io_channel_unix_new (0);
	g_io_channel_set_encoding (channel, "UTF-8", NULL);
	g_io_channel_set_flags (channel, G_IO_FLAG_NONBLOCK, NULL);
	g_io_add_watch (channel,
			G_IO_IN | G_IO_HUP, zenity_text_handle_stdin, text_view);
}

void
zenity_text (ZenityData *data, ZenityTextData *text_data)
{
	g_autoptr(GtkBuilder) builder = NULL;
	GtkWidget *dialog;
	GtkWidget *ok_button;
	GtkWidget *checkbox;
	GtkWidget *cancel_button;

	GObject *text_view;
	GtkTextBuffer *text_buffer;

#ifdef HAVE_WEBKITGTK
	GtkWidget *web_kit;
	GtkWidget *scrolled_window;
	GtkTextIter start_iter, end_iter;
	g_autofree char *content = NULL;
#endif

	zen_text_data = text_data;
	builder =
		zenity_util_load_ui_file ("zenity_text_dialog", "textbuffer1", NULL);

	if (builder == NULL) {
		data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
		return;
	}

	dialog = GTK_WIDGET(gtk_builder_get_object (builder,
				"zenity_text_dialog"));

	ok_button = GTK_WIDGET(gtk_builder_get_object (builder,
				"zenity_text_close_button"));
	cancel_button = GTK_WIDGET(gtk_builder_get_object (builder,
				"zenity_text_cancel_button"));
	checkbox = GTK_WIDGET(gtk_builder_get_object (builder,
				"zenity_text_checkbox"));

	g_signal_connect (dialog, "response",
		G_CALLBACK(zenity_text_dialog_response), data);

	if (data->dialog_title)
		gtk_window_set_title (GTK_WINDOW(dialog), data->dialog_title);

	gtk_window_set_icon_name (GTK_WINDOW(dialog),
			"accessories-text-editor");

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CLOSE);

	text_buffer = gtk_text_buffer_new (NULL);
	text_view = gtk_builder_get_object (builder, "zenity_text_view");
	gtk_text_view_set_buffer (GTK_TEXT_VIEW (text_view), text_buffer);
	gtk_text_view_set_editable (GTK_TEXT_VIEW (text_view), text_data->editable);

	if (text_data->no_wrap)
		gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text_view), GTK_WRAP_NONE);

	if (text_data->font)
	{
		PangoFontDescription *desc;
		GtkStyleContext *context;
		GtkCssProvider *provider;
		g_autofree char *css_str = NULL;

		desc = pango_font_description_from_string (text_data->font);
		css_str = zenity_util_pango_font_description_to_css (desc);
			
		provider = gtk_css_provider_new ();
		gtk_css_provider_load_from_data (provider, css_str, -1);

		context = gtk_widget_get_style_context (GTK_WIDGET(text_view));
		gtk_style_context_add_provider (context,
				GTK_STYLE_PROVIDER(provider),
				GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	}

	if (text_data->uri)
		zenity_util_fill_file_buffer (text_buffer, text_data->uri);
	else
		zenity_text_fill_entries_from_stdin (GTK_TEXT_VIEW (text_view));

	if (text_data->editable)
		zen_text_data->buffer = text_buffer;

	if (data->extra_label)
	{
		for (int i = 0; data->extra_label[i] != NULL; ++i)
		{
			gtk_dialog_add_button (GTK_DIALOG(dialog),
					data->extra_label[i], i);
		}
	}

	if (data->ok_label) {
		gtk_button_set_label (GTK_BUTTON (ok_button), data->ok_label);
	}

	if (data->cancel_label) {
		gtk_button_set_label (GTK_BUTTON (cancel_button), data->cancel_label);
	}

	if (text_data->checkbox) {
		gtk_widget_set_visible (GTK_WIDGET (checkbox), TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET (ok_button), FALSE);
		gtk_button_set_label (GTK_BUTTON (checkbox), text_data->checkbox);

		g_signal_connect (checkbox, "toggled",
			G_CALLBACK(zenity_text_toggle_button), ok_button);
	}

	if (data->width > -1 || data->height > -1)
	{
		gtk_window_set_default_size (GTK_WINDOW (dialog),
				data->width, data->height);
	}
	else {
		gtk_window_set_default_size (GTK_WINDOW (dialog), 300, 400);
	}

	if (data->modal)
		gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

/* TODO once gtk4 support fully lands in webkitgtk. */
#ifdef HAVE_WEBKITGTK
	if (text_data->html) {
		web_kit = webkit_web_view_new ();
		scrolled_window = GTK_WIDGET (
			gtk_builder_get_object (builder, "zenity_text_scrolled_window"));

		zenity_configure_webkit (WEBKIT_WEB_VIEW (web_kit));

		if (text_data->url) {
			if (!(g_str_has_prefix (text_data->url, "http://") ||
					g_str_has_prefix (text_data->url, "https://")))
				text_data->url = g_strdup_printf ("http://%s", text_data->url);

			webkit_web_view_load_uri (
				WEBKIT_WEB_VIEW (web_kit), text_data->url);
		} else {
			g_autoptr char *cwd = NULL;
			g_autoptr char *dirname = NULL;
			g_autoptr char *dirname_uri = NULL;
			dirname = text_data->uri ? g_path_get_dirname (text_data->uri)
									 : g_strdup ("/");
			cwd = g_get_current_dir ();
			dirname_uri = g_strconcat ("file://", cwd, "/", dirname, "/", NULL);
			gtk_text_buffer_get_start_iter (text_buffer, &start_iter);
			gtk_text_buffer_get_end_iter (text_buffer, &end_iter);
			content = gtk_text_buffer_get_text (
				text_buffer, &start_iter, &end_iter, TRUE);
			webkit_web_view_load_html (
				WEBKIT_WEB_VIEW (web_kit), content, dirname_uri);
		}

		// We don't want user to click on links and navigate to another page.
		// So, when the page finishes loading, we take handle of the requests.

		g_signal_connect (G_OBJECT (web_kit),
			"load-changed",
			G_CALLBACK (zenity_text_webview_load_changed),
			NULL);

		gtk_widget_destroy (GTK_WIDGET (text_view));
		gtk_container_add (GTK_CONTAINER (scrolled_window), web_kit);
		gtk_widget_show (GTK_WIDGET (web_kit));
	}
#endif /* HAVE_WEBKITGTK */

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
zenity_text_toggle_button (GtkToggleButton *button, gpointer data)
{
	GtkWidget *ok_button = GTK_WIDGET(data);

	gtk_widget_set_sensitive (ok_button,
			gtk_toggle_button_get_active (button));
}

static void
zenity_text_dialog_output (ZenityData *zen_data)
{
	if (zen_text_data->editable)
	{
		GtkTextIter start, end;
		g_autofree char *text = NULL;

		gtk_text_buffer_get_bounds (zen_text_data->buffer, &start, &end);
		text = gtk_text_buffer_get_text (zen_text_data->buffer,
				&start, &end, 0);
		g_print ("%s", text);
	}
}

static void
zenity_text_dialog_response (GtkWidget *widget, int response, gpointer data)
{
	ZenityData *zen_data = data;

	switch (response)
	{
		case GTK_RESPONSE_CLOSE:
			zenity_text_dialog_output (zen_data);
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
			break;

		case ZENITY_TIMEOUT:
			zenity_text_dialog_output (zen_data);
			zen_data->exit_code = zenity_util_return_exit_code (ZENITY_TIMEOUT);
			break;

		default:
			if (zen_data->extra_label &&
				response < (int)g_strv_length (zen_data->extra_label))
			{
				printf ("%s\n", zen_data->extra_label[response]);
			}
			zenity_util_exit_code_with_data (ZENITY_ESC, zen_data);
			break;
	}
	zenity_util_gapp_quit (GTK_WINDOW(widget));
}
