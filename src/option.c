/* vim: colorcolumn=80 ts=4 sw=4
 */
/*
 * option.h
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
 *          Lucas Rocha <lucasr@im.ufba.br>
 */

#include "option.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>
#include <langinfo.h>

#include <config.h>

/* General Options */
static char *zenity_general_dialog_title;
static int zenity_general_width;
static int zenity_general_height;
static char *zenity_general_dialog_text;
static char *zenity_general_icon;
static char *zenity_general_separator;
static gboolean zenity_general_multiple;
static gboolean zenity_general_editable;
static char *zenity_general_uri;
static gboolean zenity_general_dialog_no_wrap;
static gboolean zenity_general_dialog_no_markup;
static int zenity_general_timeout_delay;
static char *zenity_general_ok_button;
static char *zenity_general_cancel_button;
static char **zenity_general_extra_buttons;
static gboolean zenity_general_modal;
static gboolean zenity_general_dialog_ellipsize;

/* Calendar Dialog Options */
static gboolean zenity_calendar_active;
static int zenity_calendar_day;
static int zenity_calendar_month;
static int zenity_calendar_year;
static char *zenity_calendar_date_format;

/* Entry Dialog Options */
static gboolean zenity_entry_active;
static char *zenity_entry_entry_text;
static gboolean zenity_entry_hide_text;

/* Error Dialog Options */
static gboolean zenity_error_active;

/* Info Dialog Options */
static gboolean zenity_info_active;

/* File Selection Dialog Options */
static gboolean zenity_file_active;
static gboolean zenity_file_directory;
static gboolean zenity_file_save;
static char **zenity_file_filter;

/* List Dialog Options */
static gboolean zenity_list_active;
static char **zenity_list_columns;
static gboolean zenity_list_checklist;
static gboolean zenity_list_radiolist;
static char *zenity_list_print_column;
static char *zenity_list_hide_column;
static gboolean zenity_list_hide_header;
static gboolean zenity_list_imagelist;

/* Notification Dialog Options */
static gboolean zenity_notification_active;
static gboolean zenity_notification_listen;

/* Progress Dialog Options */
static gboolean zenity_progress_active;
static int zenity_progress_percentage;
static gboolean zenity_progress_pulsate;
static gboolean zenity_progress_auto_close;
static gboolean zenity_progress_auto_kill;
static gboolean zenity_progress_no_cancel;
static gboolean zenity_progress_time_remaining;

/* Question Dialog Options */
static gboolean zenity_question_active;
static gboolean zenity_question_default_cancel;
static gboolean zenity_question_switch;

/* Text Dialog Options */
static gboolean zenity_text_active;
static char *zenity_text_font;
static char *zenity_text_checkbox;
static gboolean zenity_text_auto_scroll;

#ifdef HAVE_WEBKITGTK
static gboolean zenity_text_enable_html;
static gboolean zenity_text_no_interaction;
static char *zenity_text_url;
#endif

/* Warning Dialog Options */
static gboolean zenity_warning_active;

/* Scale Dialog Options */
static gboolean zenity_scale_active;
static int zenity_scale_value;
static int zenity_scale_min_value;
static int zenity_scale_max_value;
static int zenity_scale_step;
static gboolean zenity_scale_print_partial;
static gboolean zenity_scale_hide_value;

/* Color Selection Dialog Options */
static gboolean zenity_colorsel_active;
static char *zenity_colorsel_color;
static gboolean zenity_colorsel_show_palette;

/* Password Dialog Options */
static gboolean zenity_password_active;
static gboolean zenity_password_show_username;

/* Forms Dialog Options */
static gboolean zenity_forms_active;
static gboolean zenity_forms_show_header;
static char *zenity_forms_date_format;
static char **zenity_forms_list_values;
static char **zenity_forms_column_values;
static char **zenity_forms_combo_values;

/* Miscelaneus Options */
static gboolean zenity_misc_about;
static gboolean zenity_misc_version;

/* DEPRECATED Options */

static char *zenity_general_window_icon_DEPRECATED;
static char *zenity_general_icon_name_DEPRECATED;
static gboolean zenity_list_mid_search_DEPRECATED;
static gboolean zenity_file_confirm_overwrite_DEPRECATED;
static guintptr zenity_general_attach_DEPRECATED;
static gchar **zenity_notification_hints_DEPRECATED;

static gboolean zenity_forms_callback (const char *option_name,
	const char *value, gpointer data, GError **error);

static GOptionEntry general_options[] =
		{{"title",
			 '\0',
			 0,
			 G_OPTION_ARG_STRING,
			 &zenity_general_dialog_title,
			 N_ ("Set the dialog title"),
			 N_ ("TITLE")},
		{"width",
			'\0',
			0,
			G_OPTION_ARG_INT,
			&zenity_general_width,
			N_ ("Set the width"),
			N_ ("WIDTH")},
		{"height",
			'\0',
			0,
			G_OPTION_ARG_INT,
			&zenity_general_height,
			N_ ("Set the height"),
			N_ ("HEIGHT")},
		{"timeout",
			'\0',
			0,
			G_OPTION_ARG_INT,
			&zenity_general_timeout_delay,
			N_ ("Set dialog timeout in seconds"),
			/* Timeout for closing the dialog */
			N_ ("TIMEOUT")},
		{"ok-label",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_STRING,
			&zenity_general_ok_button,
			N_ ("Set the label of the OK button"),
			N_ ("TEXT")},
		{"cancel-label",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_STRING,
			&zenity_general_cancel_button,
			N_ ("Set the label of the Cancel button"),
			N_ ("TEXT")},
		{"extra-button",
			'\0',
			0,
			G_OPTION_ARG_STRING_ARRAY,
			&zenity_general_extra_buttons,
			N_ ("Add an extra button"),
			N_ ("TEXT")},
		{"modal",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_NONE,
			&zenity_general_modal,
			N_ ("Set the modal hint"),
			NULL},
		{"attach",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_INT,
			&zenity_general_attach_DEPRECATED,
			N_ ("DEPRECATED; does nothing"),
			N_ ("WINDOW")},
		{"icon-name",
			'\0',
			G_OPTION_FLAG_HIDDEN,
			G_OPTION_ARG_STRING,
			&zenity_general_icon_name_DEPRECATED,
			N_ ("DEPRECATED; use `--icon`"),
			N_ ("ICON-NAME")},
		{"window-icon",
			'\0',
			G_OPTION_FLAG_HIDDEN,
			G_OPTION_ARG_STRING,
			&zenity_general_window_icon_DEPRECATED,
			N_ ("DEPRECATED; use `--icon`"),
			N_ ("ICON-NAME")},
		{NULL}};

static GOptionEntry calendar_options[] =
		{{"calendar",
			 '\0',
			 G_OPTION_FLAG_IN_MAIN,
			 G_OPTION_ARG_NONE,
			 &zenity_calendar_active,
			 N_ ("Display calendar dialog"),
			 NULL},
		{"text",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_STRING,
			&zenity_general_dialog_text,
			N_ ("Set the dialog text"),
			N_ ("TEXT")},
		{"day",
			'\0',
			0,
			G_OPTION_ARG_INT,
			&zenity_calendar_day,
			N_ ("Set the calendar day"),
			N_ ("DAY")},
		{"month",
			'\0',
			0,
			G_OPTION_ARG_INT,
			&zenity_calendar_month,
			N_ ("Set the calendar month"),
			N_ ("MONTH")},
		{"year",
			'\0',
			0,
			G_OPTION_ARG_INT,
			&zenity_calendar_year,
			N_ ("Set the calendar year"),
			N_ ("YEAR")},
		{"date-format",
			'\0',
			0,
			G_OPTION_ARG_STRING,
			&zenity_calendar_date_format,
			N_ ("Set the format for the returned date"),
			N_ ("PATTERN")},
		{NULL}};

static GOptionEntry entry_options[] =
		{{"entry",
			 '\0',
			 G_OPTION_FLAG_IN_MAIN,
			 G_OPTION_ARG_NONE,
			 &zenity_entry_active,
			 N_ ("Display text entry dialog"),
			 NULL},
		{"text",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_STRING,
			&zenity_general_dialog_text,
			N_ ("Set the dialog text"),
			N_ ("TEXT")},
		{"entry-text",
			'\0',
			0,
			G_OPTION_ARG_STRING,
			&zenity_entry_entry_text,
			N_ ("Set the entry text"),
			N_ ("TEXT")},
		{"hide-text",
			'\0',
			0,
			G_OPTION_ARG_NONE,
			&zenity_entry_hide_text,
			N_ ("Hide the entry text"),
			NULL},
		{NULL}};

static GOptionEntry error_options[] =
		{{"error",
			 '\0',
			 G_OPTION_FLAG_IN_MAIN,
			 G_OPTION_ARG_NONE,
			 &zenity_error_active,
			 N_ ("Display error dialog"),
			 NULL},
		{"text",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_STRING,
			&zenity_general_dialog_text,
			N_ ("Set the dialog text"),
			N_ ("TEXT")},
		{"icon",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_STRING,
			&zenity_general_icon,
			N_ ("Set the icon name"),
			N_ ("ICON-NAME")},
		{"no-wrap",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_NONE,
			&zenity_general_dialog_no_wrap,
			N_ ("Do not enable text wrapping"),
			NULL},
		{"no-markup",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_NONE,
			&zenity_general_dialog_no_markup,
			N_ ("Do not enable Pango markup"),
			NULL},
		{"ellipsize",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_NONE,
			&zenity_general_dialog_ellipsize,
			N_ ("Enable ellipsizing in the dialog text. "
					"This fixes the high window "
					"size with long texts"),
			NULL},
			{NULL}};

static GOptionEntry info_options[] =
		{{"info",
			 '\0',
			 G_OPTION_FLAG_IN_MAIN,
			 G_OPTION_ARG_NONE,
			 &zenity_info_active,
			 N_ ("Display info dialog"),
			 NULL},
		{"text",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_STRING,
			&zenity_general_dialog_text,
			N_ ("Set the dialog text"),
			N_ ("TEXT")},
		{"icon",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_STRING,
			&zenity_general_icon,
			N_ ("Set the icon name"),
			N_ ("ICON-NAME")},
		{"no-wrap",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_NONE,
			&zenity_general_dialog_no_wrap,
			N_ ("Do not enable text wrapping"),
			NULL},
		{"no-markup",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_NONE,
			&zenity_general_dialog_no_markup,
			N_ ("Do not enable Pango markup"),
			NULL},
		{"ellipsize",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_NONE,
			&zenity_general_dialog_ellipsize,
			N_ ("Enable ellipsizing in the dialog text. "
					"This fixes the high window "
					"size with long texts"),
			NULL},
			{NULL}};

static GOptionEntry file_selection_options[] =
	{{"file-selection",
		 '\0',
		 G_OPTION_FLAG_IN_MAIN,
		 G_OPTION_ARG_NONE,
		 &zenity_file_active,
		 N_ ("Display file selection dialog"),
		 NULL},
		{"filename",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_FILENAME,
			&zenity_general_uri,
			N_ ("Set the filename"),
			N_ ("FILENAME")},
		{"multiple",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_NONE,
			&zenity_general_multiple,
			N_ ("Allow multiple files to be selected"),
			NULL},
		{"directory",
			'\0',
			0,
			G_OPTION_ARG_NONE,
			&zenity_file_directory,
			N_ ("Activate directory-only selection"),
			NULL},
		{"save",
			'\0',
			0,
			G_OPTION_ARG_NONE,
			&zenity_file_save,
			N_ ("Activate save mode"),
			NULL},
		{"separator",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_STRING,
			&zenity_general_separator,
			N_ ("Set output separator character"),
			N_ ("SEPARATOR")},
		{"file-filter",
			'\0',
			0,
			G_OPTION_ARG_STRING_ARRAY,
			&zenity_file_filter,
			N_ ("Set a filename filter"),
			/* Help for file-filter argument (name and patterns for file
			   selection) */
			N_ ("NAME | PATTERN1 PATTERN2 ..."),
		},
		{"confirm-overwrite",
			'\0',
			0,
			G_OPTION_ARG_NONE,
			&zenity_file_confirm_overwrite_DEPRECATED,
			N_ ("DEPRECATED; does nothing"),
			NULL},
		{NULL}};

static GOptionEntry list_options[] =
		{{"list",
			 '\0',
			 G_OPTION_FLAG_IN_MAIN,
			 G_OPTION_ARG_NONE,
			 &zenity_list_active,
			 N_ ("Display list dialog"),
			 NULL},
		{"text",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_STRING,
			&zenity_general_dialog_text,
			N_ ("Set the dialog text"),
			N_ ("TEXT")},
		{"column",
			'\0',
			0,
			G_OPTION_ARG_STRING_ARRAY,
			&zenity_list_columns,
			N_ ("Set the column header"),
			N_ ("COLUMN")},
		{"checklist",
			'\0',
			0,
			G_OPTION_ARG_NONE,
			&zenity_list_checklist,
			N_ ("Use check boxes for the first column"),
			NULL},
		{"radiolist",
			'\0',
			0,
			G_OPTION_ARG_NONE,
			&zenity_list_radiolist,
			N_ ("Use radio buttons for the first column"),
			NULL},
		{"imagelist",
			'\0',
			0,
			G_OPTION_ARG_NONE,
			&zenity_list_imagelist,
			N_ ("Use an image for the first column"),
			NULL},
		{"separator",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_STRING,
			&zenity_general_separator,
			N_ ("Set output separator character"),
			N_ ("SEPARATOR")},
		{"multiple",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_NONE,
			&zenity_general_multiple,
			N_ ("Allow multiple rows to be selected"),
			NULL},
		{"editable",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_NONE,
			&zenity_general_editable,
			N_ ("Allow changes to text"),
			NULL},
		{"print-column",
			'\0',
			0,
			G_OPTION_ARG_STRING,
			&zenity_list_print_column,
			N_ ("Print a specific column (Default is 1. "
					"'ALL' can be used to print all columns)"),
			/* Column index number to print out on a list dialog */
			N_ ("NUMBER")},
		{"hide-column",
			'\0',
			0,
			G_OPTION_ARG_STRING,
			&zenity_list_hide_column,
			N_ ("Hide a specific column"),
			N_ ("NUMBER")},
		{"hide-header",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_NONE,
			&zenity_list_hide_header,
			N_ ("Hide the column headers"),
			NULL},
		{"mid-search",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_NONE,
			&zenity_list_mid_search_DEPRECATED,
			N_ ("DEPRECATED; does nothing"),
			NULL},
		{NULL}};

static GOptionEntry notification_options[] =
		{{"notification",
			 '\0',
			 G_OPTION_FLAG_IN_MAIN,
			 G_OPTION_ARG_NONE,
			 &zenity_notification_active,
			 N_ ("Display notification"),
			 NULL},
		{"text",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_STRING,
			&zenity_general_dialog_text,
			N_ ("Set the notification text"),
			N_ ("TEXT")},
		{"icon",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_STRING,
			&zenity_general_icon,
			N_ ("Set the icon name"),
			N_ ("ICON-NAME")},
		{"listen",
			'\0',
			0,
			G_OPTION_ARG_NONE,
			&zenity_notification_listen,
			N_ ("Listen for commands on stdin"),
			NULL},
		{"hint",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_STRING_ARRAY,
			&zenity_notification_hints_DEPRECATED,
			N_ ("DEPRECATED; does nothing"),
			N_ ("TEXT")},
		{NULL}};

static GOptionEntry progress_options[] =
		{{"progress",
			'\0',
			G_OPTION_FLAG_IN_MAIN,
			G_OPTION_ARG_NONE,
			&zenity_progress_active,
			N_ ("Display progress indication dialog"),
			NULL},
		{"text",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_STRING,
			&zenity_general_dialog_text,
			N_ ("Set the dialog text"),
			N_ ("TEXT")},
		{"percentage",
			'\0',
			0,
			G_OPTION_ARG_INT,
			&zenity_progress_percentage,
			N_ ("Set initial percentage"),
			N_ ("PERCENTAGE")},
		{"pulsate",
			'\0',
			0,
			G_OPTION_ARG_NONE,
			&zenity_progress_pulsate,
			N_ ("Pulsate progress bar"),
			NULL},
		{"auto-close",
			'\0',
			0,
			G_OPTION_ARG_NONE,
			&zenity_progress_auto_close,
			/* xgettext: no-c-format */
			N_ ("Dismiss the dialog when 100% has been reached"),
			NULL},
		{"auto-kill",
			'\0',
			0,
			G_OPTION_ARG_NONE,
			&zenity_progress_auto_kill,
			N_ ("Kill parent process if Cancel button is pressed"),
			NULL},
		{"no-cancel",
			'\0',
			0,
			G_OPTION_ARG_NONE,
			&zenity_progress_no_cancel,
			N_ ("Hide Cancel button"),
			NULL},
		{"time-remaining",
			'\0',
			0,
			G_OPTION_ARG_NONE,
			&zenity_progress_time_remaining,
			/* xgettext: no-c-format */
			N_ ("Estimate when progress will reach 100%"),
			NULL},
		{NULL}};

static GOptionEntry question_options[] =
		{{"question",
			 '\0',
			 G_OPTION_FLAG_IN_MAIN,
			 G_OPTION_ARG_NONE,
			 &zenity_question_active,
			 N_ ("Display question dialog"),
			 NULL},
		{"text",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_STRING,
			&zenity_general_dialog_text,
			N_ ("Set the dialog text"),
			N_ ("TEXT")},
		{"icon",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_STRING,
			&zenity_general_icon,
			N_ ("Set the icon name"),
			N_ ("ICON-NAME")},
		{"no-wrap",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_NONE,
			&zenity_general_dialog_no_wrap,
			N_ ("Do not enable text wrapping"),
			NULL},
		{"no-markup",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_NONE,
			&zenity_general_dialog_no_markup,
			N_ ("Do not enable Pango markup"),
			NULL},
		{"default-cancel",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_NONE,
			&zenity_question_default_cancel,
			N_ ("Give Cancel button focus by default"),
			NULL},
		{"ellipsize",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_NONE,
			&zenity_general_dialog_ellipsize,
			N_ ("Enable ellipsizing in the dialog text. "
					"This fixes the high window "
					"size with long texts"),
			NULL},
			{"switch",
				'\0',
				G_OPTION_FLAG_NOALIAS,
				G_OPTION_ARG_NONE,
				&zenity_question_switch,
				N_ ("Suppress OK and Cancel buttons"),
				NULL},
			{NULL}};

static GOptionEntry text_options[] =
	{{"text-info",
		'\0',
		G_OPTION_FLAG_IN_MAIN,
		G_OPTION_ARG_NONE,
		&zenity_text_active,
		N_ ("Display text information dialog"),
		NULL},
	{"filename",
		'\0',
		G_OPTION_FLAG_NOALIAS,
		G_OPTION_ARG_FILENAME,
		&zenity_general_uri,
		N_ ("Open file"),
		N_ ("FILENAME")},
	{"editable",
		'\0',
		G_OPTION_FLAG_NOALIAS,
		G_OPTION_ARG_NONE,
		&zenity_general_editable,
		N_ ("Allow changes to text"),
		NULL},
	{"font",
		'\0',
		0,
		G_OPTION_ARG_STRING,
		&zenity_text_font,
		N_ ("Set the text font"),
		N_ ("TEXT")},
	{"checkbox",
		'\0',
		G_OPTION_FLAG_NOALIAS,
		G_OPTION_ARG_STRING,
		&zenity_text_checkbox,
		N_ ("Enable an I read and agree checkbox"),
		N_ ("TEXT")},
#ifdef HAVE_WEBKITGTK
	{"html",
		'\0',
		G_OPTION_FLAG_NOALIAS,
		G_OPTION_ARG_NONE,
		&zenity_text_enable_html,
		N_ ("Enable HTML support"),
		NULL},
	{"no-interaction",
		'\0',
		G_OPTION_FLAG_NOALIAS,
		G_OPTION_ARG_NONE,
		&zenity_text_no_interaction,
		N_ ("Do not enable user interaction with the WebView. Only works if "
			"you use --html option"),
		NULL},
	{"url",
		'\0',
		G_OPTION_FLAG_NOALIAS,
		G_OPTION_ARG_STRING,
		&zenity_text_url,
		N_ ("Set an URL instead of a file. Only works if you use --html "
			"option"),
		N_ ("URL")},
#endif
	{"auto-scroll",
		'\0',
		G_OPTION_FLAG_NOALIAS,
		G_OPTION_ARG_NONE,
		&zenity_text_auto_scroll,
		N_ ("Auto scroll the text to the end. Only when text is captured from "
			"stdin"),
		NULL},
	{NULL}};

static GOptionEntry warning_options[] =
		{{"warning",
			 '\0',
			 G_OPTION_FLAG_IN_MAIN,
			 G_OPTION_ARG_NONE,
			 &zenity_warning_active,
			 N_ ("Display warning dialog"),
			 NULL},
		{"text",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_STRING,
			&zenity_general_dialog_text,
			N_ ("Set the dialog text"),
			N_ ("TEXT")},
		{"icon",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_STRING,
			&zenity_general_icon,
			N_ ("Set the icon name"),
			N_ ("ICON-NAME")},
		{"no-wrap",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_NONE,
			&zenity_general_dialog_no_wrap,
			N_ ("Do not enable text wrapping"),
			NULL},
		{"no-markup",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_NONE,
			&zenity_general_dialog_no_markup,
			N_ ("Do not enable Pango markup"),
			NULL},
		{"ellipsize",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_NONE,
			&zenity_general_dialog_ellipsize,
			N_ ("Enable ellipsizing in the dialog text. "
					"This fixes the high window "
					"size with long texts"),
			NULL},
			{NULL}};

static GOptionEntry scale_options[] =
		{{"scale",
			 '\0',
			 G_OPTION_FLAG_IN_MAIN,
			 G_OPTION_ARG_NONE,
			 &zenity_scale_active,
			 N_ ("Display scale dialog"),
			 NULL},
		{"text",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_STRING,
			&zenity_general_dialog_text,
			N_ ("Set the dialog text"),
			N_ ("TEXT")},
		{"value",
			'\0',
			0,
			G_OPTION_ARG_INT,
			&zenity_scale_value,
			N_ ("Set initial value"),
			N_ ("VALUE")},
		{"min-value",
			'\0',
			0,
			G_OPTION_ARG_INT,
			&zenity_scale_min_value,
			N_ ("Set minimum value"),
			N_ ("VALUE")},
		{"max-value",
			'\0',
			0,
			G_OPTION_ARG_INT,
			&zenity_scale_max_value,
			N_ ("Set maximum value"),
			N_ ("VALUE")},
		{"step",
			'\0',
			0,
			G_OPTION_ARG_INT,
			&zenity_scale_step,
			N_ ("Set step size"),
			N_ ("VALUE")},
		{"print-partial",
			'\0',
			0,
			G_OPTION_ARG_NONE,
			&zenity_scale_print_partial,
			N_ ("Print partial values"),
			NULL},
		{"hide-value",
			'\0',
			0,
			G_OPTION_ARG_NONE,
			&zenity_scale_hide_value,
			N_ ("Hide value"),
			NULL},
		{NULL}};

static GOptionEntry forms_dialog_options[] =
		{{"forms",
			 '\0',
			 G_OPTION_FLAG_IN_MAIN,
			 G_OPTION_ARG_NONE,
			 &zenity_forms_active,
			 N_ ("Display forms dialog"),
			 NULL},
		{"add-entry",
			'\0',
			0,
			G_OPTION_ARG_CALLBACK,
			zenity_forms_callback,
			N_ ("Add a new Entry in forms dialog"),
			N_ ("Field name")},
		{"add-password",
			'\0',
			0,
			G_OPTION_ARG_CALLBACK,
			zenity_forms_callback,
			N_ ("Add a new Password Entry in forms dialog"),
			N_ ("Field name")},
		{"add-multiline-entry",
			'\0',
			0,
			G_OPTION_ARG_CALLBACK,
			zenity_forms_callback,
			N_ ("Add a new multi-line entry in forms dialog (Since: 4.2)"),
			N_ ("Field name")},
		{"add-calendar",
			'\0',
			0,
			G_OPTION_ARG_CALLBACK,
			zenity_forms_callback,
			N_ ("Add a new Calendar in forms dialog"),
			N_ ("Calendar field name")},
		{"add-list",
			'\0',
			0,
			G_OPTION_ARG_CALLBACK,
			zenity_forms_callback,
			N_ ("Add a new List in forms dialog"),
			N_ ("List field and header name")},
		{"list-values",
			'\0',
			0,
			G_OPTION_ARG_STRING_ARRAY,
			&zenity_forms_list_values,
			N_ ("List of values for List"),
			N_ ("List of values separated by |")},
		{"column-values",
			'\0',
			0,
			G_OPTION_ARG_STRING_ARRAY,
			&zenity_forms_column_values,
			N_ ("List of values for columns"),
			N_ ("List of values separated by |")},
		{"add-combo",
			'\0',
			0,
			G_OPTION_ARG_CALLBACK,
			zenity_forms_callback,
			N_ ("Add a new combo box in forms dialog"),
			N_ ("Combo box field name")},
		{"combo-values",
			'\0',
			0,
			G_OPTION_ARG_STRING_ARRAY,
			&zenity_forms_combo_values,
			N_ ("List of values for combo box"),
			N_ ("List of values separated by |")},
		/* TODO: Implement how to hide specifc column
		   {
		   "hide-column",
		   '\0',
		   0,
		   G_OPTION_ARG_STRING,
		   &zenity_forms_hide_column,
		   N_("Hide a specific column"),
		   N_("NUMBER")
		   },*/
		{"show-header",
			'\0',
			0,
			G_OPTION_ARG_NONE,
			&zenity_forms_show_header,
			N_ ("Show the columns header"),
			NULL},
		{"text",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_STRING,
			&zenity_general_dialog_text,
			N_ ("Set the dialog text"),
			N_ ("TEXT")},
		{"separator",
			'\0',
			G_OPTION_FLAG_NOALIAS,
			G_OPTION_ARG_STRING,
			&zenity_general_separator,
			N_ ("Set output separator character"),
			N_ ("SEPARATOR")},
		{"date-format",
			'\0',
			0,
			G_OPTION_ARG_STRING,
			&zenity_forms_date_format,
			N_ ("Set the format for the returned date"),
			N_ ("PATTERN")},
		{NULL}};

static GOptionEntry password_dialog_options[] =
	{{"password",
		'\0',
		G_OPTION_FLAG_IN_MAIN,
		G_OPTION_ARG_NONE,
		&zenity_password_active,
		N_ ("Display password dialog"),
		NULL},
	{"username",
		'\0',
		0,
		G_OPTION_ARG_NONE,
		&zenity_password_show_username,
		N_ ("Display the username option"),
		NULL},
	{NULL}};

static GOptionEntry color_selection_options[] =
	{{"color-selection",
		'\0',
		G_OPTION_FLAG_IN_MAIN,
		G_OPTION_ARG_NONE,
		&zenity_colorsel_active,
		N_ ("Display color selection dialog"),
		NULL},
	{"color",
		'\0',
		0,
		G_OPTION_ARG_STRING,
		&zenity_colorsel_color,
		N_ ("Set the color"),
		N_ ("VALUE")},
	{"show-palette",
		'\0',
		0,
		G_OPTION_ARG_NONE,
		&zenity_colorsel_show_palette,
		N_ ("Show the palette"),
		NULL},
	{NULL}};

static GOptionEntry miscellaneous_options[] =
		{{"about",
			 '\0',
			 0,
			 G_OPTION_ARG_NONE,
			 &zenity_misc_about,
			 N_ ("About zenity"),
			 NULL},
		{"version",
			'\0',
			0,
			G_OPTION_ARG_NONE,
			&zenity_misc_version,
			N_ ("Print version"),
			NULL},
		{NULL}};

static ZenityParsingOptions *results;

/* Deprecation warnings */

static void
show_icon_name_deprecation_warning (void)
{
	g_printerr (_("Warning: --icon-name is deprecated and will be removed in a "
			"future version of zenity; Treating as --icon.\n"));
}

static void
show_window_icon_deprecation_warning (void)
{
	g_printerr (_("Warning: --window-icon is deprecated and will be removed in a "
			"future version of zenity; Treating as --icon.\n"));
}

static void
show_confirm_overwrite_deprecation_warning (void)
{
	g_printerr (_("Warning: --confirm-overwrite is deprecated and will be removed in a "
			"future version of zenity. Ignoring.\n"));
}

static void
show_attach_deprecation_warning (void)
{
	g_printerr (_("Warning: --attach is deprecated and will be removed in a "
			"future version of zenity. Ignoring.\n"));
}

static void
show_notification_hints_deprecation_warning (void)
{
	g_printerr (_("Warning: --hint is deprecated and will be removed in a "
			"future version of zenity. Ignoring.\n"));
}

static void
zenity_option_init (void)
{
	results = g_new0 (ZenityParsingOptions, 1);

	/* Initialize the various dialog structures */
	results->mode = MODE_LAST;
	results->data = g_new0 (ZenityData, 1);
	results->calendar_data = g_new0 (ZenityCalendarData, 1);
	results->msg_data = g_new0 (ZenityMsgData, 1);
	results->scale_data = g_new0 (ZenityScaleData, 1);
	results->file_data = g_new0 (ZenityFileData, 1);
	results->entry_data = g_new0 (ZenityEntryData, 1);
	results->progress_data = g_new0 (ZenityProgressData, 1);
	results->text_data = g_new0 (ZenityTextData, 1);
	results->tree_data = g_new0 (ZenityTreeData, 1);
	results->notification_data = g_new0 (ZenityNotificationData, 1);
	results->color_data = g_new0 (ZenityColorData, 1);
	results->password_data = g_new0 (ZenityPasswordData, 1);
	results->forms_data = g_new0 (ZenityFormsData, 1);
}

static void
zenity_option_set_dialog_mode (gboolean is_active, ZenityDialogMode mode)
{
	if (is_active == TRUE)
	{
		if (results->mode == MODE_LAST)
			results->mode = mode;
		else
			zenity_option_error (NULL, ERROR_DIALOG);
	}
}

static char *
zenity_option_get_name (GOptionEntry *entries, gpointer arg_data)
{
	for (int i = 1; entries[i].long_name != NULL; i++)
	{
		if (entries[i].arg_data == arg_data)
			return (char *)entries[i].long_name;
	}
	return NULL;
}

/* Forms callback */
static gboolean
zenity_forms_callback (const char *option_name, const char *value,
	gpointer data, GError **error)
{
	ZenityFormsValue *forms_value = g_new0 (ZenityFormsValue, 1);

	forms_value->option_value = g_strdup (value);

	if (g_strcmp0 (option_name, "--add-entry") == 0)
		forms_value->type = ZENITY_FORMS_ENTRY;
	else if (g_strcmp0 (option_name, "--add-calendar") == 0)
		forms_value->type = ZENITY_FORMS_CALENDAR;
	else if (g_strcmp0 (option_name, "--add-password") == 0)
		forms_value->type = ZENITY_FORMS_PASSWORD;
	else if (g_strcmp0 (option_name, "--add-list") == 0)
		forms_value->type = ZENITY_FORMS_LIST;
	else if (g_strcmp0 (option_name, "--add-combo") == 0)
		forms_value->type = ZENITY_FORMS_COMBO;
	else if (g_strcmp0 (option_name, "--add-multiline-entry") == 0)
		forms_value->type = ZENITY_FORMS_MULTLINE_ENTRY;

	results->forms_data->list =
		g_slist_append (results->forms_data->list, forms_value);

	return TRUE;
}

/* Error callback */
static void
zenity_option_error_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_option_error (NULL, ERROR_SYNTAX);
}

/* Pre parse callbacks set the default option values */

static gboolean
zenity_general_pre_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_general_dialog_title = NULL;
	zenity_general_width = -1;
	zenity_general_height = -1;
	zenity_general_dialog_text = NULL;
	zenity_general_separator = g_strdup ("|");
	zenity_general_multiple = FALSE;
	zenity_general_editable = FALSE;
	zenity_general_uri = NULL;
	zenity_general_ok_button = NULL;
	zenity_general_cancel_button = NULL;
	zenity_general_extra_buttons = NULL;
	zenity_general_dialog_no_wrap = FALSE;
	zenity_general_dialog_no_markup = FALSE;
	zenity_general_timeout_delay = -1;
	zenity_general_modal = FALSE;

	return TRUE;
}

static gboolean
zenity_calendar_pre_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_calendar_active = FALSE;
	zenity_calendar_date_format = NULL;
	zenity_calendar_day = -1;
	zenity_calendar_month = -1;
	zenity_calendar_year = -1;

	return TRUE;
}

static gboolean
zenity_entry_pre_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_entry_active = FALSE;
	zenity_entry_entry_text = NULL;
	zenity_entry_hide_text = FALSE;

	return TRUE;
}

static gboolean
zenity_error_pre_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_error_active = FALSE;

	return TRUE;
}

static gboolean
zenity_info_pre_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_info_active = FALSE;

	return TRUE;
}

static gboolean
zenity_file_pre_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_file_active = FALSE;
	zenity_file_directory = FALSE;
	zenity_file_save = FALSE;
	zenity_file_filter = NULL;
	zenity_file_confirm_overwrite_DEPRECATED = FALSE;

	return TRUE;
}

static gboolean
zenity_list_pre_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_list_active = FALSE;
	zenity_list_columns = NULL;
	zenity_list_checklist = FALSE;
	zenity_list_radiolist = FALSE;
	zenity_list_imagelist = FALSE;
	zenity_list_hide_header = FALSE;
	zenity_list_print_column = NULL;
	zenity_list_hide_column = NULL;
	zenity_list_mid_search_DEPRECATED = FALSE;

	return TRUE;
}

static gboolean
zenity_notification_pre_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_notification_active = FALSE;
	zenity_notification_listen = FALSE;

	return TRUE;
}

static gboolean
zenity_progress_pre_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_progress_active = FALSE;
	zenity_progress_percentage = 0;
	zenity_progress_pulsate = FALSE;
	zenity_progress_auto_close = FALSE;
	zenity_progress_auto_kill = FALSE;
	zenity_progress_no_cancel = FALSE;
	zenity_progress_time_remaining = FALSE;

	return TRUE;
}

static gboolean
zenity_question_pre_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_question_active = FALSE;
	zenity_question_default_cancel = FALSE;
	zenity_question_switch = FALSE;

	return TRUE;
}

static gboolean
zenity_text_pre_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_text_active = FALSE;
	zenity_text_font = NULL;
	zenity_text_checkbox = NULL;
	zenity_text_auto_scroll = FALSE;
#ifdef HAVE_WEBKITGTK
	zenity_text_enable_html = FALSE;
	zenity_text_no_interaction = FALSE;
	zenity_text_url = NULL;
#endif

	return TRUE;
}

static gboolean
zenity_warning_pre_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_warning_active = FALSE;

	return TRUE;
}

static gboolean
zenity_scale_pre_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_scale_active = FALSE;
	zenity_scale_value = 0;
	zenity_scale_min_value = 0;
	zenity_scale_max_value = 100;
	zenity_scale_step = 1;
	zenity_scale_print_partial = FALSE;
	zenity_scale_hide_value = FALSE;

	return TRUE;
}

static gboolean
zenity_color_pre_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_colorsel_active = FALSE;
	zenity_colorsel_color = NULL;
	zenity_colorsel_show_palette = FALSE;

	return TRUE;
}

static gboolean
zenity_password_pre_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_password_active = FALSE;
	zenity_password_show_username = FALSE;

	return TRUE;
}

static gboolean
zenity_forms_pre_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_forms_active = FALSE;
	zenity_forms_show_header = FALSE;
	zenity_forms_date_format = NULL;

	return TRUE;
}

static gboolean
zenity_misc_pre_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_misc_about = FALSE;
	zenity_misc_version = FALSE;

	return TRUE;
}

/* Post parse callbacks assign the option values to
   parsing result and makes some post condition tests */

static gboolean
zenity_general_post_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	results->data->dialog_title = zenity_general_dialog_title;
	results->data->width = zenity_general_width;
	results->data->height = zenity_general_height;
	results->data->timeout_delay = zenity_general_timeout_delay;
	results->data->ok_label = zenity_general_ok_button;
	results->data->cancel_label = zenity_general_cancel_button;
	results->data->extra_label = zenity_general_extra_buttons;
	results->data->modal = zenity_general_modal;

	/* Deprecated options */

	if (zenity_general_window_icon_DEPRECATED)
	{
		zenity_general_icon = zenity_general_window_icon_DEPRECATED;
		show_window_icon_deprecation_warning ();
	}
	if (zenity_general_icon_name_DEPRECATED)
	{
		zenity_general_icon = zenity_general_icon_name_DEPRECATED;
		show_icon_name_deprecation_warning ();
	}

	if (zenity_general_attach_DEPRECATED)
		show_attach_deprecation_warning ();

	return TRUE;
}

static gboolean
zenity_calendar_post_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_option_set_dialog_mode (zenity_calendar_active, MODE_CALENDAR);

	if (results->mode == MODE_CALENDAR)
	{
		struct tm *t;
		time_t current_time;

		time (&current_time);
		t = localtime (&current_time);

		if (zenity_calendar_day < 0)
			zenity_calendar_day = t->tm_mday;
		if (zenity_calendar_month < 0)
			zenity_calendar_month = t->tm_mon + 1;
		if (zenity_calendar_year < 0)
			zenity_calendar_year = t->tm_year + 1900;

		results->calendar_data->dialog_text = zenity_general_dialog_text;
		results->calendar_data->day = zenity_calendar_day;
		results->calendar_data->month = zenity_calendar_month;
		results->calendar_data->year = zenity_calendar_year;

		if (zenity_calendar_date_format) {
			results->calendar_data->date_format = zenity_calendar_date_format;
		} else {
			results->calendar_data->date_format =
				g_locale_to_utf8 (nl_langinfo (D_FMT), -1, NULL, NULL, NULL);
		}
	}
	else
	{
		if (zenity_calendar_day > -1) {
			zenity_option_error (zenity_option_get_name (calendar_options,
						&zenity_calendar_day),
				ERROR_SUPPORT);
		}

		if (zenity_calendar_month > -1) {
			zenity_option_error (zenity_option_get_name (calendar_options,
						&zenity_calendar_month),
				ERROR_SUPPORT);
		}

		if (zenity_calendar_year > -1) {
			zenity_option_error (zenity_option_get_name (calendar_options,
						&zenity_calendar_year),
				ERROR_SUPPORT);
		}

		if (zenity_calendar_date_format) {
			zenity_option_error (zenity_option_get_name (calendar_options,
						&zenity_calendar_date_format),
				ERROR_SUPPORT);
		}
	}
	return TRUE;
}

static gboolean
zenity_entry_post_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_option_set_dialog_mode (zenity_entry_active, MODE_ENTRY);

	if (results->mode == MODE_ENTRY)
	{
		results->entry_data->dialog_text = zenity_general_dialog_text;
		results->entry_data->entry_text = zenity_entry_entry_text;
		results->entry_data->hide_text = zenity_entry_hide_text;
	}
	else
	{
		if (zenity_entry_entry_text) {
			zenity_option_error (zenity_option_get_name (entry_options,
						&zenity_entry_entry_text),
				ERROR_SUPPORT);
		}

		if (zenity_entry_hide_text) {
			zenity_option_error (zenity_option_get_name (entry_options,
						&zenity_entry_hide_text),
				ERROR_SUPPORT);
		}
	}
	return TRUE;
}

static gboolean
zenity_error_post_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_option_set_dialog_mode (zenity_error_active, MODE_ERROR);

	if (results->mode == MODE_ERROR)
	{
		results->msg_data->dialog_text = zenity_general_dialog_text;
		results->msg_data->dialog_icon = zenity_general_icon;
		results->msg_data->mode = ZENITY_MSG_ERROR;
		results->msg_data->no_wrap = zenity_general_dialog_no_wrap;
		results->msg_data->no_markup = zenity_general_dialog_no_markup;
		results->msg_data->ellipsize = zenity_general_dialog_ellipsize;
	}

	return TRUE;
}

static gboolean
zenity_info_post_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_option_set_dialog_mode (zenity_info_active, MODE_INFO);

	if (results->mode == MODE_INFO)
	{
		results->msg_data->dialog_text = zenity_general_dialog_text;
		results->msg_data->dialog_icon = zenity_general_icon;
		results->msg_data->mode = ZENITY_MSG_INFO;
		results->msg_data->no_wrap = zenity_general_dialog_no_wrap;
		results->msg_data->no_markup = zenity_general_dialog_no_markup;
		results->msg_data->ellipsize = zenity_general_dialog_ellipsize;
	}

	return TRUE;
}

static gboolean
zenity_file_post_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_option_set_dialog_mode (zenity_file_active, MODE_FILE);

	if (results->mode == MODE_FILE)
	{
		results->file_data->uri = zenity_general_uri;
		results->file_data->multi = zenity_general_multiple;
		results->file_data->directory = zenity_file_directory;
		results->file_data->save = zenity_file_save;
		results->file_data->separator = zenity_general_separator;
		results->file_data->filter = zenity_file_filter;
	}
	else
	{
		if (zenity_file_directory) {
			zenity_option_error (zenity_option_get_name (file_selection_options,
						&zenity_file_directory),
				ERROR_SUPPORT);
		}

		if (zenity_file_save) {
			zenity_option_error (zenity_option_get_name (file_selection_options,
						&zenity_file_save),
				ERROR_SUPPORT);
		}

		if (zenity_file_filter) {
			zenity_option_error (zenity_option_get_name (file_selection_options,
						&zenity_file_filter),
				ERROR_SUPPORT);
		}
	}

	if (zenity_file_confirm_overwrite_DEPRECATED)
		show_confirm_overwrite_deprecation_warning ();

	return TRUE;
}

static gboolean
zenity_list_post_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	int i = 0;
	char *column;

	zenity_option_set_dialog_mode (zenity_list_active, MODE_LIST);

	if (results->mode == MODE_LIST)
	{
		results->tree_data->dialog_text = zenity_general_dialog_text;

		if (zenity_list_columns)
		{
			column = zenity_list_columns[0];

			while (column != NULL)
			{
				results->tree_data->columns =
					g_slist_append (results->tree_data->columns, column);
				column = zenity_list_columns[++i];
			}
		}

		results->tree_data->checkbox = zenity_list_checklist;
		results->tree_data->radiobox = zenity_list_radiolist;
		results->tree_data->imagebox = zenity_list_imagelist;
		results->tree_data->multi = zenity_general_multiple;
		results->tree_data->editable = zenity_general_editable;
		results->tree_data->print_column = zenity_list_print_column;
		results->tree_data->hide_column = zenity_list_hide_column;
		results->tree_data->hide_header = zenity_list_hide_header;
		results->tree_data->separator = zenity_general_separator;
		results->tree_data->mid_search_DEPRECATED = zenity_list_mid_search_DEPRECATED;
	}
	else
	{
		if (zenity_list_columns)
			zenity_option_error (
				zenity_option_get_name (list_options, &zenity_list_columns),
				ERROR_SUPPORT);

		if (zenity_list_checklist)
			zenity_option_error (
				zenity_option_get_name (list_options, &zenity_list_checklist),
				ERROR_SUPPORT);

		if (zenity_list_radiolist)
			zenity_option_error (
				zenity_option_get_name (list_options, &zenity_list_radiolist),
				ERROR_SUPPORT);

		if (zenity_list_imagelist)
			zenity_option_error (
				zenity_option_get_name (list_options, &zenity_list_imagelist),
				ERROR_SUPPORT);

		if (zenity_list_print_column)
			zenity_option_error (zenity_option_get_name (
									 list_options, &zenity_list_print_column),
				ERROR_SUPPORT);

		if (zenity_list_hide_column)
			zenity_option_error (
				zenity_option_get_name (list_options, &zenity_list_hide_column),
				ERROR_SUPPORT);

		if (zenity_list_hide_header)
			zenity_option_error (
				zenity_option_get_name (list_options, &zenity_list_hide_header),
				ERROR_SUPPORT);
		if (zenity_list_mid_search_DEPRECATED)
			zenity_option_error (
				zenity_option_get_name (list_options, &zenity_list_mid_search_DEPRECATED),
				ERROR_SUPPORT);
	}
	return TRUE;
}

static gboolean
zenity_notification_post_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_option_set_dialog_mode (zenity_notification_active,
			MODE_NOTIFICATION);

	if (results->mode == MODE_NOTIFICATION)
	{
		results->notification_data->notification_text =
			zenity_general_dialog_text;
		results->notification_data->listen = zenity_notification_listen;
		results->notification_data->icon = zenity_general_icon;
	}
	else
	{
		if (zenity_notification_listen) {
			zenity_option_error (zenity_option_get_name (notification_options,
						&zenity_notification_listen),
				ERROR_SUPPORT);
		}
	}

	if (zenity_notification_hints_DEPRECATED)
		show_notification_hints_deprecation_warning ();

	return TRUE;
}

static gboolean
zenity_progress_post_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_option_set_dialog_mode (zenity_progress_active, MODE_PROGRESS);
	if (results->mode == MODE_PROGRESS)
	{
		results->progress_data->dialog_text = zenity_general_dialog_text;
		results->progress_data->pulsate = zenity_progress_pulsate;
		results->progress_data->autoclose = zenity_progress_auto_close;
		results->progress_data->autokill = zenity_progress_auto_kill;
		results->progress_data->percentage = zenity_progress_percentage;
		results->progress_data->no_cancel = zenity_progress_no_cancel;
		results->progress_data->time_remaining = zenity_progress_time_remaining;
	}
	else
	{
		if (zenity_progress_pulsate) {
			zenity_option_error (zenity_option_get_name (progress_options,
						&zenity_progress_pulsate),
				ERROR_SUPPORT);
		}

		if (zenity_progress_percentage) {
			zenity_option_error (zenity_option_get_name (progress_options,
						&zenity_progress_percentage),
				ERROR_SUPPORT);
		}

		if (zenity_progress_auto_close) {
			zenity_option_error (zenity_option_get_name (progress_options,
						&zenity_progress_auto_close),
				ERROR_SUPPORT);
		}

		if (zenity_progress_auto_kill) {
			zenity_option_error (zenity_option_get_name (progress_options,
						&zenity_progress_auto_kill),
				ERROR_SUPPORT);
		}

		if (zenity_progress_no_cancel) {
			zenity_option_error (zenity_option_get_name (progress_options,
						&zenity_progress_no_cancel),
				ERROR_SUPPORT);
		}

		if (zenity_progress_time_remaining) {
			zenity_option_error (zenity_option_get_name (progress_options,
						&zenity_progress_time_remaining),
				ERROR_SUPPORT);
		}
	}
	return TRUE;
}

static gboolean
zenity_question_post_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_option_set_dialog_mode (zenity_question_active, MODE_QUESTION);

	if (results->mode == MODE_QUESTION)
	{
		results->msg_data->dialog_text = zenity_general_dialog_text;
		results->msg_data->dialog_icon = zenity_general_icon;
		if (zenity_question_switch)
			results->msg_data->mode = ZENITY_MSG_SWITCH;
		else
			results->msg_data->mode = ZENITY_MSG_QUESTION;
		results->msg_data->no_wrap = zenity_general_dialog_no_wrap;
		results->msg_data->no_markup = zenity_general_dialog_no_markup;
		results->msg_data->ellipsize = zenity_general_dialog_ellipsize;
		results->msg_data->default_cancel = zenity_question_default_cancel;
	}

	if (zenity_question_switch && zenity_general_extra_buttons == NULL)
	{
		zenity_option_error (zenity_option_get_name (question_options,
					&zenity_question_switch),
				ERROR_SYNTAX);
	}
	return TRUE;
}

static gboolean
zenity_text_post_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_option_set_dialog_mode (zenity_text_active, MODE_TEXTINFO);

	if (results->mode == MODE_TEXTINFO)
	{
		results->text_data->uri = zenity_general_uri;
		results->text_data->editable = zenity_general_editable;
		results->text_data->no_wrap = zenity_general_dialog_no_wrap;
		results->text_data->font = zenity_text_font;
		results->text_data->checkbox = zenity_text_checkbox;
		results->text_data->auto_scroll = zenity_text_auto_scroll;
#ifdef HAVE_WEBKITGTK
		results->text_data->html = zenity_text_enable_html;
		results->text_data->no_interaction = zenity_text_no_interaction;
		results->text_data->url = zenity_text_url;
#endif
	}
	else
	{
		if (zenity_text_font) {
			zenity_option_error (zenity_option_get_name (text_options,
						&zenity_text_font),
					ERROR_SUPPORT);
		}
#ifdef HAVE_WEBKITGTK
		if (zenity_text_enable_html) {
			zenity_option_error (zenity_option_get_name (text_options,
						&zenity_text_enable_html),
				ERROR_SUPPORT);
		}
#endif
	}
	return TRUE;
}

static gboolean
zenity_warning_post_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_option_set_dialog_mode (zenity_warning_active, MODE_WARNING);

	if (results->mode == MODE_WARNING)
	{
		results->msg_data->dialog_text = zenity_general_dialog_text;
		results->msg_data->dialog_icon = zenity_general_icon;
		results->msg_data->mode = ZENITY_MSG_WARNING;
		results->msg_data->no_wrap = zenity_general_dialog_no_wrap;
		results->msg_data->no_markup = zenity_general_dialog_no_markup;
		results->msg_data->ellipsize = zenity_general_dialog_ellipsize;
	}

	return TRUE;
}

static gboolean
zenity_scale_post_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_option_set_dialog_mode (zenity_scale_active, MODE_SCALE);

	if (results->mode == MODE_SCALE)
	{
		results->scale_data->dialog_text = zenity_general_dialog_text;
		results->scale_data->value = zenity_scale_value;
		results->scale_data->min_value = zenity_scale_min_value;
		results->scale_data->max_value = zenity_scale_max_value;
		results->scale_data->step = zenity_scale_step;
		results->scale_data->print_partial = zenity_scale_print_partial;
		results->scale_data->hide_value = zenity_scale_hide_value;
	}

	return TRUE;
}

static gboolean
zenity_color_post_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_option_set_dialog_mode (zenity_colorsel_active, MODE_COLOR);

	if (results->mode == MODE_COLOR)
	{
		results->color_data->color = zenity_colorsel_color;
		results->color_data->show_palette = zenity_colorsel_show_palette;
	}
	else
	{
		if (zenity_colorsel_color) {
			zenity_option_error
				(zenity_option_get_name (color_selection_options,
										 &zenity_colorsel_color),
				 ERROR_SUPPORT);
		}

		if (zenity_colorsel_show_palette) {
			zenity_option_error
				(zenity_option_get_name (color_selection_options,
										 &zenity_colorsel_show_palette),
				 ERROR_SUPPORT);
		}
	}
	return TRUE;
}

static gboolean
zenity_forms_post_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	char *values;
	int i = 0;

	zenity_option_set_dialog_mode (zenity_forms_active, MODE_FORMS);

	if (results->mode == MODE_FORMS)
	{
		results->forms_data->dialog_text = zenity_general_dialog_text;
		results->forms_data->separator = zenity_general_separator;
		results->forms_data->show_header = zenity_forms_show_header;

		if (zenity_forms_list_values) {
			values = zenity_forms_list_values[0];
			while (values != NULL) {
				results->forms_data->list_values =
					g_slist_append (results->forms_data->list_values, values);
				values = zenity_forms_list_values[++i];
			}
		}
		if (zenity_forms_column_values) {
			i = 0;
			values = zenity_forms_column_values[0];
			while (values != NULL) {
				results->forms_data->column_values =
					g_slist_append (results->forms_data->column_values, values);
				values = zenity_forms_list_values[++i];
			}
		} else
			results->forms_data->column_values =
				g_slist_append (NULL, "column");

		if (zenity_forms_combo_values) {
			i = 0;
			values = zenity_forms_combo_values[0];
			while (values != NULL) {
				results->forms_data->combo_values =
					g_slist_append (results->forms_data->combo_values, values);
				values = zenity_forms_combo_values[++i];
			}
		}
		if (zenity_forms_date_format)
			results->forms_data->date_format = zenity_forms_date_format;
		else
			results->forms_data->date_format =
				g_locale_to_utf8 (nl_langinfo (D_FMT), -1, NULL, NULL, NULL);
	}
	else
	{
		if (zenity_forms_date_format)
			zenity_option_error (zenity_option_get_name (forms_dialog_options,
									 &zenity_forms_date_format),
				ERROR_SUPPORT);
		if (zenity_forms_list_values)
			zenity_option_error (zenity_option_get_name (forms_dialog_options,
									 &zenity_forms_list_values),
				ERROR_SUPPORT);
		if (zenity_forms_column_values)
			zenity_option_error (zenity_option_get_name (forms_dialog_options,
									 &zenity_forms_column_values),
				ERROR_SUPPORT);
		if (zenity_forms_combo_values)
			zenity_option_error (zenity_option_get_name (forms_dialog_options,
									 &zenity_forms_combo_values),
				ERROR_SUPPORT);
		if (zenity_forms_show_header)
			zenity_option_error (zenity_option_get_name (forms_dialog_options,
									 &zenity_forms_show_header),
				ERROR_SUPPORT);
	}

	return TRUE;
}

static gboolean
zenity_password_post_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_option_set_dialog_mode (zenity_password_active, MODE_PASSWORD);

	if (results->mode == MODE_PASSWORD) {
		results->password_data->username = zenity_password_show_username;
	} else {
		if (zenity_password_show_username)
			zenity_option_error
				(zenity_option_get_name (password_dialog_options,
										 &zenity_password_show_username),
				 ERROR_SUPPORT);
	}
	return TRUE;
}

static gboolean
zenity_misc_post_callback (GOptionContext *context, GOptionGroup *group,
	gpointer data, GError **error)
{
	zenity_option_set_dialog_mode (zenity_misc_about, MODE_ABOUT);
	zenity_option_set_dialog_mode (zenity_misc_version, MODE_VERSION);

	return TRUE;
}

static GOptionContext *
zenity_create_context (void)
{
	GOptionContext *tmp_ctx;
	GOptionGroup *a_group;

	tmp_ctx = g_option_context_new (NULL);

	/* Adds general option entries */
	a_group = g_option_group_new ("general",
		N_ ("General options"),
		N_ ("Show general options"),
		NULL,
		NULL);
	g_option_group_add_entries (a_group, general_options);
	g_option_group_set_parse_hooks (a_group,
			zenity_general_pre_callback, zenity_general_post_callback);
	g_option_group_set_error_hook (a_group, zenity_option_error_callback);
	g_option_group_set_translation_domain (a_group, GETTEXT_PACKAGE);
	g_option_context_add_group (tmp_ctx, a_group);

	/* Adds calendar option entries */
	a_group = g_option_group_new ("calendar",
		N_ ("Calendar options"),
		N_ ("Show calendar options"),
		NULL,
		NULL);
	g_option_group_add_entries (a_group, calendar_options);
	g_option_group_set_parse_hooks (a_group,
			zenity_calendar_pre_callback, zenity_calendar_post_callback);
	g_option_group_set_error_hook (a_group, zenity_option_error_callback);
	g_option_group_set_translation_domain (a_group, GETTEXT_PACKAGE);
	g_option_context_add_group (tmp_ctx, a_group);

	/* Adds entry option entries */
	a_group = g_option_group_new ("entry",
		N_ ("Text entry options"),
		N_ ("Show text entry options"),
		NULL,
		NULL);
	g_option_group_add_entries (a_group, entry_options);
	g_option_group_set_parse_hooks (a_group,
			zenity_entry_pre_callback, zenity_entry_post_callback);
	g_option_group_set_error_hook (a_group, zenity_option_error_callback);
	g_option_group_set_translation_domain (a_group, GETTEXT_PACKAGE);
	g_option_context_add_group (tmp_ctx, a_group);

	/* Adds error option entries */
	a_group = g_option_group_new ("error",
			N_ ("Error options"), N_ ("Show error options"), NULL, NULL);
	g_option_group_add_entries (a_group, error_options);
	g_option_group_set_parse_hooks (
		a_group, zenity_error_pre_callback, zenity_error_post_callback);
	g_option_group_set_error_hook (a_group, zenity_option_error_callback);
	g_option_group_set_translation_domain (a_group, GETTEXT_PACKAGE);
	g_option_context_add_group (tmp_ctx, a_group);

	/* Adds info option entries */
	a_group = g_option_group_new ("info",
			N_ ("Info options"), N_ ("Show info options"), NULL, NULL);
	g_option_group_add_entries (a_group, info_options);
	g_option_group_set_parse_hooks (
		a_group, zenity_info_pre_callback, zenity_info_post_callback);
	g_option_group_set_error_hook (a_group, zenity_option_error_callback);
	g_option_group_set_translation_domain (a_group, GETTEXT_PACKAGE);
	g_option_context_add_group (tmp_ctx, a_group);

	/* Adds file selection option entries */
	a_group = g_option_group_new ("file-selection",
		N_ ("File selection options"),
		N_ ("Show file selection options"),
		NULL,
		NULL);
	g_option_group_add_entries (a_group, file_selection_options);
	g_option_group_set_parse_hooks (
		a_group, zenity_file_pre_callback, zenity_file_post_callback);
	g_option_group_set_error_hook (a_group, zenity_option_error_callback);
	g_option_group_set_translation_domain (a_group, GETTEXT_PACKAGE);
	g_option_context_add_group (tmp_ctx, a_group);

	/* Adds list option entries */
	a_group = g_option_group_new (
		"list", N_ ("List options"), N_ ("Show list options"), NULL, NULL);
	g_option_group_add_entries (a_group, list_options);
	g_option_group_set_parse_hooks (
		a_group, zenity_list_pre_callback, zenity_list_post_callback);
	g_option_group_set_error_hook (a_group, zenity_option_error_callback);
	g_option_group_set_translation_domain (a_group, GETTEXT_PACKAGE);
	g_option_context_add_group (tmp_ctx, a_group);

	/* Adds notification option entries */
	a_group = g_option_group_new ("notification",
		N_ ("Notification options"),
		N_ ("Show notification options"),
		NULL,
		NULL);
	g_option_group_add_entries (a_group, notification_options);
	g_option_group_set_parse_hooks (a_group,
		zenity_notification_pre_callback,
		zenity_notification_post_callback);
	g_option_group_set_error_hook (a_group, zenity_option_error_callback);
	g_option_group_set_translation_domain (a_group, GETTEXT_PACKAGE);
	g_option_context_add_group (tmp_ctx, a_group);

	/* Adds progress option entries */
	a_group = g_option_group_new ("progress",
		N_ ("Progress options"),
		N_ ("Show progress options"),
		NULL,
		NULL);
	g_option_group_add_entries (a_group, progress_options);
	g_option_group_set_parse_hooks (
		a_group, zenity_progress_pre_callback, zenity_progress_post_callback);
	g_option_group_set_error_hook (a_group, zenity_option_error_callback);
	g_option_group_set_translation_domain (a_group, GETTEXT_PACKAGE);
	g_option_context_add_group (tmp_ctx, a_group);

	/* Adds question option entries */
	a_group = g_option_group_new ("question",
		N_ ("Question options"),
		N_ ("Show question options"),
		NULL,
		NULL);
	g_option_group_add_entries (a_group, question_options);
	g_option_group_set_parse_hooks (a_group,
			zenity_question_pre_callback, zenity_question_post_callback);
	g_option_group_set_error_hook (a_group, zenity_option_error_callback);
	g_option_group_set_translation_domain (a_group, GETTEXT_PACKAGE);
	g_option_context_add_group (tmp_ctx, a_group);

	/* Adds warning option entries */
	a_group = g_option_group_new ("warning",
		N_ ("Warning options"),
		N_ ("Show warning options"),
		NULL,
		NULL);
	g_option_group_add_entries (a_group, warning_options);
	g_option_group_set_parse_hooks (a_group,
			zenity_warning_pre_callback, zenity_warning_post_callback);
	g_option_group_set_error_hook (a_group, zenity_option_error_callback);
	g_option_group_set_translation_domain (a_group, GETTEXT_PACKAGE);
	g_option_context_add_group (tmp_ctx, a_group);

	/* Adds scale option entries */
	a_group = g_option_group_new ("scale",
			N_ ("Scale options"), N_ ("Show scale options"), NULL, NULL);
	g_option_group_add_entries (a_group, scale_options);
	g_option_group_set_parse_hooks (
		a_group, zenity_scale_pre_callback, zenity_scale_post_callback);
	g_option_group_set_error_hook (a_group, zenity_option_error_callback);
	g_option_group_set_translation_domain (a_group, GETTEXT_PACKAGE);
	g_option_context_add_group (tmp_ctx, a_group);

	/* Adds text option entries */
	a_group = g_option_group_new ("text-info",
		N_ ("Text information options"),
		N_ ("Show text information options"),
		NULL,
		NULL);
	g_option_group_add_entries (a_group, text_options);
	g_option_group_set_parse_hooks (
		a_group, zenity_text_pre_callback, zenity_text_post_callback);
	g_option_group_set_error_hook (a_group, zenity_option_error_callback);
	g_option_group_set_translation_domain (a_group, GETTEXT_PACKAGE);
	g_option_context_add_group (tmp_ctx, a_group);

	/* Adds color selection option entries */
	a_group = g_option_group_new ("color-selection",
		N_ ("Color selection options"),
		N_ ("Show color selection options"),
		NULL,
		NULL);
	g_option_group_add_entries (a_group, color_selection_options);
	g_option_group_set_parse_hooks (a_group,
			zenity_color_pre_callback, zenity_color_post_callback);
	g_option_group_set_error_hook (a_group, zenity_option_error_callback);
	g_option_group_set_translation_domain (a_group, GETTEXT_PACKAGE);
	g_option_context_add_group (tmp_ctx, a_group);

	/* Adds password dialog option entries */
	a_group = g_option_group_new ("password",
		N_ ("Password dialog options"),
		N_ ("Show password dialog options"),
		NULL,
		NULL);
	g_option_group_add_entries (a_group, password_dialog_options);
	g_option_group_set_parse_hooks (a_group,
			zenity_password_pre_callback, zenity_password_post_callback);
	g_option_group_set_error_hook (a_group, zenity_option_error_callback);
	g_option_group_set_translation_domain (a_group, GETTEXT_PACKAGE);
	g_option_context_add_group (tmp_ctx, a_group);

	/* Adds forms dialog option entries */
	a_group = g_option_group_new ("forms",
		N_ ("Forms dialog options"),
		N_ ("Show forms dialog options"),
		NULL,
		NULL);
	g_option_group_add_entries (a_group, forms_dialog_options);
	g_option_group_set_parse_hooks (a_group,
			zenity_forms_pre_callback, zenity_forms_post_callback);
	g_option_group_set_error_hook (a_group, zenity_option_error_callback);
	g_option_group_set_translation_domain (a_group, GETTEXT_PACKAGE);
	g_option_context_add_group (tmp_ctx, a_group);

	/* Adds misc option entries */
	a_group = g_option_group_new ("misc",
		N_ ("Miscellaneous options"),
		N_ ("Show miscellaneous options"),
		NULL,
		NULL);
	g_option_group_add_entries (a_group, miscellaneous_options);
	g_option_group_set_parse_hooks (a_group,
			zenity_misc_pre_callback, zenity_misc_post_callback);
	g_option_group_set_error_hook (a_group, zenity_option_error_callback);
	g_option_group_set_translation_domain (a_group, GETTEXT_PACKAGE);
	g_option_context_add_group (tmp_ctx, a_group);

	/* Enable help options */
	g_option_context_set_help_enabled (tmp_ctx, TRUE);
	g_option_context_set_ignore_unknown_options (tmp_ctx, FALSE);

	return tmp_ctx;
}

void
zenity_option_error (char *string, ZenityError error)
{
	switch (error)
	{
		case ERROR_SYNTAX:
			g_printerr (_ ("This option is not available. Please see --help "
						   "for all possible usages.\n"));
			exit (-1);

		case ERROR_SUPPORT:
			g_printerr (_ ("--%s is not supported for this dialog\n"), string);
			exit (-1);

		case ERROR_DIALOG:
			g_printerr (_ ("Two or more dialog options specified\n"));
			exit (-1);

		default:
			return;
	}
}

ZenityParsingOptions *
zenity_option_parse (int argc, char **argv)
{
	g_autoptr(GOptionContext) context = NULL;
	GError *error = NULL;

	zenity_option_init ();

	context = zenity_create_context ();

	g_option_context_parse (context, &argc, &argv, &error);

	if (G_UNLIKELY (error))
	{
		g_printerr ("%s\n", error->message);
		exit (-1);
	}

	/* Some option pointer a shared among more than one group and don't
	   have their post condition tested. This test is done here. */

	if (zenity_general_dialog_text)
		if (results->mode == MODE_ABOUT || results->mode == MODE_VERSION)
			zenity_option_error (zenity_option_get_name (calendar_options,
									 &zenity_general_dialog_text),
				ERROR_SUPPORT);

	if (strcmp (zenity_general_separator, "|") != 0)
		if (results->mode != MODE_LIST && results->mode != MODE_FILE &&
			results->mode != MODE_FORMS)
			zenity_option_error (zenity_option_get_name (
									 list_options, &zenity_general_separator),
				ERROR_SUPPORT);

	if (zenity_general_multiple)
		if (results->mode != MODE_FILE && results->mode != MODE_LIST)
			zenity_option_error (
				zenity_option_get_name (list_options, &zenity_general_multiple),
				ERROR_SUPPORT);

	if (zenity_general_editable)
		if (results->mode != MODE_TEXTINFO && results->mode != MODE_LIST)
			zenity_option_error (
				zenity_option_get_name (list_options, &zenity_general_editable),
				ERROR_SUPPORT);

	if (zenity_general_uri)
		if (results->mode != MODE_FILE && results->mode != MODE_TEXTINFO)
			zenity_option_error (
				zenity_option_get_name (text_options, &zenity_general_uri),
				ERROR_SUPPORT);

	if (zenity_general_ok_button)
		if (results->mode == MODE_FILE)
			zenity_option_error (zenity_option_get_name (general_options,
									 &zenity_general_ok_button),
				ERROR_SUPPORT);

	if (zenity_general_cancel_button)
		if (results->mode == MODE_FILE || results->mode == MODE_ERROR ||
			results->mode == MODE_WARNING || results->mode == MODE_INFO)
			zenity_option_error (zenity_option_get_name (general_options,
									 &zenity_general_cancel_button),
				ERROR_SUPPORT);

	if (zenity_general_dialog_no_wrap)
		if (results->mode != MODE_INFO && results->mode != MODE_ERROR &&
			results->mode != MODE_QUESTION && results->mode != MODE_WARNING &&
			results->mode != MODE_TEXTINFO)
			zenity_option_error (zenity_option_get_name (text_options,
									 &zenity_general_dialog_no_wrap),
				ERROR_SUPPORT);

	if (zenity_general_dialog_ellipsize)
		if (results->mode != MODE_INFO && results->mode != MODE_ERROR &&
			results->mode != MODE_QUESTION && results->mode != MODE_WARNING)
			zenity_option_error (zenity_option_get_name (text_options,
									 &zenity_general_dialog_ellipsize),
				ERROR_SUPPORT);

	return results;
}
