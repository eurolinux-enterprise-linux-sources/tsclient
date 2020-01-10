
#ifndef __TSC_GENERIC_CONNECTION__
#define __TSC_GENERIC_CONNECTION__

#include <glib.h>
#include <glib-object.h>
#include "tsc-connection.h"
#include "tsc-spawn.h"

#define TSC_TYPE_GENERIC_CONNECTION                   (tsc_generic_connection_get_type ())
#define TSC_GENERIC_CONNECTION(connection)            (G_TYPE_CHECK_INSTANCE_CAST ((connection), TSC_TYPE_GENERIC_CONNECTION, TSCGenericConnection))
#define TSC_GENERIC_CONNECTION_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_GENERIC_CONNECTION, TSCGenericConnectionClass))
#define TSC_IS_GENERIC_CONNECTION(connection)         (G_TYPE_CHECK_INSTANCE_TYPE ((connection), TSC_TYPE_GENERIC_CONNECTION))
#define TSC_IS_GENERIC_CONNECTION_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_GENERIC_CONNECTION))
#define TSC_GENERIC_CONNECTION_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_GENERIC_CONNECTION, TSCGenericConnectionClass))

typedef struct _TSCGenericConnection {
	TSCConnection parent;

	char *command_line;
	gboolean use_terminal;

	TSCSpawn *spawn;
} TSCGenericConnection;

typedef struct _TSCGenericConnectionClass {
	TSCConnectionClass parent_class;
} TSCGenericConnectionClass;


GType       tsc_generic_connection_get_type        (void);

#endif
