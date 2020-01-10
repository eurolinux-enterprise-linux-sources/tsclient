
#ifndef __TSC_WEB_CONNECTION__
#define __TSC_WEB_CONNECTION__

#include <glib.h>
#include <glib-object.h>
#include "tsc-connection.h"
#include "tsc-manager.h"
#include "tsc-spawn.h"

#define TSC_TYPE_WEB_CONNECTION                   (tsc_web_connection_get_type ())
#define TSC_WEB_CONNECTION(connection)            (G_TYPE_CHECK_INSTANCE_CAST ((connection), TSC_TYPE_WEB_CONNECTION, TSCWebConnection))
#define TSC_WEB_CONNECTION_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_WEB_CONNECTION, TSCWebConnectionClass))
#define TSC_IS_WEB_CONNECTION(connection)         (G_TYPE_CHECK_INSTANCE_TYPE ((connection), TSC_TYPE_WEB_CONNECTION))
#define TSC_IS_WEB_CONNECTION_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_WEB_CONNECTION))
#define TSC_WEB_CONNECTION_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_WEB_CONNECTION, TSCWebConnectionClass))

typedef struct _TSCWebConnection {
	TSCConnection parent;

	char *url;

	TSCSpawn *spawn;
} TSCWebConnection;

typedef struct _TSCWebConnectionClass {
	TSCConnectionClass parent_class;
} TSCWebConnectionClass;

void        tsc_web_connection_removed         (TSCManager *, TSCConnection *, gpointer);
GType       tsc_web_connection_get_type        (void);

#endif
