
#include <config.h>
#include <glib/gi18n.h>
#include "tsc-util.h"
#include "tsc-ssh-provider.h"
#include "tsc-ssh-connection.h"
#include "tsc-ssh-edit-dialog.h"

static TSCConnection *
tsc_ssh_provider_create_connection (TSCProvider *provider)
{
	return TSC_CONNECTION (g_object_new (TSC_TYPE_SSH_CONNECTION, NULL));
}

static TSCEditDialog *
tsc_ssh_provider_create_edit_dialog (TSCProvider *provider,
					 TSCConnection *connection)
{
	return TSC_EDIT_DIALOG (g_object_new (TSC_TYPE_SSH_EDIT_DIALOG,
					      "connection", connection, NULL));
}

static void
tsc_ssh_provider_class_init (TSCSshProviderClass *klass)
{
	TSC_PROVIDER_CLASS (klass)->create_connection = tsc_ssh_provider_create_connection;
	TSC_PROVIDER_CLASS (klass)->create_edit_dialog = tsc_ssh_provider_create_edit_dialog;
}

static void
tsc_ssh_provider_init (TSCSshProvider *provider)
{
	TSCProvider *parent = TSC_PROVIDER (provider);

	parent->name = "SSH";
	parent->display_name = _("SSH");
	parent->description = _("Create an ssh connection");
	parent->icon_name = "gnome-fs-ssh";
	parent->enabled = tsc_util_program_exists ("ssh");
}

GType
tsc_ssh_provider_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCSshProviderClass),
			NULL, NULL,
			(GClassInitFunc) tsc_ssh_provider_class_init,
			NULL, NULL,
			sizeof (TSCSshProvider),
			0,
			(GInstanceInitFunc) tsc_ssh_provider_init
		};
		
		type = g_type_register_static (TSC_TYPE_PROVIDER,
					       "TSCSshProvider",
					       &type_info, 0);
	}
	
	return type;

}

TSCSshProvider *
tsc_ssh_provider_new (void)
{
	return g_object_new (TSC_TYPE_SSH_PROVIDER, NULL);
}
