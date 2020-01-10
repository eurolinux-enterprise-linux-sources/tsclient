
#include <config.h>
#include "string.h"
#include <glib.h>
#include <glib/gi18n.h>
#include <libnotify/notify.h>
#include "tsc-vnc-connection.h"
#include "tsc-util.h"
#include "tsc-connection.h"

#define VNC_CONFIG_GROUP "VNC"
#define VNC_KEY_HOST "host"
#define VNC_KEY_PASSWORD "password"
#define VNC_KEY_SHARED "shared"
#define VNC_KEY_VIEWONLY "viewonly"
#define VNC_KEY_FULLSCREEN "fullscreen"
#define VNC_KEY_WIDTH "width"
#define VNC_KEY_HEIGHT "height"

#define VNC_FULLSCREEN_NOTIFY_DELAY 5000

static GObjectClass *parent_class;

static gboolean
notify_fullscreen_cb (TSCVncConnection *connection)
{
	NotifyNotification *n;
	char *summary;
	char *body;

	if (!tsc_connection_is_connected (TSC_CONNECTION (connection)))
		return FALSE;
	
	if (!notify_is_initted ()) {
		notify_init (PACKAGE);
	}

	summary = g_strdup_printf (_("Connected to '%s'"),
				   tsc_connection_get_name (TSC_CONNECTION (connection)));
	body = g_strdup_printf (_("You are connected to '%s' in fullscreen mode.  Use F8 to switch to and from windowed mode."), tsc_connection_get_name (TSC_CONNECTION (connection)));

	n = notify_notification_new (summary, body, GTK_STOCK_DIALOG_INFO, NULL);
	g_free (summary);
	g_free (body);
	
	notify_notification_show (n, NULL);
	g_object_unref (n);

	return FALSE;
}

static void
notify_fullscreen (TSCVncConnection *connection)
{
	g_timeout_add (VNC_FULLSCREEN_NOTIFY_DELAY,
		       (GSourceFunc) notify_fullscreen_cb,
		       connection);
}

static void
tsc_vnc_connection_stopped_cb (TSCSpawn *spawn, TSCVncConnection *connection)
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
tsc_vnc_connection_start (TSCConnection *con, GError **error)
{
	TSCVncConnection *connection = TSC_VNC_CONNECTION (con);

	if (connection->spawn) {
		g_object_unref (connection->spawn);
	}

	connection->spawn = tsc_spawn_new ();
	g_signal_connect (connection->spawn, "stopped",
			  G_CALLBACK (tsc_vnc_connection_stopped_cb),
			  connection);

	tsc_spawn_set_args (connection->spawn, "vncviewer", NULL);

	if (connection->shared) {
		tsc_spawn_append_args (connection->spawn, "-shared", NULL);
	} else {
		tsc_spawn_append_args (connection->spawn, "-noshared", NULL);
	}

	if (connection->viewonly) {
		tsc_spawn_append_args (connection->spawn, "-viewonly", NULL);
	}

	if (connection->fullscreen) {
		tsc_spawn_append_args (connection->spawn, "-fullscreen", NULL);
	} else {
		char *geometry = g_strdup_printf ("%dx%d", connection->width,
						  connection->height);

		tsc_spawn_append_args (connection->spawn, "-geometry", geometry, NULL);
		g_free (geometry);
	}

	if (connection->password) {
		tsc_spawn_append_args (connection->spawn, "-autopass", NULL);
	}

	if (connection->host) {
		tsc_spawn_append_args (connection->spawn, connection->host, NULL);
	}
	
	if (!tsc_spawn_start (connection->spawn, error)) {
		return FALSE;
	}

	if (connection->password) {
		char *password = g_strdup_printf ("%s\n", connection->password);

		g_io_channel_write_chars (tsc_spawn_get_stdin (connection->spawn), password,
					  strlen (password), NULL, NULL);
		g_io_channel_flush (tsc_spawn_get_stdin (connection->spawn), NULL);
		g_free (password);
	}

	g_signal_emit_by_name (connection, "started", NULL);

	if (connection->fullscreen) {
		notify_fullscreen (connection);
	}
	
	return TRUE;
}

static void
tsc_vnc_connection_save (TSCConnection *con, GKeyFile *keys)
{
	TSCVncConnection *connection = TSC_VNC_CONNECTION (con);

	if (connection->host) {
		g_key_file_set_string (keys, VNC_CONFIG_GROUP,
				       VNC_KEY_HOST, connection->host);
	}

	if (connection->password) {
		g_key_file_set_string (keys, VNC_CONFIG_GROUP,
				       VNC_KEY_PASSWORD, connection->password);
	}

	g_key_file_set_boolean (keys, VNC_CONFIG_GROUP,
				VNC_KEY_SHARED, connection->shared);
	g_key_file_set_boolean (keys, VNC_CONFIG_GROUP,
				VNC_KEY_VIEWONLY, connection->viewonly);
	g_key_file_set_boolean (keys, VNC_CONFIG_GROUP,
				VNC_KEY_FULLSCREEN, connection->fullscreen);

	if (connection->width > 0) {
		g_key_file_set_integer (keys, VNC_CONFIG_GROUP,
					VNC_KEY_WIDTH, connection->width);
	}

	if (connection->height > 0) {
		g_key_file_set_integer (keys, VNC_CONFIG_GROUP,
					VNC_KEY_HEIGHT, connection->height);
	}
}

static void
tsc_vnc_connection_restore (TSCConnection *con, GKeyFile *keys)
{
	TSCVncConnection *connection = TSC_VNC_CONNECTION (con);
	connection->host = g_key_file_get_string (keys, VNC_CONFIG_GROUP,
						  VNC_KEY_HOST, NULL);
	connection->password = g_key_file_get_string (keys, VNC_CONFIG_GROUP,
						      VNC_KEY_PASSWORD, NULL);

	connection->shared = g_key_file_get_boolean (keys, VNC_CONFIG_GROUP,
						     VNC_KEY_SHARED, NULL);
	connection->viewonly = g_key_file_get_boolean (keys, VNC_CONFIG_GROUP,
						       VNC_KEY_VIEWONLY, NULL);
	connection->fullscreen = g_key_file_get_boolean (keys, VNC_CONFIG_GROUP,
							 VNC_KEY_FULLSCREEN, NULL);

	connection->width = g_key_file_get_integer (keys, VNC_CONFIG_GROUP,
						    VNC_KEY_WIDTH, NULL);
	connection->height = g_key_file_get_integer (keys, VNC_CONFIG_GROUP,
						     VNC_KEY_HEIGHT, NULL);
}

static void
tsc_vnc_connection_finalize (GObject *obj)
{
	TSCVncConnection *connection = TSC_VNC_CONNECTION (obj);

	g_free (connection->host);
	g_free (connection->password);

	if (parent_class->finalize) {
		parent_class->finalize (obj);
	}
}

static void
tsc_vnc_connection_class_init (TSCVncConnectionClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = tsc_vnc_connection_finalize;
	
	TSC_CONNECTION_CLASS (klass)->start = tsc_vnc_connection_start;
	TSC_CONNECTION_CLASS (klass)->save_config = tsc_vnc_connection_save;
	TSC_CONNECTION_CLASS (klass)->restore_config = tsc_vnc_connection_restore;
}

static void
tsc_vnc_connection_init (TSCVncConnection *connection)
{
	connection->fullscreen = TRUE;
	connection->shared = TRUE;
}

GType
tsc_vnc_connection_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCVncConnectionClass),
			NULL, NULL,
			(GClassInitFunc) tsc_vnc_connection_class_init,
			NULL, NULL,
			sizeof (TSCVncConnection),
			0,
			(GInstanceInitFunc) tsc_vnc_connection_init
		};
		
		type = g_type_register_static (TSC_TYPE_CONNECTION,
					       "TSCVncConnection",
					       &type_info, 0);
	}
	
	return type;

}
