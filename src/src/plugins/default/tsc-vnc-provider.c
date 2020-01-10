
#include <config.h>
#include <glib/gi18n.h>
#include "tsc-util.h"
#include "tsc-vnc-provider.h"
#include "tsc-vnc-connection.h"
#include "tsc-vnc-edit-dialog.h"

#define TSC_VNC_URI_SCHEME "vnc://"

static TSCConnection *
tsc_vnc_provider_create_connection (TSCProvider *provider)
{
	return TSC_CONNECTION (g_object_new (TSC_TYPE_VNC_CONNECTION, NULL));
}

static TSCConnection *
tsc_vnc_provider_create_from_uri (TSCProvider *provider, const char *uri)
{
	TSCConnection *connection = NULL;
	char *host;
	
	if (!tsc_util_check_uri_scheme (uri, TSC_VNC_URI_SCHEME)) {
		return NULL;
	}

	host = tsc_util_get_uri_host (uri);
	if (!host) {
		return NULL;
	}

	connection = TSC_CONNECTION (g_object_new (TSC_TYPE_VNC_CONNECTION, NULL));
	TSC_VNC_CONNECTION (connection)->host = host;

	g_free (connection->name);
	connection->name = g_strdup_printf (_("VNC on %s"),
					    TSC_VNC_CONNECTION (connection)->host);
	
	return connection;
}


static TSCEditDialog *
tsc_vnc_provider_create_edit_dialog (TSCProvider *provider,
					 TSCConnection *connection)
{
	return TSC_EDIT_DIALOG (g_object_new (TSC_TYPE_VNC_EDIT_DIALOG,
					      "connection", connection, NULL));
}

static void
tsc_vnc_provider_class_init (TSCVncProviderClass *klass)
{
	TSC_PROVIDER_CLASS (klass)->create_connection = tsc_vnc_provider_create_connection;
	TSC_PROVIDER_CLASS (klass)->create_from_uri = tsc_vnc_provider_create_from_uri;
	TSC_PROVIDER_CLASS (klass)->create_edit_dialog = tsc_vnc_provider_create_edit_dialog;
}

static void
tsc_vnc_provider_init (TSCVncProvider *provider)
{
	TSCProvider *parent = TSC_PROVIDER (provider);

	parent->name = "VNC";
	parent->display_name = _("VNC");
	parent->description = _("Create a VNC connection");
	parent->icon_name = "gnome-remote-desktop";
	parent->enabled = tsc_util_program_exists ("vncviewer");
}

GType
tsc_vnc_provider_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCVncProviderClass),
			NULL, NULL,
			(GClassInitFunc) tsc_vnc_provider_class_init,
			NULL, NULL,
			sizeof (TSCVncProvider),
			0,
			(GInstanceInitFunc) tsc_vnc_provider_init
		};
		
		type = g_type_register_static (TSC_TYPE_PROVIDER,
					       "TSCVncProvider",
					       &type_info, 0);
	}
	
	return type;

}

TSCVncProvider *
tsc_vnc_provider_new (void)
{
	return g_object_new (TSC_TYPE_VNC_PROVIDER, NULL);
}
