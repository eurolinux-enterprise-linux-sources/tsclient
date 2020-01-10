
#include <config.h>
#include <glib/gi18n.h>
#include "tsc-util.h"
#include "tsc-xdmcp-provider.h"
#include "tsc-xdmcp-connection.h"
#include "tsc-xdmcp-edit-dialog.h"

static TSCConnection *
tsc_xdmcp_provider_create_connection (TSCProvider *provider)
{
	return TSC_CONNECTION (g_object_new (TSC_TYPE_XDMCP_CONNECTION, NULL));
}

static TSCEditDialog *
tsc_xdmcp_provider_create_edit_dialog (TSCProvider *provider,
					 TSCConnection *connection)
{
	return TSC_EDIT_DIALOG (g_object_new (TSC_TYPE_XDMCP_EDIT_DIALOG,
					      "connection", connection, NULL));
}

static void
tsc_xdmcp_provider_class_init (TSCXdmcpProviderClass *klass)
{
	TSC_PROVIDER_CLASS (klass)->create_connection = tsc_xdmcp_provider_create_connection;
	TSC_PROVIDER_CLASS (klass)->create_edit_dialog = tsc_xdmcp_provider_create_edit_dialog;
}

static void
tsc_xdmcp_provider_init (TSCXdmcpProvider *provider)
{
	TSCProvider *parent = TSC_PROVIDER (provider);

	parent->name = "XDMCP";
	parent->display_name = _("XDMCP");
	parent->description = _("Create a XDMCP connection");
	parent->icon_name = "gdm-xnest";
	parent->enabled = tsc_util_program_exists ("Xnest") ||
					  tsc_util_program_exists ("Xephyr");
}

GType
tsc_xdmcp_provider_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCXdmcpProviderClass),
			NULL, NULL,
			(GClassInitFunc) tsc_xdmcp_provider_class_init,
			NULL, NULL,
			sizeof (TSCXdmcpProvider),
			0,
			(GInstanceInitFunc) tsc_xdmcp_provider_init
		};
		
		type = g_type_register_static (TSC_TYPE_PROVIDER,
					       "TSCXdmcpProvider",
					       &type_info, 0);
	}
	
	return type;

}

TSCXdmcpProvider *
tsc_xdmcp_provider_new (void)
{
	return g_object_new (TSC_TYPE_XDMCP_PROVIDER, NULL);
}
