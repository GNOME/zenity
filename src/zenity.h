#ifndef ZENITY_H
#define ZENITY_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(String) dgettext(GETTEXT_PACKAGE,String)
#ifdef gettext_noop
#define N_(String) gettext_noop(String)
#else
#define N_(String) (String)
#endif
#else /* NLS is disabled */
#define _(String) (String)
#define N_(String) (String)
#define textdomain(String) (String)
#define gettext(String) (String)
#define dgettext(Domain,String) (String)
#define dcgettext(Domain,String,Type) (String)
#define bindtextdomain(Domain,Directory) (Domain) 
#endif

typedef struct {
  gchar *dialog_title;
  gchar *window_icon;
  gchar *ok_label;
  gchar *cancel_label;
  gint   width;
  gint   height;
  gint   exit_code;
  gint   timeout_delay;
} ZenityData;

typedef enum {
  ZENITY_OK,
  ZENITY_CANCEL,
  ZENITY_ESC,
  ZENITY_ERROR,
  ZENITY_EXTRA,
  ZENITY_TIMEOUT
} ZenityExitCode;

typedef struct {
  gchar *dialog_text;
  gint   day;
  gint   month;
  gint   year;
  gchar *date_format;
} ZenityCalendarData;

typedef enum {
  ZENITY_MSG_WARNING,
  ZENITY_MSG_QUESTION,
  ZENITY_MSG_ERROR,
  ZENITY_MSG_INFO
} MsgMode;

typedef struct {
  gchar   *dialog_text;
  MsgMode  mode;
  gboolean no_wrap;
  gboolean no_markup;
} ZenityMsgData;

typedef struct {
  gchar   *dialog_text;
  gint     value;
  gint     min_value;
  gint     max_value;
  gint     step;
  gboolean print_partial;
  gboolean hide_value;
} ZenityScaleData;

typedef struct {
  gchar	  *uri;
  gboolean multi;
  gboolean directory;
  gboolean save;
  gboolean confirm_overwrite;
  gchar   *separator;
  gchar  **filter;
} ZenityFileData;

typedef struct {
  gchar   *dialog_text;
  gchar   *entry_text;
  gboolean hide_text;
  const gchar **data;
} ZenityEntryData;

typedef struct {
  gchar   *dialog_text;
  gchar   *entry_text;
  gboolean pulsate;
  gboolean autoclose;
  gboolean autokill;
  gdouble  percentage;
  gboolean no_cancel;
} ZenityProgressData;

typedef struct {
  gchar         *uri;
  gboolean       editable;
  gboolean       no_wrap;
  gchar         *font;
  GtkTextBuffer	*buffer;
  gchar         *checkbox;
#ifdef HAVE_WEBKITGTK
  gboolean       html;
  gchar         *url;
#endif
} ZenityTextData;

typedef struct {
  gchar        *dialog_text;
  GSList       *columns;
  gboolean      checkbox;
  gboolean      radiobox;
  gboolean      hide_header;
  gchar        *separator;
  gboolean      multi;
  gboolean      editable;
  gchar	       *print_column;
  gchar	       *hide_column;
  const gchar **data;
} ZenityTreeData;

#ifdef HAVE_LIBNOTIFY
typedef struct {
  gchar   *notification_text;
  gboolean listen;
} ZenityNotificationData;
#endif

typedef struct {
  gchar   *color;
  gboolean show_palette;
} ZenityColorData;

typedef struct {
  GSList *list;
  GSList *list_widgets;
  GSList *list_values;
  GSList *column_values;
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
  ZENITY_FORMS_LIST
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

void    zenity_calendar         (ZenityData             *data,
                                ZenityCalendarData      *calendar_data);
void    zenity_msg              (ZenityData             *data,
                                 ZenityMsgData          *msg_data);
void    zenity_fileselection    (ZenityData             *data,
                                 ZenityFileData         *file_data);
void    zenity_entry            (ZenityData             *data,
                                 ZenityEntryData        *entry_data);
void    zenity_progress	        (ZenityData             *data,
                                 ZenityProgressData     *progress_data);
void    zenity_text             (ZenityData             *data,
                                 ZenityTextData         *text_data);
void    zenity_tree             (ZenityData             *data,
                                 ZenityTreeData         *tree_data);
#ifdef HAVE_LIBNOTIFY
void	zenity_notification	(ZenityData		*data,
				 ZenityNotificationData	*notification_data);
#endif

void    zenity_colorselection   (ZenityData             *data,
                                 ZenityColorData        *notification_data);
void	zenity_scale		(ZenityData		*data,
				 ZenityScaleData	*scale_data);
void    zenity_about            (ZenityData             *data);

void    zenity_password_dialog  (ZenityData             *data,
                                 ZenityPasswordData     *password_data);
void    zenity_forms_dialog     (ZenityData             *data,
                                 ZenityFormsData        *forms_data);
G_END_DECLS

#endif /* ZENITY_H */
