
#ifndef __TSC_ICA_CONNECTION__
#define __TSC_ICA_CONNECTION__

#include <glib.h>
#include <glib-object.h>
#include "tsc-connection.h"
#include "tsc-spawn.h"

#define TSC_TYPE_ICA_CONNECTION                   (tsc_ica_connection_get_type ())
#define TSC_ICA_CONNECTION(connection)            (G_TYPE_CHECK_INSTANCE_CAST ((connection), TSC_TYPE_ICA_CONNECTION, TSCIcaConnection))
#define TSC_ICA_CONNECTION_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_ICA_CONNECTION, TSCIcaConnectionClass))
#define TSC_IS_ICA_CONNECTION(connection)         (G_TYPE_CHECK_INSTANCE_TYPE ((connection), TSC_TYPE_ICA_CONNECTION))
#define TSC_IS_ICA_CONNECTION_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_ICA_CONNECTION))
#define TSC_ICA_CONNECTION_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_ICA_CONNECTION, TSCIcaConnectionClass))

#define WFICA_PATH "/usr/lib/ICAClient/wfica"
#define WFCMGR_PATH "/usr/lib/ICAClient/wfcmgr"

typedef struct _TSCIcaConnection {
	TSCConnection parent;

	char *app_desc;

	TSCSpawn *spawn;
} TSCIcaConnection;

typedef struct _TSCIcaConnectionClass {
	TSCConnectionClass parent_class;
} TSCIcaConnectionClass;

TSCIcaConnection *tsc_ica_connection_new       (const char *app_desc);
GType             tsc_ica_connection_get_type  (void);

#endif
