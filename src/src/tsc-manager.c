
#include <config.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include "tsc-manager.h"
#include "tsc-marshal.h"
#include "tsc-util.h"

#define PROVIDER_KEY "provider"

enum {
	MANAGER_STARTED,
	MANAGER_ENDED,
	MANAGER_IDLE,
	MANAGER_ADDED,
	MANAGER_REMOVED,
	MANAGER_SAVED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };
static GObjectClass *parent_class;

static void
tsc_manager_finalize (GObject *obj)
{
	GList *l;
	TSCManager *manager = TSC_MANAGER (obj);

	g_free (manager->connection_dir);
	
	for (l = manager->connections; l; l = l->next) {
		g_object_unref (l->data);
	}
	
	for (l = manager->providers; l; l = l->next) {
		g_object_unref (l->data);
	}

	g_list_free (manager->connections);
	g_list_free (manager->providers);
	
	if (parent_class->finalize) {
		parent_class->finalize (obj);
	}
}

static void
tsc_manager_class_init (TSCManagerClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = tsc_manager_finalize;

	signals[MANAGER_STARTED] =
		g_signal_new ("started",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (TSCManagerClass,
					       started),
			      NULL, NULL,
			      tsc_marshal_VOID__OBJECT,
			      G_TYPE_NONE, 1, G_TYPE_OBJECT);

	signals[MANAGER_ENDED] =
		g_signal_new ("ended",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (TSCManagerClass,
					       ended),
			      NULL, NULL,
			      tsc_marshal_VOID__OBJECT,
			      G_TYPE_NONE, 1, G_TYPE_OBJECT);

	signals[MANAGER_IDLE] =
		g_signal_new ("idle",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (TSCManagerClass,
					       idle),
			      NULL, NULL,
			      tsc_marshal_VOID__VOID,
			      G_TYPE_NONE, 0, NULL);

	signals[MANAGER_ADDED] =
		g_signal_new ("added",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (TSCManagerClass,
					       added),
			      NULL, NULL,
			      tsc_marshal_VOID__OBJECT,
			      G_TYPE_NONE, 1, G_TYPE_OBJECT);

	signals[MANAGER_REMOVED] =
		g_signal_new ("removed",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (TSCManagerClass,
					       removed),
			      NULL, NULL,
			      tsc_marshal_VOID__OBJECT,
			      G_TYPE_NONE, 1, G_TYPE_OBJECT);

	signals[MANAGER_SAVED] =
		g_signal_new ("saved",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (TSCManagerClass,
					       saved),
			      NULL, NULL,
			      tsc_marshal_VOID__OBJECT,
			      G_TYPE_NONE, 1, G_TYPE_OBJECT);
}

static void
tsc_manager_init (TSCManager *manager)
{
}

GType
tsc_manager_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCManagerClass),
			NULL, NULL,
			(GClassInitFunc) tsc_manager_class_init,
			NULL, NULL,
			sizeof (TSCManager),
			0,
			(GInstanceInitFunc) tsc_manager_init
		};
		
		type = g_type_register_static (G_TYPE_OBJECT,
					       "TSCManager",
					       &type_info, 0);
	}
	
	return type;

}

static void
tsc_manager_load_plugin (TSCManager *manager, const char *path)
{
		GModule *module;
		TSCPluginInitFunc init_func = NULL;
		
		module = g_module_open (path, G_MODULE_BIND_LAZY);
		if (!module) {
			g_warning ("Failed to load plugin %s", path);
			return;
		}

		if (!g_module_symbol (module, "tsc_init_plugin", (gpointer *)&init_func) || init_func == NULL) {
			g_warning ("Failed to initialize plugin '%s': no entry point", path);
			g_module_close (module);
			return;
		}

		(*init_func) (manager);
}

static void
tsc_manager_load_plugins (TSCManager *manager)
{
	GDir *plugin_dir;
	const char *plugin_name;
	char *plugin_path;

	plugin_dir = g_dir_open (PLUGINDIR, 0, NULL);
	if (!plugin_dir) {
		g_warning ("No plugins found");
		return;
	}

	while ((plugin_name = g_dir_read_name (plugin_dir)) != NULL) {
		if (g_str_has_suffix (plugin_name, "." G_MODULE_SUFFIX)) {
			plugin_path = g_build_filename (PLUGINDIR, plugin_name, NULL);
			tsc_manager_load_plugin (manager, plugin_path);
			g_free (plugin_path);
		}
	}

	g_dir_close (plugin_dir);
}

TSCConnection *
tsc_manager_load_connection (TSCManager *manager, const char *file, GError **error)
{
	TSCProvider *provider = NULL;
	TSCConnection *connection = NULL;
	GKeyFile *keys;
	GError *keyfile_error = NULL;
	char *strval = NULL;

	keys = g_key_file_new ();
	if (!g_key_file_load_from_file (keys, file, G_KEY_FILE_KEEP_COMMENTS, &keyfile_error)) {
		g_set_error (error, TSC_ERROR, TSC_ERROR,
			     _("Failed to load connection file '%s': %s"), file,
			     keyfile_error ? keyfile_error->message : _("Unknown Error."));
		goto cleanup;
	}

	strval = g_key_file_get_string (keys, GENERAL_CONFIG_GROUP,
					PROVIDER_KEY, NULL);
	if (!strval) {
		g_set_error (error, TSC_ERROR, TSC_ERROR,
			     _("Failed to load connection file '%s': No provider specified."),
			     file);
		goto cleanup;
	}

	provider = tsc_manager_lookup_provider (manager, strval);
	if (!provider) {
		g_set_error (error, TSC_ERROR, TSC_ERROR,
			     _("Failed to load connection file '%s': Provider '%s' is not available."),
			     file, strval);
		goto cleanup;
	}

	connection = tsc_provider_create_connection (provider);
	tsc_connection_set_filename (connection, file);
	tsc_connection_restore_config (connection, keys);
cleanup:
	g_free (strval);
	g_key_file_free (keys);
	return connection;
}

static void
tsc_manager_load_connections (TSCManager *manager)
{
	GDir *conn_dir;
	const char *connfile_name;
	char *connfile_path;
	TSCConnection *connection;
	GError *error;

	conn_dir = g_dir_open (manager->connection_dir, 0, NULL);
	if (!conn_dir) {
		return;
	}

	while ((connfile_name = g_dir_read_name (conn_dir)) != NULL) {
		if (!g_str_has_suffix (connfile_name, TSC_CONNECTION_SUFFIX))
			continue;
		
		connfile_path = g_build_filename (manager->connection_dir, connfile_name, NULL);

		error = NULL;
		connection = tsc_manager_load_connection (manager, connfile_path, &error);
		g_free (connfile_path);

		if (!connection) {
			if (error && error->message) {
				g_warning (error->message);
			}
			g_error_free (error);
		} else {
			tsc_manager_add_connection (manager, connection);
			g_object_unref (connection);
		}
	}

	g_dir_close (conn_dir);
}

TSCManager *
tsc_manager_new (void)
{
	TSCManager *manager;
	

	manager = g_object_new (TSC_TYPE_MANAGER, NULL);
	manager->connection_dir = g_build_filename (g_get_home_dir (), ".config",
						    "tsclient",
						    "connections", NULL);
	tsc_manager_load_plugins (manager);
	tsc_manager_load_connections (manager);

	return manager;
}

void
tsc_manager_register_provider (TSCManager *manager, TSCProvider *provider)
{
	g_return_if_fail (manager != NULL);
	g_return_if_fail (provider != NULL);

	if (g_list_find (manager->providers, provider)) {
		return;
	}

	manager->providers = g_list_append (manager->providers, provider);
}

void
tsc_manager_unregister_provider (TSCManager *manager, TSCProvider *provider)
{
	g_return_if_fail (manager != NULL);
	g_return_if_fail (provider != NULL);

	manager->providers = g_list_remove (manager->providers, provider);
}

const GList *
tsc_manager_get_providers (TSCManager *manager)
{
	g_return_val_if_fail (manager != NULL, NULL);
	
	return manager->providers;
}

TSCProvider *
tsc_manager_lookup_provider (TSCManager * manager, const char *name)
{
	GList *l;
	
	g_return_val_if_fail (manager != NULL, NULL);

	for (l = manager->providers; l; l = l->next) {
		if (strcmp (tsc_provider_get_name (TSC_PROVIDER (l->data)), name) == 0) {
			return TSC_PROVIDER (l->data);
		}
	}

	return NULL;
}

const GList *
tsc_manager_get_connections (TSCManager *manager)
{
	g_return_val_if_fail (manager != NULL, NULL);

	return manager->connections;
}

static void
set_connection_filename (TSCManager *manager, TSCConnection *connection)
{
	char *base, *tmp, *fname;
	int i, num = 0;

	tmp = g_build_filename (manager->connection_dir, tsc_connection_get_name (connection), NULL);
	base = g_utf8_strdown (tmp, -1);
	g_free (tmp);

	/* convert spaces to dashes */
	for (i = 0; i < strlen (base); i++) {
		if (base[i] == ' ') {
			base[i] = '-';
		}
	}

	fname = NULL;
	do {
		g_free (fname);
		
		if (num > 0) {
			fname = g_strdup_printf ("%s-%d%s", base, num,
						 TSC_CONNECTION_SUFFIX);
		} else {
			fname = g_strconcat (base, TSC_CONNECTION_SUFFIX, NULL);
		}

		num++;
	} while (g_file_test (fname, G_FILE_TEST_EXISTS));

	tsc_connection_set_filename (connection, fname);
	g_free (base);
	g_free (fname);
}

static void
tsc_manager_connection_started_cb (TSCConnection *connection,
				   TSCManager *manager)
{
	g_signal_emit (manager, signals[MANAGER_STARTED],
		       0, connection, NULL);
}

static gboolean
tsc_manager_prompt_reconnect (TSCManager *manager, TSCConnection *connection, const char *error_msg)
{
	GtkWidget *dialog;
	int response;
	
	dialog = gtk_message_dialog_new_with_markup (NULL, GTK_DIALOG_MODAL,
						     GTK_MESSAGE_QUESTION,
						     GTK_BUTTONS_YES_NO,
						     _("<span weight=\"bold\">Reconnect to '%s'?</span>\n\nYou have been unexpectedly disconnected from '%s'.  Would you like to reconnect?\n\n<i>%s</i>"),
						     tsc_connection_get_name (connection),
						     tsc_connection_get_name (connection),
						     error_msg);
	gtk_window_set_title (GTK_WINDOW (dialog), _("Terminal Server Client - Reconnect"));
	response = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	
	return response == GTK_RESPONSE_YES;
}

static void
tsc_manager_connection_ended_cb (TSCConnection *connection,
				 GError *error,
				 TSCManager *manager)
{
	TSCReconnectPolicy policy;
	gboolean reconnect = FALSE;

	policy = tsc_connection_get_reconnect_policy (connection);
	if (policy == TSC_RECONNECT_ALWAYS) {
		reconnect = TRUE;
	} else if (error) {
		if (connection->reconnect_policy == TSC_RECONNECT_ERROR) {
			reconnect = TRUE;
		} else if (connection->reconnect_policy == TSC_RECONNECT_PROMPT_ERROR) {
			reconnect = tsc_manager_prompt_reconnect (manager, connection, error->message);
		} else {
			char *error_summary = g_strdup_printf (_("Connection '%s' failed"),
							       tsc_connection_get_name (connection));
			tsc_util_show_error (error_summary, error->message);
			g_free (error_summary);
		}
	}

	if (reconnect) {
		tsc_connection_start (connection, NULL);
	} else {
		g_signal_emit (manager, signals[MANAGER_ENDED],
			       0, connection, NULL);

		if (tsc_manager_is_idle (manager)) {
			g_signal_emit (manager, signals[MANAGER_IDLE],
				       0, NULL);
		}
	}
}

void
tsc_manager_add_connection (TSCManager *manager, TSCConnection *connection)
{
	g_return_if_fail (manager != NULL);
	g_return_if_fail (connection != NULL);

	manager->connections = g_list_append (manager->connections, connection);
	g_object_ref (connection);

	g_signal_connect (connection, "started",
			  G_CALLBACK (tsc_manager_connection_started_cb),
			  manager);
	g_signal_connect (connection, "ended",
			  G_CALLBACK (tsc_manager_connection_ended_cb),
			  manager);
	
	if (tsc_connection_get_filename (connection) == NULL) {
		tsc_manager_save_connection (manager, connection);
	}

	g_signal_emit (manager, signals[MANAGER_ADDED], 0,
		       connection, NULL);
}

void
tsc_manager_save_connection (TSCManager *manager, TSCConnection *connection)
{
	GKeyFile *keys;
	const char *fname;
	char *key_data = NULL;
	GError *err = NULL;
	gsize length;
	
	g_return_if_fail (manager != NULL);
	g_return_if_fail (connection != NULL);

	if (!tsc_connection_can_save (connection)) {
		return;
	}
	
	keys = g_key_file_new ();

	fname = tsc_connection_get_filename (connection);
	if (!fname) {
		set_connection_filename (manager, connection);
		fname = tsc_connection_get_filename (connection);
	}

	if (connection->provider) {
		g_key_file_set_string (keys, GENERAL_CONFIG_GROUP,
				       PROVIDER_KEY,
				       tsc_provider_get_name (connection->provider));
	}
	
	tsc_connection_save_config (connection, keys);

	if (!g_file_test (manager->connection_dir, G_FILE_TEST_IS_DIR)) {
		g_mkdir_with_parents (manager->connection_dir, 0755);
	}
	
	key_data = g_key_file_to_data (keys, &length, NULL);
	if (!key_data) {
		g_warning ("Unable to serialize key data");
	} else if (!g_file_set_contents (fname, key_data, -1, &err)) {
		g_warning ("Failed to save connection information for: %s, %s\n",
			   tsc_connection_get_name (connection), err->message);
		g_error_free (err);
	}

	g_free (key_data);
	g_key_file_free (keys);

	g_signal_emit (manager, signals[MANAGER_SAVED], 0,
		       connection, NULL);
}

void
tsc_manager_remove_connection (TSCManager *manager, TSCConnection *connection)
{
	const char *fname;
	
	g_return_if_fail (manager != NULL);
	g_return_if_fail (connection != NULL);

	tsc_connection_write_favorite (connection, FALSE);
	
	manager->connections = g_list_remove (manager->connections, connection);

	fname = tsc_connection_get_filename (connection);
	if (fname != NULL) {
		g_unlink (fname);
	}

	g_signal_emit (manager, signals[MANAGER_REMOVED], 0,
		       connection, NULL);

	g_object_unref (connection);
}

TSCConnection *
tsc_manager_lookup_connection (TSCManager *manager, const char *name)
{
	GList *l;
	
	g_return_val_if_fail (manager != NULL, NULL);
	g_return_val_if_fail (name != NULL, NULL);

	for (l = manager->connections; l; l = l->next) {
		TSCConnection *connection = TSC_CONNECTION (l->data);
		if (strcmp (tsc_connection_get_name (connection), name) == 0) {
			return connection;
		}
	}

	return NULL;
}

gboolean
tsc_manager_is_idle (TSCManager *manager)
{
	GList *l;

	g_return_val_if_fail (manager != NULL, TRUE);
	
	for (l = manager->connections; l; l = l->next) {
		if (tsc_connection_is_connected (TSC_CONNECTION (l->data))) {
			return FALSE;
		}
	}

	return TRUE;
}

/* FIXME: this import/export stuff should obviously use libarchive
 * but it's not available on some older platforms (such as SLE10)
 */
gboolean
tsc_manager_export_connections  (TSCManager *manager, GList *connections,
				 const char *file, GError **error)
{
	GList *l, *files;
	char *export_dir, *base, *dest, *absfile = NULL;
	GString *cmdline;
	int exit_status;
	gboolean retval = TRUE;
	
	g_return_val_if_fail (manager != NULL, FALSE);

	if (!connections) {
		connections = manager->connections;

		if (!connections) {
			g_set_error (error, TSC_ERROR, TSC_ERROR,
				     _("No connections specified"));
			return FALSE;
		}
	}

	export_dir = g_strdup_printf ("%s/tsc-XXXXXX", g_get_tmp_dir ());
	mkdtemp (export_dir);
	g_mkdir (export_dir, 0755);
	files = NULL;
	
	for (l = connections; l; l = l->next) {
		base = g_path_get_basename (tsc_connection_get_filename (TSC_CONNECTION (l->data)));
		files = g_list_append (files, base);

		dest = g_build_filename (export_dir, base, NULL);
		
		if (!tsc_util_copy_file (tsc_connection_get_filename (TSC_CONNECTION (l->data)),
					 dest, error)) {
			g_free (dest);
			retval = FALSE;
			goto cleanup;
		}

		g_free (dest);
	}

	absfile = tsc_util_get_absolute_path (file);

	cmdline = g_string_new (NULL);
	g_string_append_printf (cmdline, "tar cfz %s --directory %s --remove-files ", absfile, export_dir);
	for (l = files; l; l = l->next) {
		g_string_append_printf (cmdline, "%s ", (char *) l->data);
	}

	if (!g_spawn_command_line_sync (cmdline->str, NULL, NULL, &exit_status, error)) {
		retval = FALSE;
	}
	
	g_string_free (cmdline, TRUE);

	if (exit_status != 0) {
		g_set_error (error, TSC_ERROR, TSC_ERROR,
			     _("Unknown Error"));
		retval = FALSE;
	}

cleanup:
	g_free (absfile);
	g_list_foreach (files, (GFunc) g_free, NULL);
	g_list_free (files);
	unlink (export_dir);
	g_free (export_dir);
	
	return retval;
}

GList *
tsc_manager_import_connections  (TSCManager *manager, const char *file,
				 GError **error)
{
	GList *l, *connections = NULL;
	char *import_dir_path, *absfile, *cmdline;
	const char *connfile_name;
	GDir *import_dir;
	int exit_status;
	TSCConnection *connection;
	
	g_return_val_if_fail (manager != NULL, NULL);

	absfile = tsc_util_get_absolute_path (file);
	
	import_dir_path = g_strdup_printf ("%s/tsc-XXXXXX", g_get_tmp_dir ());
	mkdtemp (import_dir_path);
	g_mkdir (import_dir_path, 0755);

	cmdline = g_strdup_printf ("tar xfz %s -C %s", absfile, import_dir_path);
	if (!g_spawn_command_line_sync (cmdline, NULL, NULL, &exit_status, error)) {
		goto cleanup;
	}
	
	if (exit_status != 0) {
		g_set_error (error, TSC_ERROR, TSC_ERROR,
			     _("Unknown Error"));
		goto cleanup;
	}

	import_dir = g_dir_open (import_dir_path, 0, error);
	if (!import_dir) {
		goto cleanup;
	}

	while ((connfile_name = g_dir_read_name (import_dir)) != NULL) {
		if (g_str_has_suffix (connfile_name, TSC_CONNECTION_SUFFIX)) {
			char *connfile_path = g_build_filename (import_dir_path, connfile_name, NULL);

			connection = tsc_manager_load_connection (manager, connfile_path, error);
			if (!connection) {
				g_dir_close (import_dir);
				g_free (connfile_path);
				goto cleanup;
			}

			connections = g_list_append (connections, connection);

			unlink (connfile_path);
			g_free (connfile_path);
		}
	}

	g_dir_close (import_dir);

	for (l = connections; l; l = l->next) {
		connection = TSC_CONNECTION (l->data);

		/* we want a newly generated filename to avoid conflicts */
		tsc_connection_set_filename (connection, NULL);
		tsc_manager_add_connection (manager, connection);
	}
	
cleanup:
	g_free (cmdline);
	unlink (import_dir_path);
	g_free (import_dir_path);
	return connections;
}
