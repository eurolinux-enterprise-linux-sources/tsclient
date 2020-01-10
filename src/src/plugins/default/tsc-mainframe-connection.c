
#include <config.h>
#include <glib.h>
#include <glib/gi18n.h>
#include "tsc-mainframe-connection.h"
#include "tsc-util.h"
#include "tsc-connection.h"
#include "tsc-spawn.h"

#define MAINFRAME_CONFIG_GROUP "TN3270"
#define MAINFRAME_KEY_HOST "host"
#define MAINFRAME_KEY_PORT "port"
#define MAINFRAME_DEFAULT_PORT 23

static GObjectClass *parent_class;

static void
tsc_mainframe_connection_stopped_cb (TSCSpawn *spawn, TSCMainframeConnection *connection)
{
	GError *error = NULL;

	error = tsc_spawn_get_error (spawn);
	
	g_object_unref (connection->spawn);
	connection->spawn = NULL;
	
	g_signal_emit_by_name (connection, "ended", error, NULL);
	
	if (error) {
		g_error_free (error);
	}
}

static gboolean
tsc_mainframe_connection_start (TSCConnection *con, GError **error)
{
    TSCMainframeConnection *connection = TSC_MAINFRAME_CONNECTION (con);
	char *portstr;
	
	if (connection->spawn) {
		g_object_unref (connection->spawn);
	}

	connection->spawn = tsc_spawn_new ();
	g_signal_connect (connection->spawn, "stopped",
			  G_CALLBACK (tsc_mainframe_connection_stopped_cb),
			  connection);

	portstr = g_strdup_printf ("%d", connection->port);
	tsc_spawn_set_args (connection->spawn,
			    "x3270", "-once",
			    "-port", portstr,
			    connection->host,
			    NULL);
	g_free (portstr);

	if (!tsc_spawn_start (connection->spawn, error)) {
		return FALSE;
	}

	g_signal_emit_by_name (connection, "started", NULL);
	
	return TRUE;
}

static void
tsc_mainframe_connection_save (TSCConnection *con, GKeyFile *keys)
{
    TSCMainframeConnection *connection = TSC_MAINFRAME_CONNECTION (con);
	
    if (connection->host) {
		g_key_file_set_string (keys, MAINFRAME_CONFIG_GROUP,
				       MAINFRAME_KEY_HOST, connection->host);
	}

	g_key_file_set_integer (keys, MAINFRAME_CONFIG_GROUP,
				MAINFRAME_KEY_PORT, connection->port);
}

static void
tsc_mainframe_connection_restore (TSCConnection *con, GKeyFile *keys)
{
    TSCMainframeConnection *connection = TSC_MAINFRAME_CONNECTION (con);
	connection->host = g_key_file_get_string (keys, MAINFRAME_CONFIG_GROUP,
						  MAINFRAME_KEY_HOST, NULL);
	connection->port = g_key_file_get_integer (keys, MAINFRAME_CONFIG_GROUP,
						   MAINFRAME_KEY_PORT, NULL);
}

static void
tsc_mainframe_connection_finalize (GObject *obj)
{
	TSCMainframeConnection *connection = TSC_MAINFRAME_CONNECTION (obj);

	g_free (connection->host);

	if (parent_class->finalize) {
		parent_class->finalize (obj);
	}
}

static void
tsc_mainframe_connection_class_init (TSCMainframeConnectionClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = tsc_mainframe_connection_finalize;
	
	TSC_CONNECTION_CLASS (klass)->start = tsc_mainframe_connection_start;
	TSC_CONNECTION_CLASS (klass)->save_config = tsc_mainframe_connection_save;
	TSC_CONNECTION_CLASS (klass)->restore_config = tsc_mainframe_connection_restore;
}

static void
tsc_mainframe_connection_init (TSCMainframeConnection *connection)
{
	connection->port = MAINFRAME_DEFAULT_PORT;
}

GType
tsc_mainframe_connection_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCMainframeConnectionClass),
			NULL, NULL,
			(GClassInitFunc) tsc_mainframe_connection_class_init,
			NULL, NULL,
			sizeof (TSCMainframeConnection),
			0,
			(GInstanceInitFunc) tsc_mainframe_connection_init
		};
		
		type = g_type_register_static (TSC_TYPE_CONNECTION,
					       "TSCMainframeConnection",
					       &type_info, 0);
	}
	
	return type;

}
