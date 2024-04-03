/* vim: ts=4 sw=4
 */
/*
 * zenity-tree-column-view.c
 *
 * Copyright © 2023 Logan Rathbone
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
 */

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
	GParamFlags flags = G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY;

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
	g_return_val_if_fail (ZENITY_IS_TREE_ITEM (item), NULL);

	return item->child;
}

const char *
zenity_tree_item_get_text (ZenityTreeItem *item)
{
	g_return_val_if_fail (ZENITY_IS_TREE_ITEM (item), NULL);

	return item->text;
}

void
zenity_tree_item_set_text (ZenityTreeItem *self, const char *text)
{
	g_return_if_fail (ZENITY_IS_TREE_ITEM (self));

	g_clear_pointer (&self->text, g_free);
	self->text = g_strdup (text);

	g_object_notify_by_pspec (G_OBJECT(self), zenity_tree_item_properties[TEXT]);
}

void
zenity_tree_item_set_child (ZenityTreeItem *self, GtkWidget *child)
{
	g_return_if_fail (ZENITY_IS_TREE_ITEM (self));

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
	g_return_if_fail (ZENITY_IS_TREE_ROW (row));

	g_ptr_array_add (row->items, item);
}

guint
zenity_tree_row_get_n_items (ZenityTreeRow *row)
{
	g_return_val_if_fail (ZENITY_IS_TREE_ROW (row), 0);

	return row->items->len;
}

ZenityTreeItem *
zenity_tree_row_get_item (ZenityTreeRow *row, guint index)
{
	g_return_val_if_fail (ZENITY_IS_TREE_ROW (row), NULL);
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
	HIDE_HEADER,
	N_PROPERTIES_ZENITY_TREE_COLUMN_VIEW
};

static GParamSpec *zenity_tree_column_view_properties[N_PROPERTIES_ZENITY_TREE_COLUMN_VIEW];

struct _ZenityTreeColumnView
{
	GtkWidget parent_instance;
	GtkWidget *scrolled_window;
	GtkColumnView *child_cv;
	gulong child_cv_activate_handler_id;
	gboolean multi;
	gboolean hide_header;
	ZenityTreeListType list_type;
	GListModel *model;
	GtkStringFilter *filter;
	GtkCheckButton *initial_checkbutton_group;
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

	g_return_val_if_fail (ZENITY_IS_TREE_ROW (row), NULL);

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
	GtkSortListModel *sort_model;
	GtkStringFilter *filter;
	GtkFilterListModel *filter_model;
	GtkExpression *expr;

	g_return_if_fail (ZENITY_IS_TREE_COLUMN_VIEW (self));

	sort_model = gtk_sort_list_model_new (model,
			g_object_ref (gtk_column_view_get_sorter (self->child_cv)));

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
	filter_model = gtk_filter_list_model_new (G_LIST_MODEL(sort_model), GTK_FILTER(filter));

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

/* Returns the raw GListModel of *all* items within the column view. This will
 * *not* account for filtered items by search or what-have-you.
 */
GListModel *
zenity_tree_column_view_get_model (ZenityTreeColumnView *self)
{
	g_return_val_if_fail (ZENITY_IS_TREE_COLUMN_VIEW (self), NULL);

	return self->model;
}

/* Returns the actual selection model of the column view, which, as opposed to
 * the raw list model, may be filtered to show what is actually visible within
 * the widget.
 */
GtkSelectionModel *
zenity_tree_column_view_get_selection_model (ZenityTreeColumnView *self)
{
	g_return_val_if_fail (ZENITY_IS_TREE_COLUMN_VIEW (self), NULL);

	return gtk_column_view_get_model (self->child_cv);
}

static void
zenity_tree_column_view_emit_activated (ZenityTreeColumnView *self)
{
	g_signal_emit (self, zenity_tree_column_view_signals[ACTIVATED], 0);
}

static void
cv_check_or_radio_activated_cb (ZenityTreeColumnView *self, guint position, GtkColumnView *cv)
{
	ZenityTreeRow *row = g_list_model_get_item (self->model, position);
	ZenityTreeItem *item = zenity_tree_row_get_item (row, 0);
	GtkWidget *item_child = zenity_tree_item_get_child (item);
	GtkCheckButton *cb;

	if (! GTK_IS_CHECK_BUTTON (item_child))
		return;

	cb = GTK_CHECK_BUTTON(item_child);

	switch (self->list_type)
	{
		case ZENITY_TREE_LIST_CHECK:
			gtk_check_button_set_active (cb, !gtk_check_button_get_active (cb));
			break;

		case ZENITY_TREE_LIST_RADIO:
			gtk_check_button_set_active (cb, TRUE);
			break;

		default:
			g_warning ("%s: Programmer error: invalid list type.", __func__);
			break;
	}
}

void
zenity_tree_column_view_set_multi (ZenityTreeColumnView *self, gboolean multi)
{
	self->multi = multi;
	g_object_notify_by_pspec (G_OBJECT(self), zenity_tree_column_view_properties[MULTI]);
}

void
zenity_tree_column_view_set_hide_header (ZenityTreeColumnView *self, gboolean hide)
{
	self->hide_header = hide;
	g_object_notify_by_pspec (G_OBJECT(self), zenity_tree_column_view_properties[HIDE_HEADER]);
}

void
zenity_tree_column_view_set_list_type (ZenityTreeColumnView *self, ZenityTreeListType type)
{
	g_return_if_fail (ZENITY_IS_TREE_COLUMN_VIEW (self));

	self->list_type = type;

	g_clear_signal_handler (&self->child_cv_activate_handler_id, self->child_cv);

	switch (self->list_type)
	{
		case ZENITY_TREE_LIST_NONE:
		case ZENITY_TREE_LIST_IMAGE:
			gtk_column_view_set_single_click_activate (self->child_cv, FALSE);
			self->child_cv_activate_handler_id = g_signal_connect_object (self->child_cv, "activate", G_CALLBACK(zenity_tree_column_view_emit_activated), self, G_CONNECT_SWAPPED);
			break;

		case ZENITY_TREE_LIST_RADIO:
		case ZENITY_TREE_LIST_CHECK:
			gtk_column_view_set_single_click_activate (self->child_cv, TRUE);
			self->child_cv_activate_handler_id = g_signal_connect_object (self->child_cv, "activate", G_CALLBACK(cv_check_or_radio_activated_cb), self, G_CONNECT_SWAPPED);
			break;

		default:
			g_warning ("%s: Invalid ZenityTreeListType provided.", __func__);
			break;
	}

	g_object_notify_by_pspec (G_OBJECT(self), zenity_tree_column_view_properties[LIST_TYPE]);
}

ZenityTreeListType
zenity_tree_column_view_get_list_type (ZenityTreeColumnView *self)
{
	g_return_val_if_fail (ZENITY_IS_TREE_COLUMN_VIEW (self), ZENITY_TREE_LIST_NONE);

	return self->list_type;
}

gboolean
zenity_tree_column_view_get_multi (ZenityTreeColumnView *self)
{
	g_return_val_if_fail (ZENITY_IS_TREE_COLUMN_VIEW (self), FALSE);

	return self->multi;
}

gboolean
zenity_tree_column_view_get_hide_header (ZenityTreeColumnView *self)
{
	g_return_val_if_fail (ZENITY_IS_TREE_COLUMN_VIEW (self), FALSE);

	return self->hide_header;
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
			zenity_tree_column_view_set_multi (self, g_value_get_boolean (value));
			break;

		case LIST_TYPE:
			zenity_tree_column_view_set_list_type (self, g_value_get_enum (value));
			break;

		case MODEL:
			zenity_tree_column_view_set_model (self, G_LIST_MODEL(g_value_get_object (value)));
			break;

		case HIDE_HEADER:
			zenity_tree_column_view_set_hide_header (self, g_value_get_boolean (value));
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
			g_value_set_boolean (value, zenity_tree_column_view_get_multi (self));
			break;

		case LIST_TYPE:
			g_value_set_enum (value, zenity_tree_column_view_get_list_type (self));
			break;

		case MODEL:
			g_value_set_object (value, G_OBJECT(zenity_tree_column_view_get_model (self)));
			break;

		case HIDE_HEADER:
			g_value_set_boolean (value, zenity_tree_column_view_get_hide_header (self));
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

	g_clear_signal_handler (&self->child_cv_activate_handler_id, self->child_cv);
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
	GParamFlags flags = G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY;

	object_class->dispose = zenity_tree_column_view_dispose;
	object_class->set_property = zenity_tree_column_view_set_property;
	object_class->get_property = zenity_tree_column_view_get_property;

	zenity_tree_column_view_properties[MULTI] = g_param_spec_boolean ("multi", NULL, NULL,
			FALSE, flags);
	zenity_tree_column_view_properties[LIST_TYPE] = g_param_spec_enum ("list-type", NULL, NULL,
			ZENITY_TYPE_TREE_LIST_TYPE, ZENITY_TREE_LIST_NONE, flags);
	zenity_tree_column_view_properties[MODEL] = g_param_spec_object ("model", NULL, NULL,
			G_TYPE_LIST_MODEL, flags);
	zenity_tree_column_view_properties[HIDE_HEADER] = g_param_spec_boolean ("hide-header", NULL, NULL,
			TRUE, flags);

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

	if (col_index >= zenity_tree_row_get_n_items (row)) {
		g_debug ("%s: col_index exceeds number of items in row; ignoring", __func__);
		return;
	}

	item = zenity_tree_row_get_item (row, col_index);
	item_child = zenity_tree_item_get_child (item);
	item_text = zenity_tree_item_get_text (item);
	if (!gtk_widget_get_parent (item_child))
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

		if (! initialized)
		{
			/* Radio-button-ize our check buttons if radio list requested
			*/
			if (self->list_type == ZENITY_TREE_LIST_RADIO)
			{
				if (!self->initial_checkbutton_group)
				{
					self->initial_checkbutton_group = GTK_CHECK_BUTTON(item_child);
				}

				/* Annoying - if you try to add checkbtn to its own group, gtk
				 * spews errors instead of just returning silently.
				 */
				if (GTK_CHECK_BUTTON(item_child) != self->initial_checkbutton_group)	
					gtk_check_button_set_group (GTK_CHECK_BUTTON(item_child), self->initial_checkbutton_group);
			}

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

static char *
tree_item_str_cb (ZenityTreeRow *row, gpointer user_data)
{
	int col_index;
	ZenityTreeItem *item;

	g_return_val_if_fail (ZENITY_IS_TREE_ROW (row), NULL);

	col_index = GPOINTER_TO_INT(user_data);
	item = row->items->pdata[col_index];

	return g_strdup (item->text);
}

void
zenity_tree_column_view_add_column (ZenityTreeColumnView *self, const char *col_name)
{
	g_return_if_fail (ZENITY_IS_TREE_COLUMN_VIEW (self));

	int new_col_index;
	GtkListItemFactory *factory;
	GtkColumnViewColumn *column;

	new_col_index = zenity_tree_column_view_get_n_columns (self);

	factory = gtk_signal_list_item_factory_new ();
	g_object_set_data (G_OBJECT(factory), "col_index", GINT_TO_POINTER(new_col_index));
	g_signal_connect_swapped (factory, "bind", G_CALLBACK (factory_bind_cb), self);

	/* nb: seems the signals for the factory need to be setup first *before* creating the column. */

	column = gtk_column_view_column_new (self->hide_header ? NULL : col_name, factory);
	
	if (new_col_index == 0 &&
			(self->list_type == ZENITY_TREE_LIST_CHECK || self->list_type == ZENITY_TREE_LIST_RADIO))
	{
		gtk_column_view_column_set_resizable (column, FALSE);
	}
	else
	{
		GtkExpression *expr;
		GtkStringSorter *sorter;

		expr = gtk_cclosure_expression_new (G_TYPE_STRING,
				NULL,					/* GClosureMarshal marshal, */
				0, 						/* guint n_params, */
				NULL,					/* GtkExpression** params, */
				G_CALLBACK(tree_item_str_cb),	/* GCallback callback_func, */
				GINT_TO_POINTER(new_col_index), /* gpointer user_data, */
				NULL);					/* GClosureNotify user_destroy) */

		sorter = gtk_string_sorter_new (expr);

		gtk_column_view_column_set_resizable (column, TRUE);
		gtk_column_view_column_set_expand (column, TRUE);
		gtk_column_view_column_set_sorter (column, GTK_SORTER(sorter));
	}

	gtk_column_view_append_column (self->child_cv, column);
}

void
zenity_tree_column_view_foreach_item (ZenityTreeColumnView *self, GFunc func, gpointer user_data)
{
	g_return_if_fail (ZENITY_IS_TREE_COLUMN_VIEW (self));

	for (guint i = 0; i < g_list_model_get_n_items (self->model); ++i)
	{
		ZenityTreeRow *row = g_list_model_get_item (self->model, i);

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
	g_return_if_fail (ZENITY_IS_TREE_COLUMN_VIEW (self));

	for (guint i = 0; i < g_list_model_get_n_items (self->model); ++i)
	{
		ZenityTreeRow *row = g_list_model_get_item (self->model, i);
		func (row, user_data);
	}
}

int
zenity_tree_column_view_get_n_columns (ZenityTreeColumnView *self)
{
	g_return_val_if_fail (ZENITY_IS_TREE_COLUMN_VIEW (self), -1);

	return g_list_model_get_n_items (gtk_column_view_get_columns (self->child_cv));
}

gboolean
zenity_tree_column_view_is_selected (ZenityTreeColumnView *self, guint pos)
{
	g_return_val_if_fail (ZENITY_IS_TREE_COLUMN_VIEW (self), FALSE);

	return gtk_selection_model_is_selected (gtk_column_view_get_model (self->child_cv), pos);
}

void
zenity_tree_column_view_set_search (ZenityTreeColumnView *self, const char *search_str)
{
	g_return_if_fail (ZENITY_IS_TREE_COLUMN_VIEW (self));

	gtk_string_filter_set_search (self->filter, search_str);
}

void
zenity_tree_column_view_show_column (ZenityTreeColumnView *self, guint pos, gboolean show)
{
	g_return_if_fail (ZENITY_IS_TREE_COLUMN_VIEW (self));

	GListModel *cols = gtk_column_view_get_columns (self->child_cv);
	GtkColumnViewColumn *col = g_list_model_get_item (cols, pos);

	gtk_column_view_column_set_visible (col, show);
}
