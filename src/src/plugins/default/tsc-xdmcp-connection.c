
#include <config.h>
#include <unistd.h>
#include <signal.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>
#include "tsc-xdmcp-connection.h"
#include "tsc-util.h"
#include "tsc-connection.h"
#include "tsc-spawn.h"

#define XDMCP_CONFIG_GROUP "XDMCP"
#define XDMCP_KEY_HOST "host"
#define XDMCP_KEY_FULLSCREEN "fullscreen"
#define XDMCP_KEY_WIDTH "width"
#define XDMCP_KEY_HEIGHT "height"

static GObjectClass *parent_class;

static void
tsc_xdmcp_connection_stopped_cb (TSCSpawn *spawn, TSCXdmcpConnection *connection)
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

static gchar *
find_open_x_display ()
{
	int i;
	struct passwd *pw;

	pw = getpwnam (g_get_user_name ());
	if (!pw) {
		g_printerr ("Failed to lookup user\n");
		return NULL;
	}
	
	for (i = 0; i < 128; i++) {
		gchar buf[512];

		/*
		 * Check for X lock files. If they exist see if the process
		 * is still running.
		 */
		g_snprintf (buf, 512, "%s/.X%d-lock", g_get_tmp_dir (), i);
		if (g_file_test (buf, G_FILE_TEST_EXISTS)) {
			FILE *fp;
			pid_t pid;

			fp = fopen (buf, "r");
			if (!fp) { 
				continue;
			}

			if (fscanf (fp, "%d", &pid) != 1) {
				fclose (fp);
				continue;
			}

			fclose (fp);

			if (!kill (pid, 0)) {
				continue;
			}
		}

		/*
		 * There isn't a valid lock file. Check for socket.  If it
		 * exists, but is owned by us then we can overwrite it.
		 * If it owned by someone else then we try the next one.
		 */
		g_snprintf (buf, 512, "%s/.X11-unix/X%d", g_get_tmp_dir (), i);
		if (g_file_test (buf, G_FILE_TEST_EXISTS)) {
			struct stat info;
			
			if (g_stat (buf, &info) == -1)
				continue;

			if (pw->pw_uid == info.st_uid)
				return g_strdup_printf (":%d", i);

		} else {
			return g_strdup_printf (":%d", i);
		}
	}

	return NULL;
}

static gboolean
tsc_xdmcp_connection_start (TSCConnection *con, GError **error)
{
	gchar *display, *geometry = NULL;
	TSCXdmcpConnection *connection = TSC_XDMCP_CONNECTION (con);

	if (connection->spawn) {
		g_object_unref (connection->spawn);
	}

	display = find_open_x_display ();
	if (!display) {
		g_printerr ("Failed to find xdisplay\n");
		return FALSE;
	}

	connection->spawn = tsc_spawn_new ();
	g_signal_connect (connection->spawn, "stopped",
			  G_CALLBACK (tsc_xdmcp_connection_stopped_cb),
			  connection);

	geometry = g_strdup_printf ("%dx%d", connection->width, connection->height);
	
	if (tsc_util_program_exists ("Xephyr")) {
		tsc_spawn_set_args (connection->spawn, "Xephyr", "-br", "-once", NULL);
		
		if (connection->fullscreen)
			tsc_spawn_append_args (connection->spawn, "-fullscreen", NULL);
		else
			tsc_spawn_append_args (connection->spawn, "-screen", geometry, NULL);
	} else {
		tsc_spawn_set_args (connection->spawn, "Xnest", "-br", "-name",
							tsc_connection_get_name (TSC_CONNECTION (connection)), NULL);
		tsc_spawn_append_args (connection->spawn, "-geometry", geometry, NULL);
	}

	tsc_spawn_append_args (connection->spawn, "-query", connection->host);
	tsc_spawn_append_args (connection->spawn, display, NULL);
	g_free (display);
	g_free (geometry);
	
	if (!tsc_spawn_start (connection->spawn, error)) {
		return FALSE;
	}

	g_signal_emit_by_name (connection, "started", NULL);
	
	return TRUE;
}

static void
tsc_xdmcp_connection_save (TSCConnection *con, GKeyFile *keys)
{
	TSCXdmcpConnection *connection = TSC_XDMCP_CONNECTION (con);
	g_key_file_set_string (keys, XDMCP_CONFIG_GROUP,
				   XDMCP_KEY_HOST, connection->host);
	g_key_file_set_boolean (keys, XDMCP_CONFIG_GROUP,
				XDMCP_KEY_FULLSCREEN, connection->fullscreen);
	g_key_file_set_integer (keys, XDMCP_CONFIG_GROUP,
				XDMCP_KEY_WIDTH, connection->width);
	g_key_file_set_integer (keys, XDMCP_CONFIG_GROUP,
				XDMCP_KEY_HEIGHT, connection->height);
}

static void
tsc_xdmcp_connection_restore (TSCConnection *con, GKeyFile *keys)
{
	TSCXdmcpConnection *connection = TSC_XDMCP_CONNECTION (con);
	connection->host = g_key_file_get_string (keys, XDMCP_CONFIG_GROUP,
							  XDMCP_KEY_HOST, NULL);
	connection->fullscreen = g_key_file_get_boolean (keys, XDMCP_CONFIG_GROUP,
							 XDMCP_KEY_FULLSCREEN, NULL);
	connection->width = g_key_file_get_integer (keys, XDMCP_CONFIG_GROUP,
						    XDMCP_KEY_WIDTH, NULL);
	connection->height = g_key_file_get_integer (keys, XDMCP_CONFIG_GROUP,
						    XDMCP_KEY_HEIGHT, NULL);
}

static void
tsc_xdmcp_connection_finalize (GObject *obj)
{
	TSCXdmcpConnection *connection = TSC_XDMCP_CONNECTION (obj);

	g_free (connection->host);

	if (parent_class->finalize) {
		parent_class->finalize (obj);
	}
}

static void
tsc_xdmcp_connection_class_init (TSCXdmcpConnectionClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = tsc_xdmcp_connection_finalize;
	
	TSC_CONNECTION_CLASS (klass)->start = tsc_xdmcp_connection_start;
	TSC_CONNECTION_CLASS (klass)->save_config = tsc_xdmcp_connection_save;
	TSC_CONNECTION_CLASS (klass)->restore_config = tsc_xdmcp_connection_restore;
}

static void
tsc_xdmcp_connection_init (TSCXdmcpConnection *connection)
{
}

GType
tsc_xdmcp_connection_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCXdmcpConnectionClass),
			NULL, NULL,
			(GClassInitFunc) tsc_xdmcp_connection_class_init,
			NULL, NULL,
			sizeof (TSCXdmcpConnection),
			0,
			(GInstanceInitFunc) tsc_xdmcp_connection_init
		};
		
		type = g_type_register_static (TSC_TYPE_CONNECTION,
					       "TSCXdmcpConnection",
					       &type_info, 0);
	}
	
	return type;

}
