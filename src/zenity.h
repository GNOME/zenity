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
	gint   exit_code;
} ZenityData;

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
	gchar	*dialog_text;
	MsgMode  mode;
} ZenityMsgData;

typedef struct {
	gchar	*uri;
} ZenityFileData;

typedef struct {
	gchar		*dialog_text;
	gchar		*entry_text;
	gboolean	 visible;
} ZenityEntryData;

typedef struct {
	gchar		*dialog_text;
	gchar		*entry_text;
	gboolean	 pulsate;
	gdouble		 percentage;
} ZenityProgressData;

typedef struct {
	gchar		*uri;
	gboolean	editable;
	GtkTextBuffer	*buffer;
} ZenityTextData;

typedef struct {
	gchar		*dialog_text;
	GSList		*columns;
	gboolean 	 checkbox;
	gboolean	 radiobox;
        gchar           *separator;
	const gchar    **data;
} ZenityTreeData;

void	zenity_calendar		(ZenityData		*data,
				 ZenityCalendarData 	*calendar_data);
void	zenity_msg		(ZenityData		*data,
				 ZenityMsgData		*msg_data);
void	zenity_fileselection	(ZenityData		*data,
				 ZenityFileData         *file_data);
void	zenity_entry		(ZenityData		*data,
				 ZenityEntryData	*entry_data);
void	zenity_progress		(ZenityData		*data,
				 ZenityProgressData	*progress_data);
void	zenity_text		(ZenityData		*data,
				 ZenityTextData         *text_data);
void	zenity_tree		(ZenityData		*data,
				 ZenityTreeData         *tree_data);
void    zenity_about            (ZenityData             *data);

G_END_DECLS

#endif /* ZENITY_H */
