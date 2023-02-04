#ifndef ZENITY_H
#define ZENITY_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#include <libintl.h>
#define _(String) dgettext (GETTEXT_PACKAGE, String)
#ifdef gettext_noop
#define N_(String) gettext_noop (String)
#else
#define N_(String) (String)
#endif

typedef struct {
	gchar *dialog_title;
	gchar *window_icon;
	gchar *ok_label;
	gchar *cancel_label;
	gchar **extra_label;
	gint width;
	gint height;
	gint exit_code;
	gint timeout_delay;
	gboolean modal;
	guintptr attach;
} ZenityData;

typedef enum {
	ZENITY_OK,
	ZENITY_CANCEL,
	ZENITY_ESC,
	ZENITY_ERROR,
	ZENITY_EXTRA,
	/* Previously, this was not specified to any value, which could cause it to
	 * clash with the custom response ID that happened to match it. We could set
	 * this to a negative value tha doesn't clash with one of GtkDialog's
	 * predefined response ID's as it's doubtful GTK will ever extend the GtkDialog
	 * API at this stage -- but technically negative values are reserved for the
	 * library and positive values for applications, according to gtkdialog.c
	 */
	ZENITY_TIMEOUT = INT_MAX
} ZenityExitCode;

typedef struct {
	gchar *dialog_text;
	gint day;
	gint month;
	gint year;
	gchar *date_format;
} ZenityCalendarData;

typedef enum {
	ZENITY_MSG_WARNING,
	ZENITY_MSG_QUESTION,
	ZENITY_MSG_SWITCH,
	ZENITY_MSG_ERROR,
	ZENITY_MSG_INFO
} MsgMode;

typedef struct {
	gchar *dialog_text;
	gchar *dialog_icon;
	MsgMode mode;
	gboolean no_wrap;
	gboolean no_markup;
	gboolean default_cancel;
	gboolean ellipsize;
} ZenityMsgData;

typedef struct {
	gchar *dialog_text;
	gint value;
	gint min_value;
	gint max_value;
	gint step;
	gboolean print_partial;
	gboolean hide_value;
} ZenityScaleData;

typedef struct {
	gchar *uri;
	gboolean multi;
	gboolean directory;
	gboolean save;
	gboolean confirm_overwrite;
	gchar *separator;
	gchar **filter;
} ZenityFileData;

typedef struct {
	gchar *dialog_text;
	gchar *entry_text;
	gboolean hide_text;
	const gchar **data;
} ZenityEntryData;

typedef struct {
	gchar *dialog_text;
	gchar *entry_text;
	gboolean pulsate;
	gboolean autoclose;
	gboolean autokill;
	gdouble percentage;
	gboolean no_cancel;
	gboolean time_remaining;
} ZenityProgressData;

typedef struct {
	gchar *uri;
	gboolean editable;
	gboolean no_wrap;
	gboolean auto_scroll;
	gchar *font;
	GtkTextBuffer *buffer;
	gchar *checkbox;
#ifdef HAVE_WEBKITGTK
	gboolean html;
	gboolean no_interaction;
	gchar *url;
#endif
} ZenityTextData;

typedef struct {
	gchar *dialog_text;
	GSList *columns;
	gboolean checkbox;
	gboolean radiobox;
	gboolean hide_header;
	gboolean imagebox;
	gchar *separator;
	gboolean multi;
	gboolean editable;
	gboolean mid_search;
	gchar *print_column;
	gchar *hide_column;
	const gchar **data;
} ZenityTreeData;

#ifdef HAVE_LIBNOTIFY
typedef struct {
	gchar *notification_text;
	gboolean listen;
	gchar **notification_hints;
} ZenityNotificationData;
#endif

typedef struct {
	gchar *color;
	gboolean show_palette;
} ZenityColorData;

typedef struct {
	GSList *list;
	GSList *list_widgets;
	GSList *list_values;
	GSList *column_values;
	GSList *combo_values;
	gchar *dialog_text;
	gchar *separator;
	gchar *date_format;
	//  gchar *hide_column;
	gboolean show_header;
} ZenityFormsData;

typedef enum {
	ZENITY_FORMS_ENTRY,
	ZENITY_FORMS_PASSWORD,
	ZENITY_FORMS_CALENDAR,
	ZENITY_FORMS_LIST,
	ZENITY_FORMS_COMBO
} ZenityFormsType;

typedef struct {
	gchar *option_value;
	ZenityFormsType type;
	GtkWidget *forms_widget;
} ZenityFormsValue;

typedef struct {
	gboolean username;
	gchar *password;
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
#ifdef HAVE_LIBNOTIFY
void zenity_notification (
	ZenityData *data, ZenityNotificationData *notification_data);
#endif

void zenity_colorselection (
	ZenityData *data, ZenityColorData *notification_data);
void zenity_scale (ZenityData *data, ZenityScaleData *scale_data);
void zenity_about (ZenityData *data);

void zenity_password_dialog (
	ZenityData *data, ZenityPasswordData *password_data);
void zenity_forms_dialog (ZenityData *data, ZenityFormsData *forms_data);
G_END_DECLS

#endif /* ZENITY_H */
