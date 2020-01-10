
#include <config.h>
#include <string.h>
#include <glib/gi18n.h>
#include "tsc-rdp-provider.h"
#include "tsc-rdp-edit-dialog.h"
#include "tsc-rdp-connection.h"
#include "tsc-util.h"

#define TSC_RDP_URI_SCHEME "rdp://"

static TSCConnection *
tsc_rdp_provider_create_connection (TSCProvider *provider)
{
	return TSC_CONNECTION (g_object_new (TSC_TYPE_RDP_CONNECTION, NULL));
}

static TSCConnection *
tsc_rdp_provider_create_from_uri (TSCProvider *provider, const char *uri)
{
	TSCConnection *connection = NULL;
	char *host;
	
	if (!tsc_util_check_uri_scheme (uri, TSC_RDP_URI_SCHEME)) {
		return NULL;
	}

	host = tsc_util_get_uri_host (uri);
	if (!host) {
		return NULL;
	}

	connection = TSC_CONNECTION (g_object_new (TSC_TYPE_RDP_CONNECTION, NULL));
	TSC_RDP_CONNECTION (connection)->host = host;

	g_free (connection->name);
	connection->name = g_strdup_printf (_("RDP on %s"),
					    TSC_RDP_CONNECTION (connection)->host);
	
	return connection;
}

static TSCEditDialog *
tsc_rdp_provider_create_edit_dialog (TSCProvider *provider, TSCConnection *connection)
{
	return TSC_EDIT_DIALOG (g_object_new (TSC_TYPE_RDP_EDIT_DIALOG, "connection", connection, NULL));
}

static void
tsc_rdp_provider_class_init (TSCRdpProviderClass *klass)
{
	TSC_PROVIDER_CLASS (klass)->create_connection  = tsc_rdp_provider_create_connection;
	TSC_PROVIDER_CLASS (klass)->create_from_uri    = tsc_rdp_provider_create_from_uri;
	TSC_PROVIDER_CLASS (klass)->create_edit_dialog = tsc_rdp_provider_create_edit_dialog;
}

static void
tsc_rdp_provider_set_icon (TSCRdpProvider *provider)
{
	GtkIconTheme *theme;
	GdkPixbuf *icon;

	theme = gtk_icon_theme_get_default ();
	icon = gtk_icon_theme_load_icon (theme, "computer",
					 TSC_PROVIDER_ICON_SIZE, 0, NULL);
}

static void
tsc_rdp_provider_init (TSCRdpProvider *provider)
{
	TSCProvider *parent = TSC_PROVIDER (provider);
	

	parent->name = "RDP";
	parent->display_name = _("Windows Terminal Service");
	parent->description = _("Connect to Windows machines via Terminal Services");
	parent->icon_name = "computer";
	parent->enabled = tsc_util_program_exists (TSC_RDP_PROGRAM);

	tsc_rdp_provider_set_icon (provider);
}

GType
tsc_rdp_provider_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCRdpProviderClass),
			NULL, NULL,
			(GClassInitFunc) tsc_rdp_provider_class_init,
			NULL, NULL,
			sizeof (TSCRdpProvider),
			0,
			(GInstanceInitFunc) tsc_rdp_provider_init
		};
		
		type = g_type_register_static (TSC_TYPE_PROVIDER,
					       "TSCRdpProvider",
					       &type_info, 0);
	}
	
	return type;

}

TSCRdpProvider *
tsc_rdp_provider_new (void)
{
	return g_object_new (TSC_TYPE_RDP_PROVIDER, NULL);
}
