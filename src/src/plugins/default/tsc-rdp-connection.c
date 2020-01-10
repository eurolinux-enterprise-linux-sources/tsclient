
#include <config.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <sys/wait.h>
#include <glib.h>
#include <libnotify/notify.h>
#include "tsc-rdp-connection.h"
#include "tsc-util.h"
#include "tsc-connection.h"
#include "tsc-util.h"

#define RDP_CONFIG_GROUP "RDP"
#define RDP_KEY_HOST "host"
#define RDP_KEY_USERNAME "username"
#define RDP_KEY_PASSWORD "password"
#define RDP_KEY_DOMAIN "domain"
#define RDP_KEY_SOUND "sound"
#define RDP_KEY_SHELL "shell"
#define RDP_KEY_INITIAL_DIR "initial_directory"
#define RDP_KEY_WIDTH "width"
#define RDP_KEY_HEIGHT "height"
#define RDP_KEY_BPP "bitdepth"
#define RDP_KEY_FULLSCREEN "fullscreen"
#define RDP_KEY_ENCRYPTION "encryption"
#define RDP_KEY_COMPRESSION "compression"
#define RDP_KEY_EXPERIENCE "experience"

#define RDP_FULLSCREEN_NOTIFY_DELAY 5000

static GObjectClass *parent_class;

static gboolean
notify_fullscreen_cb (TSCRdpConnection *connection)
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
	body = g_strdup_printf (_("You are connected to '%s' in fullscreen mode.  Use Control+Alt+Enter to switch to and from windowed mode."), tsc_connection_get_name (TSC_CONNECTION (connection)));
	
	n = notify_notification_new (summary, body, GTK_STOCK_DIALOG_INFO, NULL);
	g_free (summary);
	g_free (body);
	
	notify_notification_show (n, NULL);
	g_object_unref (n);

	return FALSE;
}

static void
notify_fullscreen (TSCRdpConnection *connection)
{
	g_timeout_add (RDP_FULLSCREEN_NOTIFY_DELAY,
		       (GSourceFunc) notify_fullscreen_cb,
		       connection);
}

static const char *
tsc_rdp_experience_serialize (TSCRdpExperience experience)
{
	switch (experience) {
	case TSC_RDP_EXPERIENCE_MODEM:
		return "modem";
	case TSC_RDP_EXPERIENCE_BROADBAND:
		return "broadband";
	case TSC_RDP_EXPERIENCE_LAN:
		return "lan";
	case TSC_RDP_EXPERIENCE_DEFAULT:
	default:
		return NULL;
	}
}

static TSCRdpExperience
tsc_rdp_experience_unserialize (const char *experience)
{
	if (experience == NULL) {
		return TSC_RDP_EXPERIENCE_DEFAULT;
	} else if (g_ascii_strcasecmp (experience, "lan") == 0) {
		return TSC_RDP_EXPERIENCE_LAN;
	} else if (g_ascii_strcasecmp (experience, "broadband") == 0) {
		return TSC_RDP_EXPERIENCE_LAN;
	} else if (g_ascii_strcasecmp (experience, "modem") == 0) {
		return TSC_RDP_EXPERIENCE_MODEM;
	} else {
		return TSC_RDP_EXPERIENCE_DEFAULT;
	}
}

static void
tsc_rdp_connection_stopped_cb (TSCSpawn *spawn, TSCRdpConnection *connection)
{
	GError *error = NULL;
	int status;

	status = tsc_spawn_get_status (spawn);
	if (WIFEXITED (status) && WEXITSTATUS (status) == 2) {
		/* rdesktop returns status 2 if you just close the window
		 * which I do not consider an error
		 */
		g_signal_emit_by_name (connection, "ended", error, NULL);
	} else {
		error = tsc_spawn_get_error (spawn);
		g_signal_emit_by_name (connection, "ended", error, NULL);
	}
	
	g_object_unref (connection->spawn);
	connection->spawn = NULL;
	
	if (error) {
		g_error_free (error);
	}
}

static gboolean
tsc_rdp_should_cache_bitmaps (void)
{
	int result;
	struct statvfs sbuf;
	unsigned long long free_bytes;
	
	result = statvfs (g_getenv ("HOME"), &sbuf);
	if (result < 0) {
		g_warning ("Failed to stat '%s': %s", g_getenv ("HOME"),
			   g_strerror (errno));
		return FALSE;
	}

	/* require 100M of free space, caching could use up to 30M */
	free_bytes = ((unsigned long long) sbuf.f_bsize * (unsigned long long) sbuf.f_bavail);
	return free_bytes > (unsigned long long) (100 * 1024 * 1024);
}

static gboolean
tsc_rdp_connection_start (TSCConnection *con, GError **error)
{
	TSCRdpConnection *connection = TSC_RDP_CONNECTION (con);

	if (connection->spawn) {
		g_object_unref (connection->spawn);
	}

	connection->spawn = tsc_spawn_new ();
	g_signal_connect (connection->spawn, "stopped",
			  G_CALLBACK (tsc_rdp_connection_stopped_cb),
			  connection);

	tsc_spawn_append_args (connection->spawn, "rdesktop", NULL);

	if (connection->username) {
		tsc_spawn_append_args (connection->spawn, "-u", connection->username, NULL);
	}

	if (connection->password) {
		/* we will send password via stdin */
		tsc_spawn_append_args (connection->spawn, "-p", "-", NULL);
	}

	if (connection->domain) {
		tsc_spawn_append_args (connection->spawn, "-d", connection->domain, NULL);
	}

	if (connection->sound) {
		tsc_spawn_append_args (connection->spawn, "-r", "sound", NULL);
	}

	if (connection->shell) {
		tsc_spawn_append_args (connection->spawn, "-s", connection->shell, NULL);
	}

	if (connection->initial_directory) {
		tsc_spawn_append_args (connection->spawn, "-c", connection->initial_directory, NULL);
	}

	if (connection->width > 0 && connection->height > 0) {
		char *geometry = g_strdup_printf ("%dx%d", connection->width, connection->height);
		tsc_spawn_append_args (connection->spawn, "-g", geometry, NULL);
		g_free (geometry);
	}

	if (connection->fullscreen) {
		tsc_spawn_append_args (connection->spawn, "-f", NULL);
	}

	if (!connection->encryption) {
		tsc_spawn_append_args (connection->spawn, "-e", NULL);
	}

	if (connection->bpp > 0) {
		char *bpp_str = g_strdup_printf ("%d", connection->bpp);
		tsc_spawn_append_args (connection->spawn, "-a", bpp_str, NULL);
		g_free (bpp_str);
	}

	if (connection->compression) {
		tsc_spawn_append_args (connection->spawn, "-z", NULL);
	}

	if (connection->experience != TSC_RDP_EXPERIENCE_DEFAULT) {
		switch (connection->experience) {
		case TSC_RDP_EXPERIENCE_MODEM:
			tsc_spawn_append_args (connection->spawn, "-x", "m", NULL);
			break;
		case TSC_RDP_EXPERIENCE_BROADBAND:
			tsc_spawn_append_args (connection->spawn, "-x", "b", NULL);
			break;
		case TSC_RDP_EXPERIENCE_LAN:
			tsc_spawn_append_args (connection->spawn, "-x", "l", NULL);
			break;
		default:
			break;
		}
	}

	if (tsc_rdp_should_cache_bitmaps ()) {
		tsc_spawn_append_args (connection->spawn, "-P", NULL);
	}

	/* sets the window title to the connection name */
	tsc_spawn_append_args (connection->spawn, "-T",
			       tsc_connection_get_name (TSC_CONNECTION (connection)),
			       NULL);
	tsc_spawn_append_args (connection->spawn, connection->host, NULL);

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

	/* FIXME: should only do this if we know for sure that rdesktop came up (libsn or something) */
	if (connection->fullscreen) {
		notify_fullscreen (connection);
	}

	return TRUE;
}

static void
tsc_rdp_connection_save (TSCConnection *con, GKeyFile *keys)
{
	TSCRdpConnection *connection = TSC_RDP_CONNECTION (con);
	
	if (connection->host) {
		g_key_file_set_string (keys, RDP_CONFIG_GROUP,
				       RDP_KEY_HOST, connection->host);
	}

	if (connection->username) {
		g_key_file_set_string (keys, RDP_CONFIG_GROUP,
				       RDP_KEY_USERNAME, connection->username);
	}

	if (connection->password) {
		g_key_file_set_string (keys, RDP_CONFIG_GROUP,
				       RDP_KEY_PASSWORD, connection->password);
	}
	
	if (connection->domain) {
		g_key_file_set_string (keys, RDP_CONFIG_GROUP,
				       RDP_KEY_DOMAIN, connection->domain);
	}

	g_key_file_set_boolean (keys, RDP_CONFIG_GROUP,
				RDP_KEY_SOUND, connection->sound);

	if (connection->shell) {
		g_key_file_set_string (keys, RDP_CONFIG_GROUP,
				       RDP_KEY_SHELL, connection->shell);
	}

	if (connection->initial_directory) {
		g_key_file_set_string (keys, RDP_CONFIG_GROUP,
				       RDP_KEY_INITIAL_DIR,
				       connection->initial_directory);
	}

	if (connection->width > 0) {
		g_key_file_set_integer (keys, RDP_CONFIG_GROUP,
					RDP_KEY_WIDTH,
					connection->width);
	}

	if (connection->height > 0) {
		g_key_file_set_integer (keys, RDP_CONFIG_GROUP,
					RDP_KEY_HEIGHT,
					connection->height);
	}

	if (connection->bpp > 0) {
		g_key_file_set_integer (keys, RDP_CONFIG_GROUP,
					RDP_KEY_BPP,
					connection->bpp);
	}

	g_key_file_set_boolean (keys, RDP_CONFIG_GROUP,
				RDP_KEY_FULLSCREEN,
				connection->fullscreen);

	g_key_file_set_boolean (keys, RDP_CONFIG_GROUP,
				RDP_KEY_ENCRYPTION,
				connection->encryption);

	g_key_file_set_boolean (keys, RDP_CONFIG_GROUP,
				RDP_KEY_COMPRESSION,
				connection->compression);

	if (connection->experience != TSC_RDP_EXPERIENCE_DEFAULT) {
		const char *experience = tsc_rdp_experience_serialize (connection->experience);

		g_key_file_set_string (keys, RDP_CONFIG_GROUP,
				       RDP_KEY_EXPERIENCE,
				       experience);
	}
}

static void
get_boolean_value (GKeyFile *keys, const char *key, gboolean *dest)
{
	if (g_key_file_has_key (keys, RDP_CONFIG_GROUP, key, NULL)) {
		*dest = g_key_file_get_boolean (keys, RDP_CONFIG_GROUP,
						key, NULL);
	}
}

static void
tsc_rdp_connection_restore (TSCConnection *con, GKeyFile *keys)
{
	char *experience;
	TSCRdpConnection *connection = TSC_RDP_CONNECTION (con);
	
	connection->host = g_key_file_get_string (keys, RDP_CONFIG_GROUP,
						  RDP_KEY_HOST, NULL);
	connection->username = g_key_file_get_string (keys, RDP_CONFIG_GROUP,
						  RDP_KEY_USERNAME, NULL);
	connection->password = g_key_file_get_string (keys, RDP_CONFIG_GROUP,
						      RDP_KEY_PASSWORD, NULL);

	connection->domain = g_key_file_get_string (keys, RDP_CONFIG_GROUP,
						  RDP_KEY_DOMAIN, NULL);
	get_boolean_value (keys, RDP_KEY_SOUND,
			   &connection->sound);
	connection->shell = g_key_file_get_string (keys, RDP_CONFIG_GROUP,
						  RDP_KEY_SHELL, NULL);
	connection->initial_directory = g_key_file_get_string (keys, RDP_CONFIG_GROUP,
						  RDP_KEY_INITIAL_DIR, NULL);
	connection->width = g_key_file_get_integer (keys, RDP_CONFIG_GROUP,
						    RDP_KEY_WIDTH, NULL);
	connection->height = g_key_file_get_integer (keys, RDP_CONFIG_GROUP,
						    RDP_KEY_HEIGHT, NULL);
	connection->bpp = g_key_file_get_integer (keys, RDP_CONFIG_GROUP,
						    RDP_KEY_BPP, NULL);

	get_boolean_value (keys, RDP_KEY_FULLSCREEN,
			   &connection->fullscreen);
	get_boolean_value (keys, RDP_KEY_ENCRYPTION,
			   &connection->encryption);
	get_boolean_value (keys, RDP_KEY_COMPRESSION,
			   &connection->compression);

	experience = g_key_file_get_string (keys, RDP_CONFIG_GROUP,
					    RDP_KEY_EXPERIENCE, NULL);
	connection->experience = tsc_rdp_experience_unserialize (experience);
	g_free (experience);
}

static void
tsc_rdp_connection_finalize (GObject *obj)
{
	TSCRdpConnection *connection = TSC_RDP_CONNECTION (obj);

	g_free (connection->host);
	g_free (connection->username);
	g_free (connection->password);
	g_free (connection->domain);
	g_free (connection->shell);
	g_free (connection->initial_directory);
	
	if (parent_class->finalize) {
		parent_class->finalize (obj);
	}
}

static void
tsc_rdp_connection_class_init (TSCRdpConnectionClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = tsc_rdp_connection_finalize;
	
	TSC_CONNECTION_CLASS (klass)->start = tsc_rdp_connection_start;
	TSC_CONNECTION_CLASS (klass)->save_config = tsc_rdp_connection_save;
	TSC_CONNECTION_CLASS (klass)->restore_config = tsc_rdp_connection_restore;
}

static void
tsc_rdp_connection_init (TSCRdpConnection *connection)
{
	connection->encryption = TRUE;
	connection->fullscreen = TRUE;
}

GType
tsc_rdp_connection_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCRdpConnectionClass),
			NULL, NULL,
			(GClassInitFunc) tsc_rdp_connection_class_init,
			NULL, NULL,
			sizeof (TSCRdpConnection),
			0,
			(GInstanceInitFunc) tsc_rdp_connection_init
		};
		
		type = g_type_register_static (TSC_TYPE_CONNECTION,
					       "TSCRdpConnection",
					       &type_info, 0);
	}
	
	return type;

}
