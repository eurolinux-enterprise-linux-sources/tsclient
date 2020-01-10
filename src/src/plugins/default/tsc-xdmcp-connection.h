
#ifndef __TSC_XDMCP_CONNECTION__
#define __TSC_XDMCP_CONNECTION__

#include <glib.h>
#include <glib-object.h>
#include "tsc-connection.h"
#include "tsc-spawn.h"

#define TSC_TYPE_XDMCP_CONNECTION                   (tsc_xdmcp_connection_get_type ())
#define TSC_XDMCP_CONNECTION(connection)            (G_TYPE_CHECK_INSTANCE_CAST ((connection), TSC_TYPE_XDMCP_CONNECTION, TSCXdmcpConnection))
#define TSC_XDMCP_CONNECTION_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_XDMCP_CONNECTION, TSCXdmcpConnectionClass))
#define TSC_IS_XDMCP_CONNECTION(connection)         (G_TYPE_CHECK_INSTANCE_TYPE ((connection), TSC_TYPE_XDMCP_CONNECTION))
#define TSC_IS_XDMCP_CONNECTION_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_XDMCP_CONNECTION))
#define TSC_XDMCP_CONNECTION_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_XDMCP_CONNECTION, TSCXdmcpConnectionClass))

typedef struct _TSCXdmcpConnection {
	TSCConnection parent;

	char *host;
	TSCSpawn *spawn;

    int width;
    int height;
    gboolean fullscreen;
} TSCXdmcpConnection;

typedef struct _TSCXdmcpConnectionClass {
	TSCConnectionClass parent_class;
} TSCXdmcpConnectionClass;


GType       tsc_xdmcp_connection_get_type        (void);

#endif
