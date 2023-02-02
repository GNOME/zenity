#include "zenity-tree-column-view.h"
#include <config.h>

#define UI_FILE RESOURCE_BASE_PATH "/zenity-tree-column-view.ui"

/* ZenityTreeItem */

enum zenity_tree_item_prop_enum
{
	TEXT = 1,
	CHILD,
	N_PROPERTIES_ZENITY_TREE_ITEM
};

static GParamSpec *zenity_tree_item_properties[N_PROPERTIES_ZENITY_TREE_ITEM];

struct _ZenityTreeItem
{
	GObject parent_instance;
	GtkWidget *child;
	char *text;
};

G_DEFINE_TYPE (ZenityTreeItem, zenity_tree_item, G_TYPE_OBJECT)

static void
zenity_tree_item_set_property (GObject *object,
		guint property_id,
		const GValue *value,
		GParamSpec *pspec)
{
	ZenityTreeItem *self = ZENITY_TREE_ITEM(object);

	switch (property_id)
	{
		case TEXT:
			zenity_tree_item_set_text (self, g_value_get_string (value));
			break;

		case CHILD:
			zenity_tree_item_set_child (self, GTK_WIDGET(g_value_get_object (value)));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}

static void
zenity_tree_item_get_property (GObject *object,
		guint property_id,
		GValue *value,
		GParamSpec *pspec)
{
	ZenityTreeItem *self = ZENITY_TREE_ITEM(object);

	switch (property_id)
	{
		case TEXT:
			g_value_set_string (value, zenity_tree_item_get_text (self));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}
static void
zenity_tree_item_init (ZenityTreeItem *item)
{
}

static void
zenity_tree_item_dispose (GObject *object)
{
	ZenityTreeItem *self = ZENITY_TREE_ITEM(object);

	if (g_object_is_floating (self->child))
		g_warning ("%s: trying to dispose ZenityTreeItem before the widget has been parented. "
				"Likely a programmer error. A leak will likely result.", __func__);
	else
		g_clear_object (&self->child);

	G_OBJECT_CLASS(zenity_tree_item_parent_class)->dispose (object);
}

static void
zenity_tree_item_finalize (GObject *object)
{
	ZenityTreeItem *self = ZENITY_TREE_ITEM(object);

	g_free (self->text);

	G_OBJECT_CLASS(zenity_tree_item_parent_class)->finalize (object);
}

static void
zenity_tree_item_class_init (ZenityTreeItemClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GParamFlags flags = G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT;

	object_class->dispose = zenity_tree_item_dispose;
	object_class->finalize = zenity_tree_item_finalize;
	object_class->set_property = zenity_tree_item_set_property;
	object_class->get_property = zenity_tree_item_get_property;

	zenity_tree_item_properties[TEXT] = g_param_spec_string ("text", NULL, NULL, NULL, flags);
	zenity_tree_item_properties[CHILD] = g_param_spec_object ("child", NULL, NULL, GTK_TYPE_WIDGET, flags);

	g_object_class_install_properties (object_class, N_PROPERTIES_ZENITY_TREE_ITEM, zenity_tree_item_properties);
}

GtkWidget *
zenity_tree_item_get_child (ZenityTreeItem *item)
{
	return item->child;
}

const char *
zenity_tree_item_get_text (ZenityTreeItem *item)
{
	return item->text;
}

void
zenity_tree_item_set_text (ZenityTreeItem *self, const char *text)
{
	g_clear_pointer (&self->text, g_free);
	self->text = g_strdup (text);

	g_object_notify_by_pspec (G_OBJECT(self), zenity_tree_item_properties[TEXT]);
}

void
zenity_tree_item_set_child (ZenityTreeItem *self, GtkWidget *child)
{
	if (self->child)
		g_object_unref (self->child);

	self->child = g_object_ref (child);

	g_object_notify_by_pspec (G_OBJECT(self), zenity_tree_item_properties[CHILD]);
}

ZenityTreeItem *
zenity_tree_item_new (const char *text, GtkWidget *child)
{
	ZenityTreeItem *item;

	g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);

	item = g_object_new (ZENITY_TREE_TYPE_ITEM,
			"text", text,
			"child", child,
			NULL);

	return item;
}

/* ZenityTreeRow */

struct _ZenityTreeRow
{
	GObject parent_instance;
	GPtrArray *items;
};

G_DEFINE_TYPE (ZenityTreeRow, zenity_tree_row, G_TYPE_OBJECT)

static void
zenity_tree_row_init (ZenityTreeRow *self)
{
	self->items = g_ptr_array_new ();
}

static void
zenity_tree_row_class_init (ZenityTreeRowClass *klass)
{
}

ZenityTreeRow *
zenity_tree_row_new (void)
{
	return g_object_new (ZENITY_TREE_TYPE_ROW, NULL);
}

void
zenity_tree_row_add (ZenityTreeRow *row, ZenityTreeItem *item)
{
	g_ptr_array_add (row->items, item);
}

guint
zenity_tree_row_get_n_items (ZenityTreeRow *row)
{
	return row->items->len;
}

ZenityTreeItem *
zenity_tree_row_get_item (ZenityTreeRow *row, guint index)
{
	g_return_val_if_fail (index < row->items->len, NULL);

	return row->items->pdata[index];
}

/* ZenityTreeColumnView */

enum zenity_tree_column_view_signal_enum {
	ACTIVATED,
	LAST_SIGNAL
};

static guint zenity_tree_column_view_signals[LAST_SIGNAL];

enum zenity_tree_column_view_prop_enum
{
	MULTI = 1,
	LIST_TYPE,
	MODEL,
	N_PROPERTIES_ZENITY_TREE_COLUMN_VIEW
};

static GParamSpec *zenity_tree_column_view_properties[N_PROPERTIES_ZENITY_TREE_COLUMN_VIEW];

struct _ZenityTreeColumnView
{
	GtkWidget parent_instance;
	GtkWidget *scrolled_window;
	GtkColumnView *child_cv;
	gboolean multi;
	ZenityTreeListType list_type;
	GListModel *model;
	GtkStringFilter *filter;
	GtkCheckButton *checkbutton_group;
};

G_DEFINE_TYPE (ZenityTreeColumnView, zenity_tree_column_view, GTK_TYPE_WIDGET)

/* Callback for the GClosure defined below. Just cram the text from all row
 * items into a single string; that way, if the string in the searchbar matches
 * any text in a given row, there will be a match, and only those rows will be
 * shown.
 */
static char *
eval_str (ZenityTreeRow *row)
{
	GString *gstring;

	g_return_val_if_fail (ZENITY_TREE_IS_ROW (row), NULL);

	gstring = g_string_new (NULL);

	for (guint i = 0; i < row->items->len; ++i)
	{
		ZenityTreeItem *item = row->items->pdata[i];
		g_string_append_printf (gstring, "%s ", item->text);
	}

	return g_string_free (gstring, FALSE);
}

void
zenity_tree_column_view_set_model (ZenityTreeColumnView *self, GListModel *model)
{
	GtkStringFilter *filter;
	GtkFilterListModel *filter_model;

	GtkExpression *expr;

	/* This tells the column view to use a callback with 'this' (ZenityTreeRow)
	 * as the instance and no other params or user_data, with a string retval.
	 */
	expr = gtk_cclosure_expression_new (G_TYPE_STRING,
			NULL,					/* GClosureMarshal marshal, */
			0, 						/* guint n_params, */
			NULL,					/* GtkExpression** params, */
			G_CALLBACK(eval_str),	/* GCallback callback_func, */
			NULL,					/* gpointer user_data, */
			NULL);					/* GClosureNotify user_destroy) */

	filter = gtk_string_filter_new (expr);
	filter_model = gtk_filter_list_model_new (model, GTK_FILTER(filter));

	self->model = model;
	self->filter = filter;

	if (self->multi)
	{
		gtk_column_view_set_model (self->child_cv,
				GTK_SELECTION_MODEL(gtk_multi_selection_new (G_LIST_MODEL(filter_model))));
	}
	else
	{
		gtk_column_view_set_model (self->child_cv,
				GTK_SELECTION_MODEL(gtk_single_selection_new (G_LIST_MODEL(filter_model))));
	}

	g_object_notify_by_pspec (G_OBJECT(self), zenity_tree_column_view_properties[MODEL]);
}

GListModel *
zenity_tree_column_view_get_model (ZenityTreeColumnView *self)
{
	return self->model;
}

static void
zenity_tree_column_view_emit_activated (ZenityTreeColumnView *self)
{
	g_signal_emit (self, zenity_tree_column_view_signals[ACTIVATED], 0);
}

void
zenity_tree_column_view_set_list_type (ZenityTreeColumnView *self, ZenityTreeListType type)
{
	self->list_type = type;

	g_signal_handlers_disconnect_by_func (self->child_cv, zenity_tree_column_view_emit_activated, self);

	switch (self->list_type)
	{
		case ZENITY_TREE_LIST_NONE:
		case ZENITY_TREE_LIST_IMAGE:
			g_signal_connect_swapped (self->child_cv, "activate", G_CALLBACK(zenity_tree_column_view_emit_activated), self);
			break;

		default:
			break;
	}

	g_object_notify_by_pspec (G_OBJECT(self), zenity_tree_column_view_properties[LIST_TYPE]);
}

ZenityTreeListType
zenity_tree_column_view_get_list_type (ZenityTreeColumnView *self)
{
	return self->list_type;
}

gboolean
zenity_tree_column_view_get_multi (ZenityTreeColumnView *self)
{
	return self->multi;
}

static void
zenity_tree_column_view_set_property (GObject *object,
		guint property_id,
		const GValue *value,
		GParamSpec *pspec)
{
	ZenityTreeColumnView *self = ZENITY_TREE_COLUMN_VIEW(object);

	switch (property_id)
	{
		case MULTI:
			self->multi = g_value_get_boolean (value);
			break;

		case LIST_TYPE:
			zenity_tree_column_view_set_list_type (self, g_value_get_enum (value));
			break;

		case MODEL:
			zenity_tree_column_view_set_model (self, G_LIST_MODEL(g_value_get_object (value)));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}

static void
zenity_tree_column_view_get_property (GObject *object,
		guint property_id,
		GValue *value,
		GParamSpec *pspec)
{
	ZenityTreeColumnView *self = ZENITY_TREE_COLUMN_VIEW(object);

	switch (property_id)
	{
		case MULTI:
			g_value_set_boolean (value, self->multi);
			break;

		case LIST_TYPE:
			g_value_set_enum (value, zenity_tree_column_view_get_list_type (self));
			break;

		case MODEL:
			g_value_set_object (value, G_OBJECT(zenity_tree_column_view_get_model (self)));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}

static void
zenity_tree_column_view_dispose (GObject *object)
{
	ZenityTreeColumnView *self = ZENITY_TREE_COLUMN_VIEW(object);

	g_clear_pointer (&self->scrolled_window, gtk_widget_unparent);

	G_OBJECT_CLASS(zenity_tree_column_view_parent_class)->dispose (object);
}

static void
zenity_tree_column_view_init (ZenityTreeColumnView *self)
{
	GtkWidget *widget = GTK_WIDGET(self);

	gtk_widget_init_template (widget);

	gtk_widget_set_hexpand (widget, TRUE);
	gtk_widget_set_vexpand (widget, TRUE);
}

static void
zenity_tree_column_view_class_init (ZenityTreeColumnViewClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GParamFlags flags = G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT;

	object_class->dispose = zenity_tree_column_view_dispose;
	object_class->set_property = zenity_tree_column_view_set_property;
	object_class->get_property = zenity_tree_column_view_get_property;

	zenity_tree_column_view_properties[MULTI] = g_param_spec_boolean ("multi", NULL, NULL,
			FALSE, flags);
	zenity_tree_column_view_properties[LIST_TYPE] = g_param_spec_enum ("list-type", NULL, NULL,
			ZENITY_TYPE_TREE_LIST_TYPE, ZENITY_TREE_LIST_NONE, flags);
	zenity_tree_column_view_properties[MODEL] = g_param_spec_object ("model", NULL, NULL,
			G_TYPE_LIST_MODEL, flags);

	g_object_class_install_properties (object_class, N_PROPERTIES_ZENITY_TREE_COLUMN_VIEW, zenity_tree_column_view_properties);

	zenity_tree_column_view_signals[ACTIVATED] = g_signal_new_class_handler ("activated",
			G_OBJECT_CLASS_TYPE (object_class),
			G_SIGNAL_RUN_LAST,
		/* no default C function */
			NULL,
		/* defaults for accumulator, marshaller &c. */
			NULL, NULL, NULL,	
		/* No return type or params. */
			G_TYPE_NONE, 0);

	gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);

	gtk_widget_class_set_template_from_resource (widget_class, UI_FILE);

	gtk_widget_class_bind_template_child (widget_class, ZenityTreeColumnView, scrolled_window);
	gtk_widget_class_bind_template_child (widget_class, ZenityTreeColumnView, child_cv);
}

#if 0
static GtkWidget *
zenity_tree_column_view_new (GListModel *model)
{
	g_return_val_if_fail (G_IS_LIST_MODEL (model), NULL);

	return g_object_new (ZENITY_TREE_TYPE_COLUMN_VIEW,
			"model", model,
			NULL);
}
#endif

static void
editable_notify_text_cb (GtkEditable *editable, GParamSpec *pspec, ZenityTreeItem *item)
{
	const char *str = gtk_editable_get_text (editable);
	zenity_tree_item_set_text (item, str);
}

static void
factory_bind_cb (ZenityTreeColumnView *self,
		GtkListItem *list_item,
		GtkSignalListItemFactory *factory)
{
	ZenityTreeRow *row = gtk_list_item_get_item (list_item);
	int col_index = GPOINTER_TO_INT (g_object_get_data (G_OBJECT(factory), "col_index"));
	ZenityTreeItem *item;
	GtkWidget *item_child;
	const char *item_text;

	item = zenity_tree_row_get_item (row, col_index);
	item_child = zenity_tree_item_get_child (item);
	item_text = zenity_tree_item_get_text (item);
	gtk_list_item_set_child (list_item, item_child);

	gtk_widget_set_halign (item_child, GTK_ALIGN_START);

	if (GTK_IS_EDITABLE (item_child))	/* handle first to capture anything editable */
	{
		gtk_editable_set_text (GTK_EDITABLE(item_child), item_text);
		g_signal_connect (item_child, "notify::text", G_CALLBACK(editable_notify_text_cb), item);
	}
	else if (GTK_IS_LABEL (item_child))
	{
		gtk_label_set_text (GTK_LABEL(item_child), item_text);
	}
	else if (GTK_IS_CHECK_BUTTON (item_child) && item_text)
	{
		gboolean initialized = FALSE;
		gboolean checked = FALSE;

		if (g_object_get_data (G_OBJECT(item_child), "initialized"))
			initialized = TRUE;

		/* Radio-button-ize our check buttons if radio list requested
		 */
		if (self->list_type == ZENITY_TREE_LIST_RADIO)
		{
			if (!self->checkbutton_group)
				self->checkbutton_group = GTK_CHECK_BUTTON(item_child);

			/* Annoying - if you try to add checkbtn to its own group, gtk
			 * spews errors instead of just returning silently.
			 */
			if (GTK_CHECK_BUTTON(item_child) != self->checkbutton_group)	
				gtk_check_button_set_group (GTK_CHECK_BUTTON(item_child), self->checkbutton_group);
		}

		if (! initialized)
		{
			if (g_ascii_strcasecmp (item_text, "true") == 0)
				checked = TRUE;

			gtk_check_button_set_active (GTK_CHECK_BUTTON(item_child), checked);

			g_object_set_data (G_OBJECT(item_child), "initialized", GINT_TO_POINTER(TRUE));
		}
	}
	else if (GTK_IS_IMAGE (item_child) && item_text)
	{
		gtk_image_set_from_file (GTK_IMAGE(item_child), item_text);
	}
	else
	{
		g_warning ("%s: Widget type of child not implemented.", __func__);
	}
}

void
zenity_tree_column_view_add_column (ZenityTreeColumnView *self, const char *col_name)
{
	int new_col_index;
	GtkListItemFactory *factory;
	GtkColumnViewColumn *column;

	new_col_index = zenity_tree_column_view_get_n_columns (self);

	factory = gtk_signal_list_item_factory_new ();
	g_object_set_data (G_OBJECT(factory), "col_index", GINT_TO_POINTER(new_col_index));
	g_signal_connect_swapped (factory, "bind", G_CALLBACK (factory_bind_cb), self);

	/* nb: seems the signals for the factory need to be setup first *before* creating the column. */

	column = gtk_column_view_column_new (col_name, factory);
	
	if (new_col_index == 0 &&
			(self->list_type == ZENITY_TREE_LIST_CHECK || self->list_type == ZENITY_TREE_LIST_RADIO))
	{
		gtk_column_view_column_set_resizable (column, FALSE);
	}
	else
	{
		gtk_column_view_column_set_resizable (column, TRUE);
		gtk_column_view_column_set_expand (column, TRUE);
	}

	gtk_column_view_append_column (self->child_cv, column);
}

void
zenity_tree_column_view_foreach_item (ZenityTreeColumnView *self, GFunc func, gpointer user_data)
{
	GListModel *model = zenity_tree_column_view_get_model (self);

	for (guint i = 0; i < g_list_model_get_n_items (model); ++i)
	{
		ZenityTreeRow *row = g_list_model_get_item (model, i);

		for (guint j = 0; j < zenity_tree_row_get_n_items (row); ++j)
		{
			ZenityTreeItem *item = zenity_tree_row_get_item (row, j);

			func (item, user_data);
		}
	}
}

void
zenity_tree_column_view_foreach_row (ZenityTreeColumnView *self, GFunc func, gpointer user_data)
{
	GListModel *model = zenity_tree_column_view_get_model (self);

	for (guint i = 0; i < g_list_model_get_n_items (model); ++i)
	{
		ZenityTreeRow *row = g_list_model_get_item (model, i);
		func (row, user_data);
	}
}

int
zenity_tree_column_view_get_n_columns (ZenityTreeColumnView *self)
{
	return g_list_model_get_n_items (gtk_column_view_get_columns (self->child_cv));
}

gboolean
zenity_tree_column_view_is_selected (ZenityTreeColumnView *self, guint pos)
{
	return gtk_selection_model_is_selected (gtk_column_view_get_model (self->child_cv), pos);
}

void
zenity_tree_column_view_set_search (ZenityTreeColumnView *self, const char *search_str)
{
	gtk_string_filter_set_search (self->filter, search_str);
}
