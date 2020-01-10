
#ifndef __TSC_VNC_CONNECTION__
#define __TSC_VNC_CONNECTION__

#include <glib.h>
#include <glib-object.h>
#include "tsc-connection.h"
#include "tsc-spawn.h"

#define TSC_TYPE_VNC_CONNECTION                   (tsc_vnc_connection_get_type ())
#define TSC_VNC_CONNECTION(connection)            (G_TYPE_CHECK_INSTANCE_CAST ((connection), TSC_TYPE_VNC_CONNECTION, TSCVncConnection))
#define TSC_VNC_CONNECTION_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_VNC_CONNECTION, TSCVncConnectionClass))
#define TSC_IS_VNC_CONNECTION(connection)         (G_TYPE_CHECK_INSTANCE_TYPE ((connection), TSC_TYPE_VNC_CONNECTION))
#define TSC_IS_VNC_CONNECTION_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_VNC_CONNECTION))
#define TSC_VNC_CONNECTION_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_VNC_CONNECTION, TSCVncConnectionClass))

typedef struct _TSCVncConnection {
	TSCConnection parent;

	char *host;
	char *password;

	gboolean shared;
	gboolean viewonly;
	gboolean fullscreen;
	
	int width;
	int height;

	TSCSpawn *spawn;
} TSCVncConnection;

typedef struct _TSCVncConnectionClass {
	TSCConnectionClass parent_class;
} TSCVncConnectionClass;


GType       tsc_vnc_connection_get_type        (void);

#endif
