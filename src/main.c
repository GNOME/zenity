/*
 * main.c
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
 */

#include "config.h"
#include "zenity.h"
#include <popt.h>
#include <langinfo.h>

typedef enum {
	MODE_CALENDAR,
	MODE_ENTRY,
	MODE_ERROR,
	MODE_FILE,
	MODE_LIST,
	MODE_PROGRESS,
	MODE_QUESTION,
	MODE_TEXTINFO,
	MODE_WARNING,
	MODE_INFO,
        MODE_ABOUT,
	MODE_LAST
} ZenityDialogMode;

typedef enum {
        ERROR_DUPLICATE,
        ERROR_SUPPORT,
        ERROR_DIALOG,
        ERROR_LAST
} ZenityError;

typedef struct {
	ZenityDialogMode mode;
	ZenityData *data;

	ZenityCalendarData *calendar_data;
	ZenityMsgData *msg_data;
	ZenityFileData *file_data;
	ZenityEntryData *entry_data;
	ZenityProgressData *progress_data;
	ZenityTextData *text_data;
	ZenityTreeData *tree_data;
} ZenityParsingOptions;

enum {
	OPTION_CALENDAR = 1,
	OPTION_DATEFORMAT,
	OPTION_ENTRY,
	OPTION_ERROR,
	OPTION_INFO,
	OPTION_FILE,
	OPTION_LIST,
	OPTION_PROGRESS,
	OPTION_QUESTION,
	OPTION_TEXTINFO,
	OPTION_TEXTEDIT,
	OPTION_WARNING,
	OPTION_TITLE,
	OPTION_ICON,
	OPTION_CALENDARTEXT,
	OPTION_DAY,
	OPTION_MONTH,
	OPTION_YEAR,
	OPTION_ENTRYTEXT,
	OPTION_INPUTTEXT,
	OPTION_HIDETEXT,
	OPTION_ERRORTEXT,
	OPTION_INFOTEXT,
	OPTION_FILENAME,
	OPTION_COLUMN,
        OPTION_SEPERATOR,
	OPTION_CHECKLIST,
	OPTION_RADIOLIST,
	OPTION_PROGRESSTEXT,
	OPTION_PERCENTAGE,
	OPTION_PULSATE,
	OPTION_QUESTIONTEXT,
	OPTION_TEXTFILE,
	OPTION_WARNINGTEXT,
	OPTION_ABOUT,
	OPTION_VERSION,
	OPTION_LAST,
};

static void zenity_parse_options_callback (poptContext              ctx,
                                    	    enum poptCallbackReason  reason,
                                    	    const struct poptOption *opt,
                                    	    const char              *arg,
                                    	    void                    *data);

struct poptOption options[] = {
	{
		NULL,
		'\0',
		POPT_ARG_CALLBACK | POPT_CBFLAG_POST,
		zenity_parse_options_callback,
		0,
		NULL,
		NULL
	},
	{
		"calendar",
		'\0',
		POPT_ARG_NONE,
		NULL,
		OPTION_CALENDAR,
		N_("Display calendar dialog"),
		NULL
	},
	{
		"entry",
		'\0',
		POPT_ARG_NONE,
		NULL,
		OPTION_ENTRY,
		N_("Display text entry dialog"),
		NULL
	},
	{
		"error",
		'\0',
		POPT_ARG_NONE,
		NULL,
		OPTION_ERROR,
		N_("Display error dialog"),
		NULL
	},
	{
		"file-selection",
		'\0',
		POPT_ARG_NONE,
		NULL,
		OPTION_FILE,
		N_("Display file selection dialog"),
		NULL
	},
	{
		"info",
		'\0',
		POPT_ARG_NONE,
		NULL,
		OPTION_INFO,
		N_("Display info dialog"),
		NULL
	},
	{
		"list",
		'\0',
		POPT_ARG_NONE,
		NULL,
		OPTION_LIST,
		N_("Display list dialog"),
		NULL
	},
	{
		"progress",
		'\0',
		POPT_ARG_NONE,
		NULL,
		OPTION_PROGRESS,
		N_("Display progress indication dialog"),
		NULL
	},
	{
		"question",
		'\0',
		POPT_ARG_NONE,
		NULL,
		OPTION_QUESTION,
		N_("Display question dialog"),
		NULL
	},
	{
		"text-info",
		'\0',
		POPT_ARG_NONE,
		NULL,
		OPTION_TEXTINFO,
		N_("Display text information dialog"),
		NULL
	},
	{
		"warning",
		'\0',
		POPT_ARG_NONE,
		NULL,
		OPTION_WARNING,
		N_("Display warning dialog"),
		NULL
	},
	POPT_TABLEEND
};

struct poptOption general_options[] = {
	{
		NULL,
		'\0',
		POPT_ARG_CALLBACK | POPT_CBFLAG_POST,
		zenity_parse_options_callback,
		0,
		NULL,
		NULL
	},
	{
		"title",
		'\0',
		POPT_ARG_STRING,
		NULL,
		OPTION_TITLE,
		N_("Set the dialog title"),
		N_("TITLE")
	},
	{
		"window-icon",
		'\0',
		POPT_ARG_STRING,
		NULL,
		OPTION_ICON,
		N_("Set the window icon"),
		N_("ICONPATH")
	},
	POPT_TABLEEND
};

struct poptOption calendar_options[] = {
	{
		NULL,
		'\0',
		POPT_ARG_CALLBACK | POPT_CBFLAG_POST,
		zenity_parse_options_callback,
		0,
		NULL,
		NULL
	},
	{
		"text",
		'\0',
		POPT_ARG_STRING,
		NULL,
		OPTION_CALENDARTEXT,
		N_("Set the dialog text"),
		NULL
	},
	{
		"day",
		'\0',
		POPT_ARG_INT,
		NULL,
		OPTION_DAY,
		N_("Set the calendar day"),
		NULL
	},
	{
		"month",
		'\0',
		POPT_ARG_INT,
		NULL,
		OPTION_MONTH,
		N_("Set the calendar month"),
		NULL
	},
	{
		"year",
		'\0',
		POPT_ARG_INT,
		NULL,
		OPTION_YEAR,
		N_("Set the calendar year"),
		NULL
	},
	{	"date-format",
		'\0',
		POPT_ARG_STRING,
		NULL,
		OPTION_DATEFORMAT,
		N_("Set the format for the returned date"),
		NULL
	},
	POPT_TABLEEND
};

struct poptOption entry_options[] = {
	{
		NULL,
		'\0',
		POPT_ARG_CALLBACK | POPT_CBFLAG_POST,
		zenity_parse_options_callback,
		0,
		NULL,
		NULL
	},
	{
		"text",
		'\0',
		POPT_ARG_STRING,
		NULL,
		OPTION_ENTRYTEXT,
		N_("Set the dialog text"),
		NULL
	},
	{
		"entry-text",
		'\0',
		POPT_ARG_STRING,
		NULL,
		OPTION_INPUTTEXT,
		N_("Set the entry text"),
		NULL
	},
	{
		"hide-text",
		'\0',
		POPT_ARG_NONE,
		NULL,
		OPTION_HIDETEXT,
		N_("Hide the entry text"),
		NULL
	},
	POPT_TABLEEND
};

struct poptOption error_options[] = {
	{
		NULL,
		'\0',
		POPT_ARG_CALLBACK | POPT_CBFLAG_POST,
		zenity_parse_options_callback,
		0,
		NULL,
		NULL
	},
	{
		"text",
		'\0',
		POPT_ARG_STRING,
		NULL,
		OPTION_ERRORTEXT,
		N_("Set the dialog text"),
		NULL
	},
	POPT_TABLEEND
};

struct poptOption info_options[] = {
	{
		NULL,
		'\0',
		POPT_ARG_CALLBACK | POPT_CBFLAG_POST,
		zenity_parse_options_callback,
		0,
		NULL,
		NULL
	},
	{
		"text",
		'\0',
		POPT_ARG_STRING,
		NULL,
		OPTION_INFOTEXT,
		N_("Set the dialog text"),
		NULL
	},
	POPT_TABLEEND
};

struct poptOption file_selection_options[] = {
	{
		NULL,
		'\0',
		POPT_ARG_CALLBACK | POPT_CBFLAG_POST,
		zenity_parse_options_callback,
		0,
		NULL,
		NULL
	},
	{
		"filename",
		'\0',
		POPT_ARG_STRING,
		NULL,
		OPTION_FILENAME,
		N_("Set the filename"),
		N_("FILENAME")
	},
	POPT_TABLEEND
};

struct poptOption list_options[] = {
	{
		NULL,
		'\0',
		POPT_ARG_CALLBACK | POPT_CBFLAG_POST,
		zenity_parse_options_callback,
		0,
		NULL,
		NULL
	},
	{
		"column",
		'\0',
		POPT_ARG_STRING,
		NULL,
		OPTION_COLUMN,
		N_("Set the column header"),
		NULL
	},
	{
		"checklist",
		'\0',
		POPT_ARG_NONE,
		NULL,
		OPTION_CHECKLIST,
		N_("Use check boxes for first column"),
		NULL
	},
	{
		"radiolist",
		'\0',
		POPT_ARG_NONE,
		NULL,
		OPTION_RADIOLIST,
		N_("Use radio buttons for first column"),
		NULL
	},
	{
		"separator",
		'\0',
		POPT_ARG_STRING,
		NULL,
		OPTION_SEPERATOR,
		N_("Set output separator character"),
		NULL
	},
	POPT_TABLEEND
};

struct poptOption progress_options[] = {
	{
		NULL,
		'\0',
		POPT_ARG_CALLBACK | POPT_CBFLAG_POST,
		zenity_parse_options_callback,
		0,
		NULL,
		NULL
	},
	{
		"text",
		'\0',
		POPT_ARG_STRING,
		NULL,
		OPTION_PROGRESSTEXT,
		N_("Set the dialog text"),
		NULL
	},
	{
		"percentage",
		'\0',
		POPT_ARG_INT,
		NULL,
		OPTION_PERCENTAGE,
		N_("Set initial percentage"),
		NULL
	},
	{
		"pulsate",
		'\0',
		POPT_ARG_NONE,
		NULL,
		OPTION_PULSATE,
		N_("Pulsate progress bar"),
		NULL
	},
	POPT_TABLEEND
};
 
struct poptOption question_options[] = {
	{
		"text",
		'\0',
		POPT_ARG_STRING,
		NULL,
		OPTION_QUESTIONTEXT,
		N_("Set the dialog text"),
		NULL
	},
	POPT_TABLEEND
};

struct poptOption text_options[] = {
	{
		NULL,
		'\0',
		POPT_ARG_CALLBACK | POPT_CBFLAG_POST,
		zenity_parse_options_callback,
		0,
		NULL,
		NULL
	},
	{
		"filename",
		'\0',
		POPT_ARG_STRING,
		NULL,
		OPTION_TEXTFILE,
		N_("Open file"),
		N_("FILENAME")
	},
	{
		"editable",
		'\0',
		POPT_ARG_NONE,
		NULL,
		OPTION_TEXTEDIT,
		N_("Allow changes to text"),
		NULL
	},
	POPT_TABLEEND
};

struct poptOption warning_options[] = {
	{
		NULL,
		'\0',
		POPT_ARG_CALLBACK | POPT_CBFLAG_POST,
		zenity_parse_options_callback,
		0,
		NULL,
		NULL
	},
	{
		"text",
		'\0',
		POPT_ARG_STRING,
		NULL,
		OPTION_WARNINGTEXT,
		N_("Set the dialog text"),
		NULL
	},
	POPT_TABLEEND
};

struct poptOption miscellaneous_options[] = {
	{
		NULL,
		'\0',
		POPT_ARG_CALLBACK | POPT_CBFLAG_POST,
		zenity_parse_options_callback,
		0,
		NULL,
		NULL
	},
	{
		"about",
		'\0',
		POPT_ARG_NONE,
		NULL,
		OPTION_ABOUT,
		N_("About zenity"),
		NULL
	},
	{
		"version",
		'\0',
		POPT_ARG_NONE,
		NULL,
		OPTION_VERSION,
		N_("Print version"),
		NULL
	},
	POPT_TABLEEND
};

struct poptOption application_options[] = {
	{
		NULL,
		'\0',
		POPT_ARG_INCLUDE_TABLE,
		options,
		0,
		N_("Dialog options"),
		NULL
	},
	{
		NULL,
		'\0',
		POPT_ARG_INCLUDE_TABLE,
		general_options,
		0,
		N_("General options"),
		NULL
	},
	{
		NULL,
		'\0',
		POPT_ARG_INCLUDE_TABLE,
		calendar_options,
		0,
		N_("Calendar options"),
		NULL
	},
	{
		NULL,
		'\0',
		POPT_ARG_INCLUDE_TABLE,
		entry_options,
		0,
		N_("Text entry options"),
		NULL
	},
	{
		NULL,
		'\0',
		POPT_ARG_INCLUDE_TABLE,
		error_options,
		0,
		N_("Error options"),
		NULL
	},
	{
		NULL,
		'\0',
		POPT_ARG_INCLUDE_TABLE,
		file_selection_options,
		0,
		N_("File selection options"),
		NULL
	},
	{
		NULL,
		'\0',
		POPT_ARG_INCLUDE_TABLE,
		info_options,
		0,
		N_("Info options"),
		NULL
	},
	{
		NULL,
		'\0',
		POPT_ARG_INCLUDE_TABLE,
		list_options,
		0,
		N_("List options"),
		NULL
	},
	{
		NULL,
		'\0',
		POPT_ARG_INCLUDE_TABLE,
		progress_options,
		0,
		N_("Progress options"),
		NULL
	},
	{
		NULL,
		'\0',
		POPT_ARG_INCLUDE_TABLE,
		question_options,
		0,
		N_("Question options"),
		NULL
	},
	{
		NULL,
		'\0',
		POPT_ARG_INCLUDE_TABLE,
		text_options,
		0,
		N_("Text options"),
		NULL
	},
	{
		NULL,
		'\0',
		POPT_ARG_INCLUDE_TABLE,
		warning_options,
		0,
		N_("Warning options"),
		NULL
	},
	{
		NULL,
		'\0',
		POPT_ARG_INCLUDE_TABLE,
		miscellaneous_options,
		0,
		N_("Miscellaneous options"),
		NULL
	},
	{
		NULL,
		'\0',
		POPT_ARG_INCLUDE_TABLE,
		poptHelpOptions,
		0,
		N_("Help options"), 
		NULL
	},
	POPT_TABLEEND
};

ZenityParsingOptions *results;

static void
zenity_init_parsing_options (void) {

	results = g_new0 (ZenityParsingOptions, 1);

	/* Initialize the various dialog structures */
	results->mode = MODE_LAST;
	results->data = g_new0 (ZenityData, 1);
	results->calendar_data = g_new0 (ZenityCalendarData, 1);
	results->msg_data = g_new0 (ZenityMsgData, 1);
	results->file_data = g_new0 (ZenityFileData, 1);
	results->entry_data = g_new0 (ZenityEntryData, 1); 
	results->progress_data = g_new0 (ZenityProgressData, 1); 
	results->text_data = g_new0 (ZenityTextData, 1);
	results->tree_data = g_new0 (ZenityTreeData, 1);

	/* Give some sensible defaults */
	results->calendar_data->date_format = g_strdup (nl_langinfo (D_FMT));
	results->calendar_data->day = 0;
	results->calendar_data->month = 0;
	results->calendar_data->year = 0;
	results->calendar_data->dialog_text = NULL;
	results->text_data->editable = FALSE;
        results->tree_data->separator = g_strdup ("/");
	results->progress_data->percentage = -1;
	results->progress_data->pulsate = FALSE;
	results->entry_data->visible = TRUE;
	results->tree_data->checkbox = FALSE;
	results->tree_data->radiobox = FALSE;
}

static void
zenity_free_parsing_options (void) {

	/* General options */
	if (results->data->dialog_title)
		g_free (results->data->dialog_title);
	if (results->data->window_icon)
		g_free (results->data->window_icon);

	/* Dialog options */
	switch (results->mode) {
		case MODE_CALENDAR:
			if (results->calendar_data->dialog_text)
				g_free (results->calendar_data->dialog_text);
			if (results->calendar_data->date_format)
				g_free (results->calendar_data->date_format);
			break;
		case MODE_ENTRY:
			if (results->entry_data->dialog_text)
				g_free (results->entry_data->dialog_text);
			if (results->entry_data->entry_text)
				g_free (results->entry_data->entry_text);
			break;
		case MODE_ERROR:
		case MODE_QUESTION:
		case MODE_WARNING:
		case MODE_INFO:
			if (results->msg_data->dialog_text)
				g_free (results->msg_data->dialog_text);
			break;
		case MODE_FILE:
			if (results->file_data->uri)
				g_free (results->file_data->uri);
			break;
		case MODE_TEXTINFO:
			if (results->text_data->uri)
				g_free (results->text_data->uri);
			break;
		case MODE_LIST:
			if (results->tree_data->columns)
				g_slist_foreach (results->tree_data->columns, (GFunc) g_free, NULL);
                        if (results->tree_data->separator)
                                g_free (results->tree_data->separator);
			break;
		default:
			break;
	}
}

gint 
main (gint argc, gchar **argv) {
	ZenityData *general;
	ZenityCalendarData *cal_data;
	poptContext ctx;
	gchar **args;
	gint nextopt, retval;

	bindtextdomain(GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

	zenity_init_parsing_options ();

	/* FIXME: popt doesn't like passing stuff through data 
         * but it doesn't seem to cope with the data that I try
         * to pass in, not quite sure why though. If someone knows
         * what I'm doing wrong, we could probably put this back:
	 * options[0].descrip = (void*) results; 
         */

	ctx = poptGetContext ("zenity", argc, (const char **)argv, application_options, 0);

	poptReadDefaultConfig(ctx, TRUE);
	while((nextopt = poptGetNextOpt(ctx)) > 0)
		/*nothing*/;

	if (nextopt != -1) {
                /* FIXME : We should probably handle --display, or at least maybe load some of the gtk+
                 * commandline options
                 */
		g_printerr (_("%s in an invalid option for this dialog\n"), poptBadOption (ctx, 0));
		zenity_free_parsing_options ();
		exit (-1);
	}	

	gtk_init (&argc, &argv);
	
	if (argc < 2) {
		zenity_free_parsing_options ();
		exit (-1);
	}

	switch (results->mode) {
		case MODE_CALENDAR:
			zenity_calendar (results->data, results->calendar_data);
			break;
		case MODE_ENTRY:
			zenity_entry (results->data, results->entry_data);
			break;
		case MODE_ERROR:
		case MODE_QUESTION:
		case MODE_WARNING:
		case MODE_INFO:
			zenity_msg (results->data, results->msg_data);
			break;
		case MODE_FILE:
			zenity_fileselection (results->data, results->file_data);
			break;
		case MODE_LIST:
			results->tree_data->data = poptGetArgs (ctx);
			zenity_tree (results->data, results->tree_data);
			break;
		case MODE_PROGRESS:
			zenity_progress (results->data, results->progress_data);
			break;
		case MODE_TEXTINFO:
			zenity_text (results->data, results->text_data);
			break;
                case MODE_ABOUT:
                        zenity_about (results->data);
                        break;
		default:
			g_assert_not_reached ();
			zenity_free_parsing_options ();
			exit (-1);
	}

	retval = results->data->exit_code;
	poptFreeContext(ctx);
	zenity_free_parsing_options ();
	exit (retval);
}

static void
zenity_error (gchar *string, ZenityError error)
{
        switch (error) {
                case ERROR_DUPLICATE:
			g_printerr (_("%s given twice for the same dialog\n"), string);
			zenity_free_parsing_options ();
			exit (-1);
                case ERROR_SUPPORT:
			g_printerr (_("%s is not supported for this dialog\n"), string);
			zenity_free_parsing_options ();
			exit (-1);
                case ERROR_DIALOG:
			g_printerr (_("Two or more dialog options specified\n"));
			zenity_free_parsing_options ();
			exit (-1);
                default:
                        return;
        }
}

static void 
zenity_parse_options_callback (poptContext              ctx,
                               enum poptCallbackReason  reason,
                               const struct poptOption *opt,
                               const char              *arg,
                               void                    *data)
{
        static gboolean parse_option_dateformat = FALSE;
        static gboolean parse_option_separator = FALSE;
        static gint parse_option_text = 0;
        static gint parse_option_file = 0;

	if (reason == POPT_CALLBACK_REASON_POST) {
		return;
	} 
	else if (reason != POPT_CALLBACK_REASON_OPTION)
		return;

	switch (opt->val & POPT_ARG_MASK) {

 		case OPTION_CALENDAR:
			if (results->mode != MODE_LAST)
                                zenity_error (NULL, ERROR_DIALOG);

			results->mode = MODE_CALENDAR;
			break;
		case OPTION_ENTRY:
			if (results->mode != MODE_LAST)
                                zenity_error (NULL, ERROR_DIALOG);

			results->mode = MODE_ENTRY;
			break;
		case OPTION_ERROR:
			if (results->mode != MODE_LAST)
                                zenity_error (NULL, ERROR_DIALOG);

			results->mode = MODE_ERROR;
			results->msg_data->mode = ZENITY_MSG_ERROR;
			break;
		case OPTION_INFO:
			if (results->mode != MODE_LAST)
                                zenity_error (NULL, ERROR_DIALOG);

			results->mode = MODE_INFO;
			results->msg_data->mode = ZENITY_MSG_INFO;
			break;
		case OPTION_FILE:
			if (results->mode != MODE_LAST)
                                zenity_error (NULL, ERROR_DIALOG);

			results->mode = MODE_FILE;
			break;
		case OPTION_LIST:
			if (results->mode != MODE_LAST)
                                zenity_error (NULL, ERROR_DIALOG);

			results->mode = MODE_LIST;
			break;
		case OPTION_PROGRESS:
			if (results->mode != MODE_LAST)
                                zenity_error (NULL, ERROR_DIALOG);

			results->mode = MODE_PROGRESS;
			break;
		case OPTION_QUESTION:
			if (results->mode != MODE_LAST)
                                zenity_error (NULL, ERROR_DIALOG);

			results->mode = MODE_QUESTION;
			results->msg_data->mode = ZENITY_MSG_QUESTION;
			break;
		case OPTION_TEXTINFO:
			if (results->mode != MODE_LAST)
                                zenity_error (NULL, ERROR_DIALOG);

			results->mode = MODE_TEXTINFO;
			break;
		case OPTION_WARNING:
			if (results->mode != MODE_LAST)
                                zenity_error (NULL, ERROR_DIALOG);

			results->mode = MODE_WARNING;
			results->msg_data->mode = ZENITY_MSG_WARNING;
			break;
		case OPTION_TITLE:
			if (results->data->dialog_title != NULL)
                                zenity_error ("--title", ERROR_DUPLICATE);

			results->data->dialog_title = g_strdup (arg);
			break;
		case OPTION_ICON:
			if (results->data->window_icon != NULL)
                                zenity_error ("--window-icon", ERROR_DUPLICATE);

			results->data->window_icon = g_strdup (arg);
			break;
		case OPTION_CALENDARTEXT:
		case OPTION_ENTRYTEXT:
		case OPTION_ERRORTEXT:
		case OPTION_QUESTIONTEXT:
		case OPTION_PROGRESSTEXT:
		case OPTION_WARNINGTEXT:

                        /* FIXME: This is an ugly hack because of the way the poptOptions are
                         * ordered above. When you try and use an --option more than once
                         * parse_options_callback gets called for each option. Suckage
                         */

                        if (parse_option_text > 6)
                                zenity_error ("--text", ERROR_DUPLICATE);

			switch (results->mode) {
				case MODE_CALENDAR:
					results->calendar_data->dialog_text = g_strdup (arg);
					break;
				case MODE_ENTRY:
					results->entry_data->dialog_text = g_strdup (arg);
					break;
				case MODE_ERROR:
				case MODE_QUESTION:
				case MODE_WARNING:
				case MODE_INFO:
					results->msg_data->dialog_text = g_strdup (arg);
					break;
				case MODE_PROGRESS:
					results->progress_data->dialog_text = g_strdup (arg);
					break;
				default:
                                        zenity_error ("--text", ERROR_SUPPORT);
				}
                        parse_option_text++;
			break;
		case OPTION_DAY:
			if (results->mode != MODE_CALENDAR)
                                zenity_error ("--day", ERROR_SUPPORT);

			if (results->calendar_data->day > 0)
                                zenity_error ("--day", ERROR_DUPLICATE);

			results->calendar_data->day = atoi (arg);
			break;
		case OPTION_MONTH:
			if (results->mode != MODE_CALENDAR)
                                zenity_error ("--month", ERROR_SUPPORT);

			if (results->calendar_data->month > 0)
                                zenity_error ("--day", ERROR_DUPLICATE);

			results->calendar_data->month = atoi (arg);
			break;
		case OPTION_YEAR:
			if (results->mode != MODE_CALENDAR)
                                zenity_error ("--year", ERROR_SUPPORT);
			
                        if (results->calendar_data->year > 0)
                                zenity_error ("--year", ERROR_DUPLICATE);

			results->calendar_data->year = atoi (arg);
			break;
		case OPTION_DATEFORMAT:
			if (results->mode != MODE_CALENDAR)
                                zenity_error ("--date-format", ERROR_SUPPORT);
                        
                        if (parse_option_dateformat == TRUE)
                                zenity_error ("--date-format", ERROR_DUPLICATE);

			results->calendar_data->date_format = g_strdup (arg);
                        parse_option_dateformat = TRUE;
			break;
		case OPTION_INPUTTEXT:
			if (results->mode != MODE_ENTRY)
                                zenity_error ("--entry-text", ERROR_SUPPORT);
			
                        if (results->entry_data->entry_text != NULL)
                                zenity_error ("--entry-text", ERROR_DUPLICATE);

			results->entry_data->entry_text = g_strdup (arg);
			break;
		case OPTION_HIDETEXT:
			if (results->mode != MODE_ENTRY)
                                zenity_error ("--hide-text", ERROR_SUPPORT);
			
                        if (results->entry_data->visible == FALSE)
                                zenity_error ("--hide-text", ERROR_DUPLICATE);

			results->entry_data->visible = FALSE;
			break;
		case OPTION_TEXTEDIT:
			if (results->mode != MODE_TEXTINFO)
                                zenity_error ("--editable", ERROR_SUPPORT);

			if (results->text_data->editable == TRUE)
                                zenity_error ("--editable", ERROR_DUPLICATE);

			results->text_data->editable = TRUE;
			break;
		case OPTION_FILENAME:
		case OPTION_TEXTFILE:

                        /* FIXME: This is an ugly hack because of the way the poptOptions are
                         * ordered above. When you try and use an --option more than once
                         * parse_options_callback gets called for each option. Suckage
                         */

                        if (parse_option_file > 2)
                                zenity_error ("--filename", ERROR_DUPLICATE);

			switch (results->mode) {
				case MODE_FILE:
					results->file_data->uri = g_strdup (arg);
					break;
				case MODE_TEXTINFO:
					results->text_data->uri = g_strdup (arg);
					break;
				default:
                                        zenity_error ("--filename", ERROR_SUPPORT);
			}
                        parse_option_file++;
			break;
		case OPTION_COLUMN:
			if (results->mode != MODE_LIST)
                                zenity_error ("--column", ERROR_SUPPORT);

			results->tree_data->columns = g_slist_append (results->tree_data->columns, g_strdup (arg));
			break;
		case OPTION_CHECKLIST:
			if (results->mode != MODE_LIST)
                                zenity_error ("--checkbox", ERROR_SUPPORT);
			
                        if (results->tree_data->checkbox == TRUE)
                                zenity_error ("--checkbox", ERROR_DUPLICATE);

			results->tree_data->checkbox = TRUE;
			break;
		case OPTION_RADIOLIST:
			if (results->mode != MODE_LIST)
                                zenity_error ("--radiobox", ERROR_SUPPORT);
			
                        if (results->tree_data->radiobox == TRUE)
                                zenity_error ("--radiobox", ERROR_DUPLICATE);

			results->tree_data->radiobox = TRUE;
			break;
                case OPTION_SEPERATOR:
                        if (results->mode != MODE_LIST)
                                zenity_error ("--separator", ERROR_SUPPORT);
                        
                        if (parse_option_separator == TRUE)
                                zenity_error ("--separator", ERROR_DUPLICATE);

			results->tree_data->separator = g_strdup (arg);
                        parse_option_separator = TRUE;
			break;
		case OPTION_PERCENTAGE:
			if (results->mode != MODE_PROGRESS)
                                zenity_error ("--percentage", ERROR_SUPPORT);
			
                        if (results->progress_data->percentage > -1)
                                zenity_error ("--percentage", ERROR_DUPLICATE);

			results->progress_data->percentage = atoi (arg);
			break;
		case OPTION_PULSATE:
			if (results->mode != MODE_PROGRESS)
                                zenity_error ("--pulsate", ERROR_SUPPORT);

			results->progress_data->pulsate = TRUE;
			break;
		case OPTION_ABOUT:
			if (results->mode != MODE_LAST)
                                zenity_error (NULL, ERROR_DIALOG);

                        results->mode = MODE_ABOUT; 
			break;
		case OPTION_VERSION:
			if (results->mode != MODE_LAST)
                                zenity_error (NULL, ERROR_DIALOG);

			g_print ("%s\n", VERSION);
			exit (0);
			break;
		default:
                        break;
	}
}
