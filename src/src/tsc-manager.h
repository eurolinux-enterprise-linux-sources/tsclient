
#ifndef __TSC_MANAGER__
#define __TSC_MANAGER__

#include <gtk/gtk.h>
#include "tsc-provider.h"

#define TSC_TYPE_MANAGER                   (tsc_manager_get_type ())
#define TSC_MANAGER(manager)               (G_TYPE_CHECK_INSTANCE_CAST ((manager), TSC_TYPE_MANAGER, TSCManager))
#define TSC_MANAGER_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_MANAGER, TSCManagerClass))
#define TSC_IS_MANAGER(manager)            (G_TYPE_CHECK_INSTANCE_TYPE ((manager), TSC_TYPE_MANAGER))
#define TSC_IS_MANAGER_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_MANAGER))
#define TSC_MANAGER_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_MANAGER, TSCManagerClass))

#define TSC_CONNECTION_SUFFIX ".tsc"
#define TSC_CONNECTION_ARCHIVE_SUFFIX ".nca"

typedef struct _TSCManager {
	GObject parent;
	
	GList *providers;
	GList *connections;

	char *connection_dir;
} TSCManager;

typedef struct _TSCManagerClass {
	GObjectClass parent_class;

	void (* started) (TSCManager *manager, TSCConnection *connection);
	void (* ended) (TSCManager *manager, TSCConnection *connection);
	void (* idle) (TSCManager *manager);

	void (* added) (TSCManager *manager, TSCConnection *connection);
	void (* removed) (TSCManager *manager, TSCConnection *connection);
	void (* saved) (TSCManager *manager, TSCConnection *connection);
} TSCManagerClass;

typedef void (* TSCPluginInitFunc) (TSCManager *manager);

GType          tsc_manager_get_type            (void);  
TSCManager    *tsc_manager_new                 (void);

/* providers */
void           tsc_manager_register_provider   (TSCManager *manager, TSCProvider *provider);
void           tsc_manager_unregister_provider (TSCManager *manager, TSCProvider *provider);
const GList   *tsc_manager_get_providers       (TSCManager *manager);
TSCProvider   *tsc_manager_lookup_provider     (TSCManager *manager, const char *name);

/* connections */
const GList   *tsc_manager_get_connections     (TSCManager *manager);
TSCConnection *tsc_manager_lookup_connection   (TSCManager *manager, const char *name);
TSCConnection *tsc_manager_load_connection     (TSCManager *manager, const char *file, GError **error);
void           tsc_manager_add_connection      (TSCManager *manager, TSCConnection *connection);
void           tsc_manager_save_connection     (TSCManager *manager, TSCConnection *connection);
void           tsc_manager_remove_connection   (TSCManager *manager, TSCConnection *connection);

gboolean       tsc_manager_is_idle             (TSCManager *manager);

/* import/export of connections */
gboolean       tsc_manager_export_connections  (TSCManager *manager, GList *connections,
						const char *file, GError **error);

GList         *tsc_manager_import_connections  (TSCManager *manager, const char *file,
						GError **error);


#endif
