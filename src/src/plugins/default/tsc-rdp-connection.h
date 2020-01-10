
#ifndef __TSC_RDP_CONNECTION__
#define __TSC_RDP_CONNECTION__

#include <glib.h>
#include <glib-object.h>
#include "tsc-connection.h"
#include "tsc-spawn.h"

#define TSC_TYPE_RDP_CONNECTION                   (tsc_rdp_connection_get_type ())
#define TSC_RDP_CONNECTION(connection)            (G_TYPE_CHECK_INSTANCE_CAST ((connection), TSC_TYPE_RDP_CONNECTION, TSCRdpConnection))
#define TSC_RDP_CONNECTION_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_RDP_CONNECTION, TSCRdpConnectionClass))
#define TSC_IS_RDP_CONNECTION(connection)         (G_TYPE_CHECK_INSTANCE_TYPE ((connection), TSC_TYPE_RDP_CONNECTION))
#define TSC_IS_RDP_CONNECTION_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_RDP_CONNECTION))
#define TSC_RDP_CONNECTION_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_RDP_CONNECTION, TSCRdpConnectionClass))

#define TSC_RDP_PROGRAM "rdesktop"

typedef enum {
	TSC_RDP_EXPERIENCE_DEFAULT,
	TSC_RDP_EXPERIENCE_MODEM,
	TSC_RDP_EXPERIENCE_BROADBAND,
	TSC_RDP_EXPERIENCE_LAN,
} TSCRdpExperience;
	

typedef struct _TSCRdpConnection {
	TSCConnection parent;

	char *host;
	
	char *username;
	char *password;
	char *domain;

	char *shell;
	char *initial_directory;

	int width;
	int height;
	int bpp;

	gboolean fullscreen;
	gboolean encryption;
	gboolean compression;
	gboolean sound;

	TSCRdpExperience experience;

	TSCSpawn *spawn;
} TSCRdpConnection;

typedef struct _TSCRdpConnectionClass {
	TSCConnectionClass parent_class;
} TSCRdpConnectionClass;


GType       tsc_rdp_connection_get_type        (void);

#endif
