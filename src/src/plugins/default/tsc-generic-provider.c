
#include <config.h>
#include <glib/gi18n.h>
#include "tsc-generic-provider.h"
#include "tsc-generic-connection.h"
#include "tsc-generic-edit-dialog.h"

static TSCConnection *
tsc_generic_provider_create_connection (TSCProvider *provider)
{
	return TSC_CONNECTION (g_object_new (TSC_TYPE_GENERIC_CONNECTION, NULL));
}

static TSCEditDialog *
tsc_generic_provider_create_edit_dialog (TSCProvider *provider,
					 TSCConnection *connection)
{
	return TSC_EDIT_DIALOG (g_object_new (TSC_TYPE_GENERIC_EDIT_DIALOG,
					      "connection", connection, NULL));
}

static void
tsc_generic_provider_class_init (TSCGenericProviderClass *klass)
{
	TSC_PROVIDER_CLASS (klass)->create_connection = tsc_generic_provider_create_connection;
	TSC_PROVIDER_CLASS (klass)->create_edit_dialog = tsc_generic_provider_create_edit_dialog;
}

static void
tsc_generic_provider_init (TSCGenericProvider *provider)
{
	TSCProvider *parent = TSC_PROVIDER (provider);

	parent->name = "Generic";
	parent->display_name = _("Custom Command");
	parent->description = _("Create a generic freeform connection");
	parent->icon_name = "gnome-run";
	parent->enabled = TRUE;
}

GType
tsc_generic_provider_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCGenericProviderClass),
			NULL, NULL,
			(GClassInitFunc) tsc_generic_provider_class_init,
			NULL, NULL,
			sizeof (TSCGenericProvider),
			0,
			(GInstanceInitFunc) tsc_generic_provider_init
		};
		
		type = g_type_register_static (TSC_TYPE_PROVIDER,
					       "TSCGenericProvider",
					       &type_info, 0);
	}
	
	return type;

}

TSCGenericProvider *
tsc_generic_provider_new (void)
{
	return g_object_new (TSC_TYPE_GENERIC_PROVIDER, NULL);
}
