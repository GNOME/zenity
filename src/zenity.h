/* vim: colorcolumn=80 ts=4 sw=4
 */
#pragma once

#include <adwaita.h>
#include <glib/gi18n.h>

#include <config.h>

G_BEGIN_DECLS

typedef enum
{
	ZENITY_OK 		= -1,
	ZENITY_CANCEL	= -2,
	ZENITY_ESC		= -3,
	ZENITY_ERROR	= -4,
	ZENITY_EXTRA	= -5,
	ZENITY_TIMEOUT	= -6
} ZenityExitCode;

typedef struct {
	char *dialog_title;
	char *ok_label;
	char *cancel_label;
	char **extra_label;
	int width;
	int height;
	int exit_code;
	int timeout_delay;
	gboolean modal;
} ZenityData;

typedef struct {
	char *dialog_text;
	int day;
	int month;
	int year;
	char *date_format;
} ZenityCalendarData;

typedef enum {
	ZENITY_MSG_WARNING,
	ZENITY_MSG_QUESTION,
	ZENITY_MSG_SWITCH,
	ZENITY_MSG_ERROR,
	ZENITY_MSG_INFO
} MsgMode;

typedef struct {
	char *dialog_text;
	char *dialog_icon;
	MsgMode mode;
	gboolean no_wrap;
	gboolean no_markup;
	gboolean default_cancel;
	gboolean ellipsize;
} ZenityMsgData;

typedef struct {
	char *dialog_text;
	int value;
	int min_value;
	int max_value;
	int step;
	gboolean print_partial;
	gboolean hide_value;
} ZenityScaleData;

typedef struct {
	char *uri;
	gboolean multi;
	gboolean directory;
	gboolean save;
	char *separator;
	char **filter;
} ZenityFileData;

typedef struct {
	char *dialog_text;
	char *entry_text;
	gboolean hide_text;
	char **data;
} ZenityEntryData;

typedef struct {
	char *dialog_text;
	char *entry_text;
	gboolean pulsate;
	gboolean autoclose;
	gboolean autokill;
	gdouble percentage;
	gboolean no_cancel;
	gboolean time_remaining;
} ZenityProgressData;

typedef struct {
	char *uri;
	gboolean editable;
	gboolean no_wrap;
	gboolean auto_scroll;
	char *font;
	GtkTextBuffer *buffer;
	char *checkbox;
#ifdef HAVE_WEBKITGTK
	gboolean html;
	gboolean no_interaction;
	char *url;
#endif
} ZenityTextData;

typedef struct {
	char *dialog_text;
	GSList *columns;
	gboolean checkbox;
	gboolean radiobox;
	gboolean hide_header;
	gboolean imagebox;
	char *separator;
	gboolean multi;
	gboolean editable;
	gboolean mid_search_DEPRECATED;
	char *print_column;
	char *hide_column;
	char **data;
} ZenityTreeData;

typedef struct {
	char *notification_text;
	gboolean listen;
	char *icon;
} ZenityNotificationData;

typedef struct {
	char *color;
	gboolean show_palette;
} ZenityColorData;

typedef struct {
	GSList *list;
	GSList *list_widgets;
	GSList *list_values;
	GSList *column_values;
	GSList *combo_values;
	GSList *text_info_values;
	char *dialog_text;
	char *separator;
	char *date_format;
	gboolean show_header;
} ZenityFormsData;

typedef enum {
	ZENITY_FORMS_ENTRY,
	ZENITY_FORMS_PASSWORD,
	ZENITY_FORMS_CALENDAR,
	ZENITY_FORMS_LIST,
	ZENITY_FORMS_COMBO,
	ZENITY_FORMS_MULTLINE_ENTRY,
} ZenityFormsType;

typedef struct {
	char *option_value;
	ZenityFormsType type;
	GtkWidget *forms_widget;
} ZenityFormsValue;

typedef struct {
	gboolean username;
	char *password;
	GtkWidget *entry_username;
	GtkWidget *entry_password;
} ZenityPasswordData;

void zenity_calendar (ZenityData *data, ZenityCalendarData *calendar_data);
void zenity_msg (ZenityData *data, ZenityMsgData *msg_data);
void zenity_fileselection (ZenityData *data, ZenityFileData *file_data);
void zenity_entry (ZenityData *data, ZenityEntryData *entry_data);
void zenity_progress (ZenityData *data, ZenityProgressData *progress_data);
void zenity_text (ZenityData *data, ZenityTextData *text_data);
void zenity_tree (ZenityData *data, ZenityTreeData *tree_data);
void zenity_notification (ZenityData *data,
		ZenityNotificationData *notification_data);

void zenity_colorselection (
	ZenityData *data, ZenityColorData *notification_data);
void zenity_scale (ZenityData *data, ZenityScaleData *scale_data);
void zenity_about (ZenityData *data);

void zenity_password_dialog (
	ZenityData *data, ZenityPasswordData *password_data);
void zenity_forms_dialog (ZenityData *data, ZenityFormsData *forms_data);
G_END_DECLS
