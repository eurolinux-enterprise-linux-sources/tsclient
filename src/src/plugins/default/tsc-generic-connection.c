
#include <config.h>
#include <glib.h>
#include <glib/gi18n.h>
#include "tsc-generic-connection.h"
#include "tsc-util.h"
#include "tsc-connection.h"
#include "tsc-spawn.h"

#define GENERIC_CONFIG_GROUP "Generic"
#define GENERIC_KEY_COMMAND_LINE "command_line"
#define GENERIC_KEY_USE_TERMINAL "use_terminal"

static GObjectClass *parent_class;

static void
tsc_generic_connection_stopped_cb (TSCSpawn *spawn, TSCGenericConnection *connection)
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
tsc_generic_connection_start (TSCConnection *con, GError **error)
{
	TSCGenericConnection *connection = TSC_GENERIC_CONNECTION (con);
	char *command;
	char **argv;
	int i, argc;
	
	if (connection->use_terminal) {
		command = tsc_util_create_terminal_command (connection->command_line);
	} else {
		command = g_strdup (connection->command_line);
	}

	if (!g_shell_parse_argv (command, &argc, &argv, error)) {
		g_free (command);
		return FALSE;
	}

	g_free (command);

	if (connection->spawn) {
		g_object_unref (connection->spawn);
	}

	connection->spawn = tsc_spawn_new ();
	g_signal_connect (connection->spawn, "stopped",
			  G_CALLBACK (tsc_generic_connection_stopped_cb),
			  connection);

	for (i = 0; i < argc; i++) {
		tsc_spawn_append_args (connection->spawn, argv[i], NULL);
	}

	g_strfreev (argv);

	if (!tsc_spawn_start (connection->spawn, error)) {
		return FALSE;
	}

	g_signal_emit_by_name (connection, "started", NULL);
	
	return TRUE;
}

static void
tsc_generic_connection_save (TSCConnection *con, GKeyFile *keys)
{
	TSCGenericConnection *connection = TSC_GENERIC_CONNECTION (con);
	if (connection->command_line) {
		g_key_file_set_string (keys, GENERIC_CONFIG_GROUP,
				       GENERIC_KEY_COMMAND_LINE, connection->command_line);
	}

	g_key_file_set_boolean (keys, GENERIC_CONFIG_GROUP,
				GENERIC_KEY_USE_TERMINAL, connection->use_terminal);
}

static void
tsc_generic_connection_restore (TSCConnection *con, GKeyFile *keys)
{
	TSCGenericConnection *connection = TSC_GENERIC_CONNECTION (con);
	
	connection->command_line = g_key_file_get_string (keys, GENERIC_CONFIG_GROUP,
							  GENERIC_KEY_COMMAND_LINE, NULL);
	connection->use_terminal = g_key_file_get_boolean (keys, GENERIC_CONFIG_GROUP,
							   GENERIC_KEY_USE_TERMINAL, NULL);
}

static void
tsc_generic_connection_finalize (GObject *obj)
{
	TSCGenericConnection *connection = TSC_GENERIC_CONNECTION (obj);

	g_free (connection->command_line);

	if (parent_class->finalize) {
		parent_class->finalize (obj);
	}
}

static void
tsc_generic_connection_class_init (TSCGenericConnectionClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = tsc_generic_connection_finalize;
	
	TSC_CONNECTION_CLASS (klass)->start = tsc_generic_connection_start;
	TSC_CONNECTION_CLASS (klass)->save_config = tsc_generic_connection_save;
	TSC_CONNECTION_CLASS (klass)->restore_config = tsc_generic_connection_restore;
}

static void
tsc_generic_connection_init (TSCGenericConnection *connection)
{
}

GType
tsc_generic_connection_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCGenericConnectionClass),
			NULL, NULL,
			(GClassInitFunc) tsc_generic_connection_class_init,
			NULL, NULL,
			sizeof (TSCGenericConnection),
			0,
			(GInstanceInitFunc) tsc_generic_connection_init
		};
		
		type = g_type_register_static (TSC_TYPE_CONNECTION,
					       "TSCGenericConnection",
					       &type_info, 0);
	}
	
	return type;

}
