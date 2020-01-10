
#include <config.h>
#include "tsc-provider.h"

static void
tsc_provider_class_init (TSCProviderClass *klass)
{
}

static void
tsc_provider_init (TSCProvider *provider)
{
}

GType
tsc_provider_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCProviderClass),
			NULL, NULL,
			(GClassInitFunc) tsc_provider_class_init,
			NULL, NULL,
			sizeof (TSCProvider),
			0,
			(GInstanceInitFunc) tsc_provider_init
		};
		
		type = g_type_register_static (G_TYPE_OBJECT,
					       "TSCProvider",
					       &type_info,
					       G_TYPE_FLAG_ABSTRACT);
	}
	
	return type;

}

const char *
tsc_provider_get_name (TSCProvider *provider)
{
	g_return_val_if_fail (provider != NULL, NULL);

	return provider->name;
}

const char *
tsc_provider_get_display_name (TSCProvider *provider)
{
	g_return_val_if_fail (provider != NULL, NULL);

	return provider->display_name;
}

const char *
tsc_provider_get_description (TSCProvider *provider)
{
	g_return_val_if_fail (provider != NULL, NULL);

	return provider->description;
}

gboolean
tsc_provider_is_enabled (TSCProvider *provider)
{
	g_return_val_if_fail (provider != NULL, FALSE);
	
	return provider->enabled;
}

const char *
tsc_provider_get_icon_name (TSCProvider *provider)
{
	g_return_val_if_fail (provider != NULL, NULL);
	return provider->icon_name;
}

TSCConnection *
tsc_provider_create_connection (TSCProvider *provider)
{
	TSCProviderClass *class;
	TSCConnection *connection;
	
	g_return_val_if_fail (provider != NULL, NULL);

	class = TSC_PROVIDER_GET_CLASS (provider);
	if (class->create_connection == NULL) {
		return NULL;
	}

	connection = class->create_connection (provider);
	if (connection) {
		connection->provider = provider;
	}

	return connection;
}

TSCConnection *
tsc_provider_create_from_uri (TSCProvider *provider, const char  *uri)
{
	TSCProviderClass *class;
	TSCConnection *connection;
	
	g_return_val_if_fail (provider != NULL, NULL);

	class = TSC_PROVIDER_GET_CLASS (provider);
	if (class->create_from_uri == NULL) {
		return NULL;
	}

	connection = class->create_from_uri (provider, uri);
	if (connection) {
		connection->provider = provider;
	}

	return connection;
}

TSCEditDialog *
tsc_provider_create_edit_dialog (TSCProvider   *provider,
				 TSCConnection *connection)
{
	TSCProviderClass *class;

	g_return_val_if_fail (provider != NULL, NULL);
	g_return_val_if_fail (connection != NULL, NULL);

	class = TSC_PROVIDER_GET_CLASS (provider);
	if (class->create_edit_dialog) {
		return class->create_edit_dialog (provider, connection);
	} else {
		return tsc_edit_dialog_new (NULL, connection);
	}
}

gboolean
tsc_provider_can_create (TSCProvider *provider)
{
	TSCProviderClass *class;

	g_return_val_if_fail (provider != NULL, FALSE);

	class = TSC_PROVIDER_GET_CLASS (provider);
	return class->create_connection != NULL;
}

gboolean
tsc_provider_can_create_from_uri (TSCProvider *provider)
{
	TSCProviderClass *class;

	g_return_val_if_fail (provider != NULL, FALSE);

	class = TSC_PROVIDER_GET_CLASS (provider);
	return class->create_from_uri != NULL;
}

gboolean
tsc_provider_can_edit (TSCProvider *provider)
{
	TSCProviderClass *class;

	g_return_val_if_fail (provider != NULL, FALSE);

	class = TSC_PROVIDER_GET_CLASS (provider);
	return class->create_edit_dialog != NULL;
}
