
#include <config.h>
#include <string.h>
#include <glib/gi18n.h>
#include "tsc-util.h"
#include "tsc-mainframe-provider.h"
#include "tsc-mainframe-connection.h"
#include "tsc-mainframe-edit-dialog.h"

static TSCConnection *
tsc_mainframe_provider_create_connection (TSCProvider *provider)
{
	return TSC_CONNECTION (g_object_new (TSC_TYPE_MAINFRAME_CONNECTION, NULL));
}

static TSCEditDialog *
tsc_mainframe_provider_create_edit_dialog (TSCProvider *provider,
					 TSCConnection *connection)
{
	return TSC_EDIT_DIALOG (g_object_new (TSC_TYPE_MAINFRAME_EDIT_DIALOG,
					      "connection", connection, NULL));
}

static void
tsc_mainframe_provider_class_init (TSCMainframeProviderClass *klass)
{
	TSC_PROVIDER_CLASS (klass)->create_connection = tsc_mainframe_provider_create_connection;
	TSC_PROVIDER_CLASS (klass)->create_edit_dialog = tsc_mainframe_provider_create_edit_dialog;
}

static void
tsc_mainframe_provider_init (TSCMainframeProvider *provider)
{
	TSCProvider *parent = TSC_PROVIDER (provider);

	parent->name = "TN3270";
	parent->display_name = _("TN3270");
	parent->description = _("Create a TN3270 terminal connection");
	parent->icon_name = "gnome-terminal";
	parent->enabled = tsc_util_program_exists ("x3270");
}

GType
tsc_mainframe_provider_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCMainframeProviderClass),
			NULL, NULL,
			(GClassInitFunc) tsc_mainframe_provider_class_init,
			NULL, NULL,
			sizeof (TSCMainframeProvider),
			0,
			(GInstanceInitFunc) tsc_mainframe_provider_init
		};
		
		type = g_type_register_static (TSC_TYPE_PROVIDER,
					       "TSCMainframeProvider",
					       &type_info, 0);
	}
	
	return type;

}

TSCMainframeProvider *
tsc_mainframe_provider_new (void)
{
	return g_object_new (TSC_TYPE_MAINFRAME_PROVIDER, NULL);
}
