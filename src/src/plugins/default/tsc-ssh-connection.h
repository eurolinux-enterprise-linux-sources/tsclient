
#ifndef __TSC_SSH_CONNECTION__
#define __TSC_SSH_CONNECTION__

#include <glib.h>
#include <glib-object.h>
#include "tsc-connection.h"
#include "tsc-spawn.h"

#define TSC_TYPE_SSH_CONNECTION                   (tsc_ssh_connection_get_type ())
#define TSC_SSH_CONNECTION(connection)            (G_TYPE_CHECK_INSTANCE_CAST ((connection), TSC_TYPE_SSH_CONNECTION, TSCSshConnection))
#define TSC_SSH_CONNECTION_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_SSH_CONNECTION, TSCSshConnectionClass))
#define TSC_IS_SSH_CONNECTION(connection)         (G_TYPE_CHECK_INSTANCE_TYPE ((connection), TSC_TYPE_SSH_CONNECTION))
#define TSC_IS_SSH_CONNECTION_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_SSH_CONNECTION))
#define TSC_SSH_CONNECTION_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_SSH_CONNECTION, TSCSshConnectionClass))

typedef struct _TSCSshConnection {
	TSCConnection parent;

	char *host_line;
	char *user_line;
	char *password_line;
	char *options_line;
	char *remote_line;
	gboolean use_x11;
	gboolean use_terminal;

	TSCSpawn *spawn;
} TSCSshConnection;

typedef struct _TSCSshConnectionClass {
	TSCConnectionClass parent_class;
} TSCSshConnectionClass;


GType       tsc_ssh_connection_get_type        (void);

#endif
