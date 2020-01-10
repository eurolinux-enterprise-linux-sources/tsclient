
#include <config.h>
#include <glib/gi18n.h>
#include "tsc-web-provider.h"
#include "tsc-web-connection.h"
#include "tsc-web-edit-dialog.h"

static TSCConnection *
tsc_web_provider_create_connection (TSCProvider *provider)
{
	return TSC_CONNECTION (g_object_new (TSC_TYPE_WEB_CONNECTION, NULL));
}

static TSCEditDialog *
tsc_web_provider_create_edit_dialog (TSCProvider *provider,
					 TSCConnection *connection)
{
	return TSC_EDIT_DIALOG (g_object_new (TSC_TYPE_WEB_EDIT_DIALOG,
					      "connection", connection, NULL));
}

static void
tsc_web_provider_class_init (TSCWebProviderClass *klass)
{
	TSC_PROVIDER_CLASS (klass)->create_connection = tsc_web_provider_create_connection;
	TSC_PROVIDER_CLASS (klass)->create_edit_dialog = tsc_web_provider_create_edit_dialog;
}

static void
tsc_web_provider_init (TSCWebProvider *provider)
{
	TSCProvider *parent = TSC_PROVIDER (provider);

	parent->name = "Web";
	parent->display_name = _("Web");
	parent->description = _("Create a web connection");
	parent->icon_name = "applications-internet";
	parent->enabled = TRUE;
}

GType
tsc_web_provider_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCWebProviderClass),
			NULL, NULL,
			(GClassInitFunc) tsc_web_provider_class_init,
			NULL, NULL,
			sizeof (TSCWebProvider),
			0,
			(GInstanceInitFunc) tsc_web_provider_init
		};
		
		type = g_type_register_static (TSC_TYPE_PROVIDER,
					       "TSCWebProvider",
					       &type_info, 0);
	}
	
	return type;

}

TSCWebProvider *
tsc_web_provider_new (void)
{
	return g_object_new (TSC_TYPE_WEB_PROVIDER, NULL);
}
