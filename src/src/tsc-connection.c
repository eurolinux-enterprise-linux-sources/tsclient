
#include <config.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <string.h>
#include <libgnome/gnome-desktop-item.h>
#include "tsc-util.h"
#include "tsc-connection.h"
#include "tsc-provider.h"
#include "tsc-network.h"
#include "tsc-marshal.h"

#if ! GLIB_CHECK_VERSION (2, 12, 0)
#	include "tsc-bookmarkfile.h"
#endif

#define CONNECTION_KEY_NAME "name"
#define CONNECTION_KEY_RECON_POLICY "reconnect_policy"
#define CONNECTION_KEY_AUTOSTART "autostart"
#define CONNECTION_KEY_FAVORITE "favorite"

enum {
	CONNECTION_STARTED,
	CONNECTION_ENDED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };
static GObjectClass *parent_class;

static void
tsc_connection_finalize (GObject *obj)
{
	TSCConnection *connection = TSC_CONNECTION (obj);

	g_free (connection->name);
	g_free (connection->filename);
	
	if (parent_class->finalize) {
		parent_class->finalize (obj);
	}
}

static void
tsc_connection_started (TSCConnection *connection)
{
	connection->connected = TRUE;
}

static void
tsc_connection_ended (TSCConnection *connection, GError *error)
{
	connection->connected = FALSE;
}

static void
tsc_connection_class_init (TSCConnectionClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = tsc_connection_finalize;

	signals[CONNECTION_STARTED] =
		g_signal_new ("started",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (TSCConnectionClass,
					       started),
			      NULL, NULL,
			      tsc_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);

	signals[CONNECTION_ENDED] =
		g_signal_new ("ended",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (TSCConnectionClass,
					       ended),
			      NULL, NULL,
			      tsc_marshal_VOID__POINTER,
			      G_TYPE_NONE, 1, G_TYPE_POINTER);

	klass->started = tsc_connection_started;
	klass->ended = tsc_connection_ended;
}

static void
tsc_connection_init (TSCConnection *connection)
{
	tsc_connection_set_name (connection, _("New Connection"));
	tsc_connection_set_reconnect_policy (connection,
					     TSC_RECONNECT_PROMPT_ERROR);
}

GType
tsc_connection_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCConnectionClass),
			NULL, NULL,
			(GClassInitFunc) tsc_connection_class_init,
			NULL, NULL,
			sizeof (TSCConnection),
			0,
			(GInstanceInitFunc) tsc_connection_init
		};
		
		type = g_type_register_static (G_TYPE_OBJECT,
					       "TSCConnection",
					       &type_info,
					       G_TYPE_FLAG_ABSTRACT);
	}
	
	return type;

}

const char *
tsc_connection_get_filename (TSCConnection *connection)
{
	g_return_val_if_fail (connection != NULL, NULL);

	return connection->filename;
}

void
tsc_connection_set_filename (TSCConnection *connection, const char *fname)
{
	g_return_if_fail (connection != NULL);

	g_free (connection->filename);
	connection->filename = g_strdup (fname);
		
}

const char *
tsc_connection_get_name (TSCConnection *connection)
{
	g_return_val_if_fail (connection != NULL, NULL);

	return connection->name;
}

void
tsc_connection_set_name (TSCConnection *connection, const char *name)
{
	g_return_if_fail (connection != NULL);
	g_return_if_fail (name != NULL);

	g_free (connection->name);
	connection->name = g_strdup (name);
}

gboolean
tsc_connection_start (TSCConnection *connection, GError **error)
{
	TSCConnectionClass *class;
	GError *my_error = NULL;

	g_return_val_if_fail (connection != NULL, FALSE);

	if (tsc_connection_is_connected (connection)) {
		return TRUE;
	}

	if (!tsc_network_wait ()) {
		g_set_error (&my_error, TSC_ERROR, TSC_ERROR, _("Network connection is not available."));
	}

	class = TSC_CONNECTION_GET_CLASS (connection);
	if (class->start == NULL) {
		g_set_error (&my_error, TSC_ERROR, TSC_ERROR, "Start method not implemented");
	}

	if (my_error != NULL) {
		g_signal_emit (connection, signals[CONNECTION_ENDED], 0, my_error, NULL);

		if (error) {
			*error = my_error;
		} else {
			g_error_free (my_error);
		}
		return FALSE;
	}

	return class->start (connection, error);
}

static const char *
tsc_reconnect_policy_serialize (TSCReconnectPolicy policy)
{
	switch (policy) {
	case TSC_RECONNECT_NEVER:
		return "never";
	case TSC_RECONNECT_ALWAYS:
		return "always";
	case TSC_RECONNECT_ERROR:
		return "error";
	case TSC_RECONNECT_PROMPT_ERROR:
	default:
		return "prompt";
	}
}

static TSCReconnectPolicy
tsc_reconnect_policy_unserialize (const char *policy)
{
	if (policy == NULL) {
		return TSC_RECONNECT_PROMPT_ERROR;
	} else if (g_ascii_strcasecmp (policy, "never") == 0) {
		return TSC_RECONNECT_NEVER;
	} else if (g_ascii_strcasecmp (policy, "always") == 0) {
		return TSC_RECONNECT_ALWAYS;
	} else if (g_ascii_strcasecmp (policy, "prompt") == 0) {
		return TSC_RECONNECT_PROMPT_ERROR;
	} else if (g_ascii_strcasecmp (policy, "error") == 0) {
		return TSC_RECONNECT_ERROR;
	} else {
		g_warning ("Unrecognized reconnect policy: %s", policy);
		return TSC_RECONNECT_PROMPT_ERROR;
	}
}

void
tsc_connection_save_config (TSCConnection *connection, GKeyFile *keys)
{
	TSCConnectionClass *class;

	g_return_if_fail (connection != NULL);
	g_return_if_fail (keys != NULL);

	if (connection->name) {
		g_key_file_set_string (keys, GENERAL_CONFIG_GROUP,
				       CONNECTION_KEY_NAME,
				       tsc_connection_get_name (connection));
	}

	g_key_file_set_string (keys, GENERAL_CONFIG_GROUP,
			       CONNECTION_KEY_RECON_POLICY,
			       tsc_reconnect_policy_serialize (connection->reconnect_policy));

	g_key_file_set_boolean (keys, GENERAL_CONFIG_GROUP,
				CONNECTION_KEY_AUTOSTART,
				connection->autostart);

	g_key_file_set_boolean (keys, GENERAL_CONFIG_GROUP,
				CONNECTION_KEY_FAVORITE,
				connection->favorite);

	tsc_connection_write_favorite (connection, connection->favorite);

	class = TSC_CONNECTION_GET_CLASS (connection);
	if (class->save_config) {
		class->save_config (connection, keys);
	}
}

void
tsc_connection_restore_config (TSCConnection *connection, GKeyFile *keys)
{
	TSCConnectionClass *class;
	char *strval;
	
	g_return_if_fail (connection != NULL);
	g_return_if_fail (keys != NULL);

	connection->name = g_key_file_get_string (keys, GENERAL_CONFIG_GROUP,
						  CONNECTION_KEY_NAME, NULL);

	strval = g_key_file_get_string (keys, GENERAL_CONFIG_GROUP,
					CONNECTION_KEY_RECON_POLICY, NULL);
	connection->reconnect_policy = tsc_reconnect_policy_unserialize (strval);
	g_free (strval);

	connection->autostart = g_key_file_get_boolean (keys, GENERAL_CONFIG_GROUP,
							CONNECTION_KEY_AUTOSTART,
							NULL);
	
	connection->favorite = g_key_file_get_boolean (keys, GENERAL_CONFIG_GROUP,
							CONNECTION_KEY_FAVORITE,
							NULL);
	
	class = TSC_CONNECTION_GET_CLASS (connection);
	if (class->restore_config) {
		class->restore_config (connection, keys);
	}
}

gboolean
tsc_connection_is_connected (TSCConnection *connection)
{
	g_return_val_if_fail (connection != NULL, FALSE);
	return connection->connected;
}

struct _TSCProvider *
tsc_connection_get_provider (TSCConnection *connection)
{
	g_return_val_if_fail (connection != NULL, NULL);

	return connection->provider;
}

void
tsc_connection_set_reconnect_policy (TSCConnection *connection, TSCReconnectPolicy policy)
{
	g_return_if_fail (connection != NULL);

	connection->reconnect_policy = policy;
}

TSCReconnectPolicy
tsc_connection_get_reconnect_policy (TSCConnection *connection)
{
	g_return_val_if_fail (connection != NULL, TSC_RECONNECT_NEVER);

	return connection->reconnect_policy;
}

gboolean
tsc_connection_get_autostart (TSCConnection *connection)
{
	g_return_val_if_fail (connection != NULL, FALSE);

	return connection->autostart;
}

void
tsc_connection_set_autostart (TSCConnection *connection,
			      gboolean       autostart)
{
	g_return_if_fail (connection != NULL);
	connection->autostart = autostart;
}

gboolean
tsc_connection_get_favorite (TSCConnection *connection)
{
	g_return_val_if_fail (connection != NULL, FALSE);

	return connection->favorite;
}

void
tsc_connection_set_favorite (TSCConnection *connection, gboolean favorite)
{
	g_return_if_fail (connection != NULL);
	connection->favorite = favorite;
}

typedef struct {
	TSCConnection *connection;
	GPid pid;
	GIOChannel *stderr;
	GString *errors;

	guint error_watch;
	guint child_watch;

	TSCWatchCallback callback;
} ProcessWatchData;

static gboolean
child_error_cb (GIOChannel *channel, GIOCondition condition, gpointer user_data)
{
	ProcessWatchData *data = user_data;
	GIOStatus status;
	char *line;
	
	if (condition & G_IO_IN) {
		status = g_io_channel_read_line (channel, &line, NULL, NULL, NULL);
		if (status == G_IO_STATUS_NORMAL) {
			data->errors = g_string_append (data->errors, line);
			g_free (line);
		}
	}

	if (condition & G_IO_HUP) {
		return FALSE;
	}

	return TRUE;
}

static void
child_watch_cb (GPid pid, int status, gpointer user_data)
{
	ProcessWatchData *data = user_data;
	char *errors;

	g_source_remove (data->error_watch);
	errors = g_string_free (data->errors, FALSE);
	g_io_channel_unref (data->stderr);

	data->callback (data->connection, data->pid, status, errors);
	g_object_unref (data->connection);
	g_free (errors);
	g_free (data);
}

void
tsc_connection_watch_process (TSCConnection *  connection,
			      GPid             pid,
			      int              child_stderr,
			      TSCWatchCallback callback)
{
	ProcessWatchData *data;
	
	g_return_if_fail (connection != NULL);
	g_return_if_fail (callback != NULL);
	
	data = g_new0 (ProcessWatchData, 1);
	data->connection = TSC_CONNECTION (g_object_ref (connection));
	data->pid = pid;
	data->errors = g_string_new (NULL);
	data->callback = callback;

	data->stderr = g_io_channel_unix_new (child_stderr);
	data->error_watch = g_io_add_watch (data->stderr,
					    G_IO_IN | G_IO_HUP | G_IO_ERR,
					    child_error_cb, data);

	g_child_watch_add (pid, child_watch_cb, data);
}

gboolean
tsc_connection_can_save (TSCConnection *connection)
{
	TSCConnectionClass *class;

	g_return_val_if_fail (connection != NULL, FALSE);

	class = TSC_CONNECTION_GET_CLASS (connection);
	return class->save_config != NULL;
}

gboolean
tsc_connection_can_restore (TSCConnection *connection)
{
	TSCConnectionClass *class;

	g_return_val_if_fail (connection != NULL, FALSE);

	class = TSC_CONNECTION_GET_CLASS (connection);
	return class->restore_config != NULL;
}

gboolean
tsc_connection_create_shortcut (TSCConnection *connection, const gchar *location)
{
	GnomeDesktopItem *ditem;
	gchar *exec_val = g_strdup_printf ("tsclient -s \"%s\"", connection->filename);
	GError *error = NULL;
	gboolean ret;
	
	ditem = gnome_desktop_item_new ();
	gnome_desktop_item_set_entry_type (ditem, GNOME_DESKTOP_ITEM_TYPE_APPLICATION);
	gnome_desktop_item_set_string (ditem, GNOME_DESKTOP_ITEM_ENCODING, "UTF-8");
	gnome_desktop_item_set_string (ditem, GNOME_DESKTOP_ITEM_NAME, connection->name);
	gnome_desktop_item_set_string (ditem, GNOME_DESKTOP_ITEM_EXEC, exec_val);
	gnome_desktop_item_set_string (ditem, GNOME_DESKTOP_ITEM_ICON,
								   tsc_provider_get_icon_name (connection->provider));

	g_free (exec_val);
	ret = gnome_desktop_item_save (ditem, location, FALSE, &error);
	if (error) {
		g_printerr ("%s\n", error->message);
		g_error_free (error);
	}

	gnome_desktop_item_unref (ditem);
	return ret;
}

void
tsc_connection_write_favorite (TSCConnection *connection, gboolean add)
{
	const gchar * const *data_dirs = g_get_system_data_dirs ();
	const gchar *home_dir = g_get_home_dir ();
	const gchar *cname;
	gchar *uri, *local_places, *dir, *mime_type = "application/x-tsclient";
	TSCProvider *provider = tsc_connection_get_provider (connection);
	GBookmarkFile *bf;
	int i;

	cname = tsc_connection_get_filename (connection);
	if (!cname) {
		g_printerr ("Filename is not set for %s\n", tsc_connection_get_name (connection));
		return;
	}

	bf = g_bookmark_file_new ();
	uri = g_strconcat ("file://~", cname + strlen (home_dir), NULL);
	local_places = g_build_filename (g_get_user_data_dir (), "gnome-main-menu", "places.xbel", NULL);
	
	if (g_file_test (local_places, G_FILE_TEST_EXISTS)) {
		g_bookmark_file_load_from_file (bf, local_places, NULL);
	} else {
		for (i = 0; data_dirs && data_dirs[i]; i++) {
			gchar *system_places = g_build_filename (data_dirs[i], "gnome-main-menu", "places.xbel", NULL);
			
			if (g_file_test (system_places, G_FILE_TEST_EXISTS)) {
				g_bookmark_file_load_from_file (bf, system_places, NULL);
				g_free (system_places);
				break;
			}
			
			g_free (system_places);
		}
	}
	
	if (add) {
		g_bookmark_file_set_title (bf, uri, tsc_connection_get_name (connection));
		g_bookmark_file_set_mime_type (bf, uri, mime_type);
		g_bookmark_file_set_icon (bf, uri, tsc_provider_get_icon_name (provider), "image/png");
		g_bookmark_file_add_application (bf, uri, "tsclient", "tsclient -s %u");
	} else {
		if (g_bookmark_file_has_item (bf, uri) && !g_bookmark_file_remove_item (bf, uri, NULL)) {
			g_printerr ("Failed to remove %s\n", uri);
		}
	}

	dir = g_path_get_dirname (local_places);
	g_mkdir_with_parents (dir, 0700);
	g_free (dir);
	
	if (!g_bookmark_file_to_file (bf, local_places, NULL)) {
		tsc_util_show_error (_("Failed to Save"), _("Failed to save places.xbel"));
	}

	g_free (uri);
	g_free (local_places);
	g_bookmark_file_free (bf);
}
