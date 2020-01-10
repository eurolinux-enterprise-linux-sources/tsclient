
#include <config.h>
#include <string.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gconf/gconf-client.h>
#include <gconf/gconf-value.h>
#include "tsc-web-connection.h"
#include "tsc-util.h"
#include "tsc-connection.h"
#include "tsc-spawn.h"

#define WEB_CONFIG_GROUP "Web"
#define WEB_KEY_URL "url"
#define DEFAULT_BROWSER_KEY "/desktop/gnome/applications/browser/exec"

static GObjectClass *parent_class;

static void
tsc_web_connection_stopped_cb (TSCSpawn *spawn, TSCWebConnection *connection)
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
tsc_web_get_default_browser ()
{
	gchar *ret;
	GError *error = NULL;
	GConfClient *client = gconf_client_get_default ();
	GConfValue *value = gconf_client_get (client, DEFAULT_BROWSER_KEY, &error);
	if (error) {
		g_printerr ("Failed to get default browser from gconf: %s\n", error->message);
		g_error_free (error);
	}

	ret = value ? g_strdup (gconf_value_get_string (value)) : g_strdup ("firefox");
	gconf_value_free (value);

	g_object_unref (client);
	return ret;
}

static gboolean
tsc_web_connection_start (TSCConnection *con, GError **error)
{
	TSCWebConnection *connection = TSC_WEB_CONNECTION (con);
	char *browser;
	
	if (connection->spawn) {
		g_object_unref (connection->spawn);
	}

	connection->spawn = tsc_spawn_new ();
	g_signal_connect (connection->spawn, "stopped",
			  G_CALLBACK (tsc_web_connection_stopped_cb),
			  connection);

	browser = tsc_web_get_default_browser ();
	if (!tsc_util_program_exists (browser)) {
		g_set_error (error, TSC_ERROR, TSC_ERROR, _("Failed to find default browser"));
		return FALSE;
	}

	tsc_spawn_set_args (connection->spawn, browser, NULL);

	if (!g_ascii_strncasecmp (browser, "firefox", 7)) {
		gchar *fn = g_path_get_basename (tsc_connection_get_filename (TSC_CONNECTION (connection)));
		char *argv[] = { browser, "-no-remote", "-CreateProfile", fn, NULL};
	
		/* Creates a new firefox profile for this connection, if necessary. */
		g_spawn_sync (NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL, NULL, NULL);
		tsc_spawn_append_args (connection->spawn, "-no-remote", "-P", fn, NULL);
		g_free (fn);
	}

	tsc_spawn_append_args (connection->spawn, connection->url, NULL);
	
	if (!tsc_spawn_start (connection->spawn, error)) {
		return FALSE;
	}

	g_signal_emit_by_name (connection, "started", NULL);
	
	return TRUE;
}

static void
tsc_web_connection_save (TSCConnection *con, GKeyFile *keys)
{
	TSCWebConnection *connection = TSC_WEB_CONNECTION (con);
	if (connection->url) {
		g_key_file_set_string (keys, WEB_CONFIG_GROUP,
				       WEB_KEY_URL, connection->url);
	}
}

static void
tsc_web_connection_restore (TSCConnection *con, GKeyFile *keys)
{
	TSCWebConnection *connection = TSC_WEB_CONNECTION (con);
	
	connection->url = g_key_file_get_string (keys, WEB_CONFIG_GROUP,
							  WEB_KEY_URL, NULL);
}

void
tsc_web_connection_removed (TSCManager *manager, TSCConnection *con, gpointer user_data)
{
	TSCWebConnection *connection;
	gchar *profile_config, *profile_name;
	gchar **groups = NULL;
	GError *error = NULL;
	GKeyFile *kf;
	gsize len;
	int i;
	
	if (!TSC_IS_WEB_CONNECTION (con)) {
		return;
	}

	connection = TSC_WEB_CONNECTION (con);
	profile_name = g_path_get_basename (tsc_connection_get_filename (TSC_CONNECTION (connection)));
	profile_config = g_build_filename (g_get_home_dir (), ".mozilla", "firefox", "profiles.ini", NULL);

	kf = g_key_file_new ();
	if (!g_key_file_load_from_file (kf, profile_config,
		 G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &error)) {
		if (error) {
			g_printerr ("Failed to load firefox config: %s\n", error->message);
			g_error_free (error);
		} else {
			g_printerr ("Failed to load firefox config\n");
		}

		goto cleanup;
	}

	groups = g_key_file_get_groups (kf, &len);
	for (i = 0; i < len; i++) {
		gchar *name, *path, *full_path, *key_data;
		char *argv[] = { "rm", "-rf", NULL, NULL };
		gsize data_len;
		int rm_ret = 0;
		
		name = g_key_file_get_string (kf, groups[i], "Name", NULL);
		if (!name) {
			continue;
		}

		if (strcmp (profile_name, name)) {
			g_free (name);
			continue;
		}

		path = g_key_file_get_string (kf, groups[i], "Path", NULL);
		if (!path) {
			g_free (name);
			continue;
		}
	
		g_key_file_remove_group (kf, groups[i], &error);
		if (error) {
			g_printerr ("Unable to remove profile: %s\n", error->message);
			g_error_free (error);
		} 

		key_data = g_key_file_to_data (kf, &data_len, NULL);
		if (!g_file_set_contents (profile_config, key_data, data_len, NULL)) {
			g_printerr ("Failed to save profiles.ini\n");
			goto cleanup;
		}
		
		full_path = g_build_filename (g_get_home_dir (), ".mozilla", "firefox", path, NULL);
		argv[2] = full_path;
		g_spawn_sync (NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL, &rm_ret, NULL);

		if (rm_ret != 0) {
			g_printerr ("Failed to remove %s\n", full_path);
		}
		
		g_free (name);
		g_free (path);
		g_free (full_path);
		g_free (key_data);
		break;
	}

cleanup:
	g_free (profile_name);
	g_free (profile_config);
	g_strfreev (groups);
	g_key_file_free (kf);
}

static void
tsc_web_connection_finalize (GObject *obj)
{
	TSCWebConnection *connection = TSC_WEB_CONNECTION (obj);

	g_free (connection->url);

	if (parent_class->finalize) {
		parent_class->finalize (obj);
	}
}

static void
tsc_web_connection_class_init (TSCWebConnectionClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = tsc_web_connection_finalize;
	
	TSC_CONNECTION_CLASS (klass)->start = tsc_web_connection_start;
	TSC_CONNECTION_CLASS (klass)->save_config = tsc_web_connection_save;
	TSC_CONNECTION_CLASS (klass)->restore_config = tsc_web_connection_restore;
}

static void
tsc_web_connection_init (TSCWebConnection *connection)
{
}

GType
tsc_web_connection_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCWebConnectionClass),
			NULL, NULL,
			(GClassInitFunc) tsc_web_connection_class_init,
			NULL, NULL,
			sizeof (TSCWebConnection),
			0,
			(GInstanceInitFunc) tsc_web_connection_init
		};
		
		type = g_type_register_static (TSC_TYPE_CONNECTION,
					       "TSCWebConnection",
					       &type_info, 0);
	}
	
	return type;

}
