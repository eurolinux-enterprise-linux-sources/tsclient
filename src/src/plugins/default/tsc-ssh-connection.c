
#include <config.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <unistd.h>
#include <stdio.h>
#include "tsc-ssh-connection.h"
#include "tsc-util.h"
#include "tsc-connection.h"
#include "tsc-spawn.h"

#define SSH_CONFIG_GROUP "SSH"
#define SSH_KEY_HOST_LINE "host_line"
#define SSH_KEY_USER_LINE "user_line"
#define SSH_KEY_PASSWORD_LINE "password_line"
#define SSH_KEY_OPTIONS_LINE "options_line"
#define SSH_KEY_REMOTE_LINE "remote_line"
#define SSH_KEY_USE_X11 "use_x11"
#define SSH_KEY_USE_TERMINAL "use_terminal"
#define TSC_SSH_ASKPASS LIBDIR "/tsclient/tsc-ssh-askpass"

static GObjectClass *parent_class;

static void
tsc_ssh_connection_stopped_cb (TSCSpawn *spawn, TSCSshConnection *connection)
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
tsc_ssh_connection_parse_and_append (TSCSshConnection *connection, const char *command, GError ***error)
{
	char **argv;
	int i, argc;
	
	if (!g_shell_parse_argv (command, &argc, &argv, *error)) {
		return FALSE;
	}
	
	for (i = 0; i < argc; i++) {
		tsc_spawn_append_args (connection->spawn, argv[i], NULL);
	}
	
	g_strfreev (argv);
	return TRUE;
}

static void
tsc_ssh_connection_setup (gpointer user_data)
{
	TSCConnection *con = TSC_CONNECTION (user_data);
	
	setsid ();
	
	if (g_file_test (TSC_SSH_ASKPASS, G_FILE_TEST_EXISTS)) {
		g_setenv ("OLD_SSH_ASKPASS", g_getenv ("SSH_ASKPASS"), TRUE);
		g_setenv ("TSC_USER", g_get_user_name (), TRUE);
		g_setenv ("TSC_CONNECTION_FILE", tsc_connection_get_filename (con), TRUE);
		g_setenv ("SSH_ASKPASS", TSC_SSH_ASKPASS, TRUE);
	}
}

static gboolean
tsc_ssh_connection_start (TSCConnection *con, GError **error)
{
	TSCSshConnection *connection = TSC_SSH_CONNECTION (con);

	if (connection->spawn) {
		g_object_unref (connection->spawn);
	}

	connection->spawn = tsc_spawn_new ();
	g_signal_connect (connection->spawn, "stopped",
			  G_CALLBACK (tsc_ssh_connection_stopped_cb),
			  connection);

	if (connection->use_terminal) {
		gchar *cmd = tsc_util_create_terminal_command ("ssh");
		
		if (!tsc_ssh_connection_parse_and_append (connection, cmd, &error)) {
			return FALSE;
		}

		g_free (cmd);
	} else {
		tsc_spawn_set_args (connection->spawn, "ssh", "-f", "-n", NULL);
	}

	if (connection->use_x11) {
		tsc_spawn_append_args (connection->spawn, "-X", NULL);
	}

	if (connection->user_line) {
		tsc_spawn_append_args (connection->spawn, "-l", connection->user_line, NULL);
	}
	
	if (connection->options_line && g_ascii_strcasecmp ("", connection->options_line) && 
		!tsc_ssh_connection_parse_and_append (connection, connection->options_line, &error)) {
		return FALSE;
	}
	
	if (connection->host_line) {
		tsc_spawn_append_args (connection->spawn, connection->host_line, NULL);
	}

	if (connection->remote_line && g_ascii_strcasecmp ("", connection->remote_line) &&
		!tsc_ssh_connection_parse_and_append (connection, connection->remote_line, &error)) {
		return FALSE;
	}
	
	if (connection->use_terminal) {
		if (!tsc_spawn_start (connection->spawn, error)) {
			return FALSE;
		}
	} else {
		/* 
		 * If we're not using a terminal then we need to get
		 * rid of our controlling terminal so ssh consults
		 * SSH_ASKPASS.
		 */
		if (!tsc_spawn_start_with_setup (connection->spawn, 
										 tsc_ssh_connection_setup, connection, error)) {
			return FALSE;
		}
	}

	g_signal_emit_by_name (connection, "started", NULL);
	
	return TRUE;
}

static void
tsc_ssh_connection_save (TSCConnection *con, GKeyFile *keys)
{
	TSCSshConnection *connection = TSC_SSH_CONNECTION (con);

	if (connection->host_line) {
		g_key_file_set_string (keys, SSH_CONFIG_GROUP,
				       SSH_KEY_HOST_LINE, connection->host_line);
	}

	if (connection->user_line) {
		g_key_file_set_string (keys, SSH_CONFIG_GROUP,
				       SSH_KEY_USER_LINE, connection->user_line);
	}
	
	if (connection->password_line) {
		g_key_file_set_string (keys, SSH_CONFIG_GROUP,
				       SSH_KEY_PASSWORD_LINE, connection->password_line);
	}
	
	if (connection->options_line) {
		g_key_file_set_string (keys, SSH_CONFIG_GROUP,
				       SSH_KEY_OPTIONS_LINE, connection->options_line);
	}
	
	if (connection->remote_line) {
		g_key_file_set_string (keys, SSH_CONFIG_GROUP,
				       SSH_KEY_REMOTE_LINE, connection->remote_line);
	}
	
	g_key_file_set_boolean (keys, SSH_CONFIG_GROUP,
				SSH_KEY_USE_X11, connection->use_x11);
	
	g_key_file_set_boolean (keys, SSH_CONFIG_GROUP,
				SSH_KEY_USE_TERMINAL, connection->use_terminal);
}

static void
tsc_ssh_connection_restore (TSCConnection *con, GKeyFile *keys)
{
	TSCSshConnection *connection = TSC_SSH_CONNECTION (con);
	connection->host_line = g_key_file_get_string (keys, SSH_CONFIG_GROUP,
							  SSH_KEY_HOST_LINE, NULL);
	connection->user_line = g_key_file_get_string (keys, SSH_CONFIG_GROUP,
							  SSH_KEY_USER_LINE, NULL);
	connection->password_line = g_key_file_get_string (keys, SSH_CONFIG_GROUP,
							  SSH_KEY_PASSWORD_LINE, NULL);
	connection->options_line = g_key_file_get_string (keys, SSH_CONFIG_GROUP,
							  SSH_KEY_OPTIONS_LINE, NULL);
	connection->remote_line = g_key_file_get_string (keys, SSH_CONFIG_GROUP,
							  SSH_KEY_REMOTE_LINE, NULL);
	connection->use_x11 = g_key_file_get_boolean (keys, SSH_CONFIG_GROUP,
							   SSH_KEY_USE_X11, NULL);
	connection->use_terminal = g_key_file_get_boolean (keys, SSH_CONFIG_GROUP,
							   SSH_KEY_USE_TERMINAL, NULL);
}

static void
tsc_ssh_connection_finalize (GObject *obj)
{
	TSCSshConnection *connection = TSC_SSH_CONNECTION (obj);

	g_free (connection->host_line);
	g_free (connection->user_line);
	g_free (connection->password_line);
	g_free (connection->options_line);
	g_free (connection->remote_line);

	if (parent_class->finalize) {
		parent_class->finalize (obj);
	}
}

static void
tsc_ssh_connection_class_init (TSCSshConnectionClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = tsc_ssh_connection_finalize;
	
	TSC_CONNECTION_CLASS (klass)->start = tsc_ssh_connection_start;
	TSC_CONNECTION_CLASS (klass)->save_config = tsc_ssh_connection_save;
	TSC_CONNECTION_CLASS (klass)->restore_config = tsc_ssh_connection_restore;
}

static void
tsc_ssh_connection_init (TSCSshConnection *connection)
{
}

GType
tsc_ssh_connection_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCSshConnectionClass),
			NULL, NULL,
			(GClassInitFunc) tsc_ssh_connection_class_init,
			NULL, NULL,
			sizeof (TSCSshConnection),
			0,
			(GInstanceInitFunc) tsc_ssh_connection_init
		};
		
		type = g_type_register_static (TSC_TYPE_CONNECTION,
					       "TSCSshConnection",
					       &type_info, 0);
	}
	
	return type;

}
