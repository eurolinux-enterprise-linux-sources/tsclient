
#ifndef __TSC_MAINFRAME_CONNECTION__
#define __TSC_MAINFRAME_CONNECTION__

#include <glib.h>
#include <glib-object.h>
#include "tsc-connection.h"
#include "tsc-spawn.h"

#define TSC_TYPE_MAINFRAME_CONNECTION                   (tsc_mainframe_connection_get_type ())
#define TSC_MAINFRAME_CONNECTION(connection)            (G_TYPE_CHECK_INSTANCE_CAST ((connection), TSC_TYPE_MAINFRAME_CONNECTION, TSCMainframeConnection))
#define TSC_MAINFRAME_CONNECTION_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_MAINFRAME_CONNECTION, TSCMainframeConnectionClass))
#define TSC_IS_MAINFRAME_CONNECTION(connection)         (G_TYPE_CHECK_INSTANCE_TYPE ((connection), TSC_TYPE_MAINFRAME_CONNECTION))
#define TSC_IS_MAINFRAME_CONNECTION_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_MAINFRAME_CONNECTION))
#define TSC_MAINFRAME_CONNECTION_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_MAINFRAME_CONNECTION, TSCMainframeConnectionClass))

typedef struct _TSCMainframeConnection {
	TSCConnection parent;

	char *host;
	int port;

	TSCSpawn *spawn;
} TSCMainframeConnection;

typedef struct _TSCMainframeConnectionClass {
	TSCConnectionClass parent_class;
} TSCMainframeConnectionClass;


GType       tsc_mainframe_connection_get_type        (void);

#endif
