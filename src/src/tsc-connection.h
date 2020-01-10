
#ifndef __TSC_CONNECTION__
#define __TSC_CONNECTION__

#include <glib.h>
#include <glib-object.h>

#define TSC_TYPE_CONNECTION                   (tsc_connection_get_type ())
#define TSC_CONNECTION(connection)            (G_TYPE_CHECK_INSTANCE_CAST ((connection), TSC_TYPE_CONNECTION, TSCConnection))
#define TSC_CONNECTION_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_CONNECTION, TSCConnectionClass))
#define TSC_IS_CONNECTION(connection)         (G_TYPE_CHECK_INSTANCE_TYPE ((connection), TSC_TYPE_CONNECTION))
#define TSC_IS_CONNECTION_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_CONNECTION))
#define TSC_CONNECTION_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_CONNECTION, TSCConnectionClass))

typedef enum {
	TSC_RECONNECT_NEVER,
	TSC_RECONNECT_ALWAYS,
	TSC_RECONNECT_ERROR,
	TSC_RECONNECT_PROMPT_ERROR,
} TSCReconnectPolicy;

typedef struct _TSCConnection {
	GObject parent;

	struct _TSCProvider *provider;
	char *filename;
	char *name;

	TSCReconnectPolicy reconnect_policy;
	gboolean autostart;
	gboolean favorite;
	gboolean connected;
} TSCConnection;

typedef struct _TSCConnectionClass {
	GObjectClass parent_class;

	/* signals */
	void (* started) (TSCConnection *connection);
	void (* ended)   (TSCConnection *connection, GError *error);

	/* virtual methods (not signals) */
	gboolean (* start) (TSCConnection *connection, GError **error);
	void (* save_config) (TSCConnection *connection, GKeyFile *kf);
	void (* restore_config) (TSCConnection *connection, GKeyFile *kf);
} TSCConnectionClass;


GType       tsc_connection_get_type        (void);

const char *tsc_connection_get_filename    (TSCConnection *connection);
void        tsc_connection_set_filename    (TSCConnection *connection, const char *fname);

const char *tsc_connection_get_name        (TSCConnection *connection);
void        tsc_connection_set_name        (TSCConnection *connection, const char *name);



gboolean    tsc_connection_start           (TSCConnection *connection, GError **error);
void        tsc_connection_save_config     (TSCConnection *connection, GKeyFile *kf);
void        tsc_connection_restore_config  (TSCConnection *connection, GKeyFile *kf);

gboolean    tsc_connection_is_connected    (TSCConnection *connection);

struct _TSCProvider *tsc_connection_get_provider         (TSCConnection *connection);

void                 tsc_connection_set_reconnect_policy (TSCConnection *connection,
							  TSCReconnectPolicy policy);
TSCReconnectPolicy   tsc_connection_get_reconnect_policy (TSCConnection *connection);

gboolean             tsc_connection_get_autostart        (TSCConnection *connection);
void                 tsc_connection_set_autostart        (TSCConnection *connection,
							  gboolean       autostart);

gboolean             tsc_connection_get_favorite        (TSCConnection *connection);
void                 tsc_connection_set_favorite        (TSCConnection *connection, gboolean favorite);
void                 tsc_connection_write_favorite      (TSCConnection *connection, gboolean add);

gboolean             tsc_connection_can_save             (TSCConnection *connection);
gboolean             tsc_connection_can_restore          (TSCConnection *connection);
gboolean             tsc_connection_create_shortcut 	 (TSCConnection *connection, const gchar *location);

/* protected methods -- only subclasses should use these */
typedef void (* TSCWatchCallback) (TSCConnection *connection, GPid pid, int status, const char *errors);

void                 tsc_connection_watch_process        (TSCConnection   *connection,
							  GPid             pid,
							  int              child_stderr,
							  TSCWatchCallback callback);

#endif
