
#include <config.h>
#include <glib.h>
#include <glib/gi18n.h>
#include "tsc-util.h"
#include "tsc-connection-model.h"
#include "tsc-connection.h"
#include "tsc-util.h"

static GObjectClass *parent_class;

static void
tsc_connection_model_added_cb (TSCManager         *manager,
			       TSCConnection      *connection,
			       GtkListStore       *store)
{
	GtkTreeIter iter;

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    TSC_MODEL_COLUMN_NAME, tsc_connection_get_name (connection),
			    TSC_MODEL_COLUMN_PIXBUF,
			    tsc_util_get_icon (tsc_provider_get_icon_name (tsc_connection_get_provider (connection)),
					       TSC_PROVIDER_ICON_SIZE),
			    TSC_MODEL_COLUMN_CONNECTION, connection, -1);
}

static gboolean
tsc_connection_model_lookup (GtkTreeModel *model, GtkTreeIter *iter,
			     TSCConnection *connection)
{
	TSCConnection *item;
	
	g_return_val_if_fail (model != NULL, FALSE);
	g_return_val_if_fail (iter != NULL, FALSE);

	if (!gtk_tree_model_get_iter_first (model, iter))
		return FALSE;

	do {
		gtk_tree_model_get (model, iter,
				    TSC_MODEL_COLUMN_CONNECTION,
				    &item, -1);
		if (item == connection) {
			return TRUE;
		}
	} while (gtk_tree_model_iter_next (model, iter));

	return FALSE;
}

static void
tsc_connection_model_removed_cb (TSCManager    *manager,
				 TSCConnection *connection,
				 GtkTreeModel  *model)
{
	GtkTreeIter iter;

	if (tsc_connection_model_lookup (model, &iter, connection)) {
		gtk_list_store_remove (GTK_LIST_STORE (model),
				       &iter);
	}
}

static void
tsc_connection_model_saved_cb (TSCManager    *manager,
			       TSCConnection *connection,
			       GtkTreeModel  *model)
{
	GtkTreeIter iter;
	char *markup;

	if (tsc_connection_model_lookup (model, &iter, connection)) {
		if (tsc_connection_is_connected (connection)) {
			markup = g_strdup_printf ("<b>%s</b>",
						  tsc_connection_get_name (connection));
		} else {
			markup = g_strdup (tsc_connection_get_name (connection));
		}
		
		gtk_list_store_set (GTK_LIST_STORE (model), &iter,
				    TSC_MODEL_COLUMN_NAME,
				    markup,
				    -1);
		g_free (markup);
	}
				    
}

static void
tsc_connection_model_finalize (GObject *obj)
{
	TSCConnectionModel *mdl = TSC_CONNECTION_MODEL (obj);

	g_object_unref (mdl->manager);
	
	if (parent_class->finalize) {
		parent_class->finalize (obj);
	}
}

static void
tsc_connection_model_class_init (TSCConnectionModelClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = tsc_connection_model_finalize;
}

static void
tsc_connection_model_init (TSCConnectionModel *model)
{
	GType types[] = { G_TYPE_STRING, GDK_TYPE_PIXBUF, TSC_TYPE_CONNECTION };
	
	gtk_list_store_set_column_types (GTK_LIST_STORE (model), TSC_MODEL_COLUMN_LAST, types);
}

GType
tsc_connection_model_get_type (void)
{
	static GType type = 0;

	
	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCConnectionModelClass),
			NULL, NULL,
			(GClassInitFunc) tsc_connection_model_class_init,
			NULL, NULL,
			sizeof (TSCConnectionModel),
			0,
			(GInstanceInitFunc) tsc_connection_model_init
		};
		
		type = g_type_register_static (GTK_TYPE_LIST_STORE,
					       "TSCConnectionModel",
					       &type_info, 0);
	}
	
	return type;
}


GtkTreeModel *
tsc_connection_model_new (TSCManager *manager)
{
	TSCConnectionModel *model;
	const GList *l;
	
	model = g_object_new (TSC_TYPE_CONNECTION_MODEL, NULL);
	model->manager = g_object_ref (manager);

	for (l = tsc_manager_get_connections (manager); l; l = l->next) {
		tsc_connection_model_added_cb (manager, TSC_CONNECTION (l->data),
					       GTK_LIST_STORE (model));
	}

	g_signal_connect (model->manager, "added",
			  G_CALLBACK (tsc_connection_model_added_cb),
			  model);
	g_signal_connect (model->manager, "removed",
			  G_CALLBACK (tsc_connection_model_removed_cb),
			  model);
	g_signal_connect (model->manager, "saved",
			  G_CALLBACK (tsc_connection_model_saved_cb),
			  model);
	g_signal_connect (model->manager, "started",
			  G_CALLBACK (tsc_connection_model_saved_cb),
			  model);
	g_signal_connect (model->manager, "ended",
			  G_CALLBACK (tsc_connection_model_saved_cb),
			  model);

	return GTK_TREE_MODEL (model);
}
