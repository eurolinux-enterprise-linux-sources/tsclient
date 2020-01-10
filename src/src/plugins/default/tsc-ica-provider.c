
#include <config.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <regex.h>
#include <glib/gi18n.h>
#include <glib.h>
#include "tsc-ica-provider.h"
#include "tsc-ica-connection.h"
#include "tsc-util.h"

#define APPSRV_GROUP_NAME "ApplicationServers"

static TSCManager *tsc_manager;

typedef struct {
	GPid pid;
	TSCConnection *connection;
	TSCProvider *provider;
} ICATimeoutData;

static GList *find_app_descriptions (void);

static TSCConnection *
tsc_ica_provider_create_connection (TSCProvider *provider)
{
	return TSC_CONNECTION (g_object_new (TSC_TYPE_ICA_CONNECTION, NULL));
}

static gboolean
tsc_ica_provider_watch_connections (gpointer user_data)
{
	ICATimeoutData *data = user_data;
	GList *apps = NULL, *l = NULL;
	
	if (!kill (data->pid, 0))
		return TRUE;

	if (!tsc_manager) {
		g_printerr ("Manager is not set for ICA provider\n");
		return FALSE;
	}

	apps = find_app_descriptions ();
	if (!apps) {
		g_printerr ("There are no app descriptions\n");
		return FALSE;
	}

	for (l = apps; l; l = l->next) {
		if (!tsc_manager_lookup_connection (tsc_manager, l->data)) {
			if (!strcmp (tsc_connection_get_name (data->connection), l->data)) {
				tsc_manager_add_connection (tsc_manager, data->connection);
			} else {
				TSCIcaConnection *connection = tsc_ica_connection_new ((const char *) l->data);
				TSC_CONNECTION (connection)->provider = TSC_PROVIDER (data->provider);
				tsc_manager_add_connection (tsc_manager, TSC_CONNECTION (connection));
			}
		}
	}
	
	g_list_foreach (apps, (GFunc) g_free, NULL);
	g_list_free (apps);
	g_free (data);
	return FALSE;
}

static TSCEditDialog *
tsc_ica_provider_create_edit_dialog (TSCProvider *provider,
					 TSCConnection *connection)
{
	char *argv[] = { WFCMGR_PATH , NULL};
	GError *error = NULL;
	ICATimeoutData *td = NULL;
	GPid pid;

	if (!g_file_test (WFCMGR_PATH, G_FILE_TEST_EXISTS)) {
		tsc_util_show_error ("Error", "Can't find wfcmgr");
		return NULL;
	}

	if (!g_spawn_async (NULL, argv, NULL, 0, NULL, NULL, &pid, &error)) {
		if (error) {
			tsc_util_show_error ("Error", error->message);
			g_error_free (error);
		} else {
			tsc_util_show_error ("Error", "Failed to start wfcmgr");
		}
	}

	td = g_new0 (ICATimeoutData, 1);
	td->pid = pid;
	td->connection = connection;
	td->provider = provider;
	g_timeout_add (3000, tsc_ica_provider_watch_connections, td);
	
	return NULL;
}

static void
tsc_ica_provider_class_init (TSCIcaProviderClass *klass)
{
	TSC_PROVIDER_CLASS (klass)->create_connection = tsc_ica_provider_create_connection;
	TSC_PROVIDER_CLASS (klass)->create_edit_dialog = tsc_ica_provider_create_edit_dialog;
}

static void
tsc_ica_provider_init (TSCIcaProvider *provider)
{
	TSCProvider *parent = TSC_PROVIDER (provider);

	parent->name = "ICA";
	parent->display_name = _("Citrix ICA Connection");
	parent->description = _("Create a connection which uses the Citrix ICA client");
	parent->icon_name = "gnome-fs-network";
	parent->enabled = tsc_util_program_exists (WFICA_PATH);
}

GType
tsc_ica_provider_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCIcaProviderClass),
			NULL, NULL,
			(GClassInitFunc) tsc_ica_provider_class_init,
			NULL, NULL,
			sizeof (TSCIcaProvider),
			0,
			(GInstanceInitFunc) tsc_ica_provider_init
		};
		
		type = g_type_register_static (TSC_TYPE_PROVIDER,
					       "TSCIcaProvider",
					       &type_info, 0);
	}
	
	return type;

}

TSCIcaProvider *
tsc_ica_provider_new (void)
{
	return g_object_new (TSC_TYPE_ICA_PROVIDER, NULL);
}

static GKeyFile *
open_appsrv (void)
{
	GKeyFile *key_file;
	char *appsrv_path, *data;
	gsize data_len;
	int i;

	data = NULL;
	key_file = NULL;
	
	appsrv_path = g_build_filename (g_get_home_dir (), ".ICAClient", "appsrv.ini", NULL);
	if (!g_file_test (appsrv_path, G_FILE_TEST_IS_REGULAR)) {
		goto cleanup;
	}

	if (!g_file_get_contents (appsrv_path, &data, &data_len, NULL)) {
		goto cleanup;
	}

	/* replace the ';' comments with '#' so GKeyFile can cope */
	for (i = 0; i < data_len; i++) {
		if (data[i] == ';' && (i == 0 || data[i - 1] == '\n')) {
			data[i] = '#';
		}
	}

	key_file = g_key_file_new ();
	if (!g_key_file_load_from_data (key_file, data, data_len,
					G_KEY_FILE_NONE, NULL)) {
		g_key_file_free (key_file);
		key_file = NULL;
	}

cleanup:
	g_free (appsrv_path);

	if (data) {
		g_free (data);
	}

	return key_file;
}

static GList *
find_app_descriptions (void)
{
	GKeyFile *key_file;
	GList *list;
	char **keys;
	int i;

	list = NULL;

	key_file = open_appsrv ();
	if (!key_file) {
		return list;
	}

	keys = g_key_file_get_keys (key_file, APPSRV_GROUP_NAME,
				    NULL, NULL);
	if (!keys) {
		g_key_file_free (key_file);
		return list;
	}

	for (i = 0; keys[i]; i++) {
		list = g_list_prepend (list, g_strdup (keys[i]));
	}

	g_key_file_free (key_file);
	return list;
}

void
tsc_ica_provider_appsrv_removed (TSCManager *manager, TSCConnection *con, gpointer user_data)
{
	GKeyFile *kf = NULL;
	const gchar *cname;
	gchar *appsrv_path = NULL, *data = NULL;
	gsize data_len;
	GError *err = NULL;
	int i;

	if (!TSC_IS_ICA_CONNECTION (con)) {
		return;
	}

	kf = open_appsrv ();
	cname = tsc_connection_get_name (con);
	
	if (!kf || !g_key_file_remove_key (kf, APPSRV_GROUP_NAME, cname, NULL)) {
		g_printerr ("Failed to remove key %s\n", cname);
		goto done;
	}

	if (!g_key_file_remove_group (kf, cname, NULL)) {
		g_printerr ("Failed to remove group %s\n", cname);
		goto done;
	}

	appsrv_path = g_build_filename (g_get_home_dir (), ".ICAClient", "appsrv.ini", NULL);
	data = g_key_file_to_data (kf, &data_len, NULL);

	/* replace the '#' comments with ';' so wfcmgr doesn't barf */
	for (i = 0; i < data_len; i++) {
		if (data[i] == '#' && (i == 0 || data[i - 1] == '\n')) {
			data[i] = ';';
		}
	}
	
	if (!g_file_set_contents (appsrv_path, data, data_len, &err)) {
		g_printerr ("Failed to save appsrv.ini: %s\n", err->message);
		g_error_free (err);
	}

done:
	g_free (appsrv_path);
	g_free (data);
	g_key_file_free (kf);
}

void
tsc_ica_provider_load_connections (TSCIcaProvider *provider, TSCManager *manager)
{
	GList *apps, *l;
	TSCIcaConnection *connection;

	tsc_manager = manager;
	apps = find_app_descriptions ();
	if (!apps) {
		return;
	}

	for (l = apps; l; l = l->next) {
		connection = tsc_ica_connection_new ((const char *) l->data);
		TSC_CONNECTION (connection)->provider = TSC_PROVIDER (provider);
		
		tsc_manager_add_connection (manager, TSC_CONNECTION (connection));
	}

	g_list_foreach (apps, (GFunc) g_free, NULL);
	g_list_free (apps);
}
