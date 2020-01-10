
#include <config.h>
#include <glib.h>
#include <glib/gi18n.h>
#include "tsc-ica-connection.h"
#include "tsc-util.h"
#include "tsc-connection.h"
#include "tsc-spawn.h"

static GObjectClass *parent_class;

static void
tsc_ica_connection_stopped_cb (TSCSpawn *spawn, TSCIcaConnection *connection)
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
tsc_ica_connection_start (TSCConnection *con, GError **error)
{
	TSCIcaConnection *connection = TSC_ICA_CONNECTION (con);
	
	if (connection->spawn) {
		g_object_unref (connection->spawn);
	}

	connection->spawn = tsc_spawn_new ();
	g_signal_connect (connection->spawn, "stopped",
			  G_CALLBACK (tsc_ica_connection_stopped_cb),
			  connection);

	tsc_spawn_set_args (connection->spawn, WFICA_PATH, "-desc",
			    connection->app_desc, NULL);
	
	if (!tsc_spawn_start (connection->spawn, error)) {
		return FALSE;
	}

	g_signal_emit_by_name (connection, "started", NULL);
	
	return TRUE;
}

static void
tsc_ica_connection_finalize (GObject *obj)
{
	TSCIcaConnection *connection = TSC_ICA_CONNECTION (obj);

	g_free (connection->app_desc);

	if (parent_class->finalize) {
		parent_class->finalize (obj);
	}
}

static void
tsc_ica_connection_class_init (TSCIcaConnectionClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = tsc_ica_connection_finalize;
	
	TSC_CONNECTION_CLASS (klass)->start = tsc_ica_connection_start;
}

static void
tsc_ica_connection_init (TSCIcaConnection *connection)
{
}

GType
tsc_ica_connection_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCIcaConnectionClass),
			NULL, NULL,
			(GClassInitFunc) tsc_ica_connection_class_init,
			NULL, NULL,
			sizeof (TSCIcaConnection),
			0,
			(GInstanceInitFunc) tsc_ica_connection_init
		};
		
		type = g_type_register_static (TSC_TYPE_CONNECTION,
					       "TSCIcaConnection",
					       &type_info, 0);
	}
	
	return type;

}

TSCIcaConnection *
tsc_ica_connection_new (const char *app_desc)
{
	TSCIcaConnection *connection;

	g_return_val_if_fail (app_desc != NULL, NULL);

	connection = TSC_ICA_CONNECTION (g_object_new (TSC_TYPE_ICA_CONNECTION, NULL));
	tsc_connection_set_name (TSC_CONNECTION (connection), app_desc);
	connection->app_desc = g_strdup (app_desc);

	return connection;
}
