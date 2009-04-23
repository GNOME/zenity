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
  gchar   *ok_label;
  gchar   *cancel_label;
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
} ZenityProgressData;

typedef struct {
  gchar         *uri;
  gboolean       editable;
  GtkTextBuffer	*buffer;
} ZenityTextData;

typedef struct {
  gchar        *dialog_text;
  GSList       *columns;
  gboolean      checkbox;
  gboolean      radiobox;
  gchar        *separator;
  gboolean      multi;
  gboolean      editable;
  gchar	       *print_column;
  gchar	       *hide_column;
  const gchar **data;
} ZenityTreeData;

typedef struct {
  gchar   *notification_text;
  gboolean listen;
} ZenityNotificationData;

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
void	zenity_notification	(ZenityData		*data,
				 ZenityNotificationData	*notification_data);
void	zenity_scale		(ZenityData		*data,
				 ZenityScaleData	*scale_data);
void    zenity_about            (ZenityData             *data);

G_END_DECLS

#endif /* ZENITY_H */
