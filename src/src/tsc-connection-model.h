
#ifndef __TSC_CONNECTION_MODEL__
#define __TSC_CONNECTION_MODEL__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include "tsc-manager.h"

#define TSC_TYPE_CONNECTION_MODEL                   (tsc_connection_model_get_type ())
#define TSC_CONNECTION_MODEL(model)                 (G_TYPE_CHECK_INSTANCE_CAST ((model), TSC_TYPE_CONNECTION_MODEL, TSCConnectionModel))
#define TSC_CONNECTION_MODEL_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_CONNECTION_MODEL, TSCConnectionModelClass))
#define TSC_IS_CONNECTION_MODEL(model)              (G_TYPE_CHECK_INSTANCE_TYPE ((model), TSC_TYPE_CONNECTION_MODEL))
#define TSC_IS_CONNECTION_MODEL_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_CONNECTION_MODEL))
#define TSC_CONNECTION_MODEL_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_CONNECTION_MODEL, TSCConnectionModelClass))

enum {
	TSC_MODEL_COLUMN_NAME,
	TSC_MODEL_COLUMN_PIXBUF,
	TSC_MODEL_COLUMN_CONNECTION,
	TSC_MODEL_COLUMN_LAST
};

typedef struct _TSCConnectionModel {
	GtkListStore parent;

	TSCManager *manager;
} TSCConnectionModel;

typedef struct _TSCConnectionModelClass {
	GtkListStoreClass parent_class;
} TSCConnectionModelClass;


GType         tsc_connection_model_get_type        (void);

GtkTreeModel *tsc_connection_model_new             (TSCManager *manager);

#endif
