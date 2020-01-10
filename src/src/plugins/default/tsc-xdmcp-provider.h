
#ifndef __TSC_XDMCP_PROVIDER__
#define __TSC_XDMCP_PROVIDER__

#include <glib.h>
#include <glib-object.h>
#include "tsc-provider.h"

#define TSC_TYPE_XDMCP_PROVIDER                   (tsc_xdmcp_provider_get_type ())
#define TSC_XDMCP_PROVIDER(xdmcp_provider)      (G_TYPE_CHECK_INSTANCE_CAST ((xdmcp_provider), TSC_TYPE_GENERIC_PROVIDER, TSCXdmcpProvider))
#define TSC_XDMCP_PROVIDER_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_GENERIC_PROVIDER, TSCXdmcpProviderClass))
#define TSC_IS_XDMCP_PROVIDER(xdmcp_provider)   (G_TYPE_CHECK_INSTANCE_TYPE ((xdmcp_provider), TSC_TYPE_GENERIC_PROVIDER))
#define TSC_IS_XDMCP_PROVIDER_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_GENERIC_PROVIDER))
#define TSC_XDMCP_PROVIDER_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_GENERIC_PROVIDER, TSCXdmcpProviderClass))


typedef struct _TSCXdmcpProvider {
	TSCProvider parent;
} TSCXdmcpProvider;

typedef struct _TSCXdmcpProviderClass {
	TSCProviderClass parent_class;
} TSCXdmcpProviderClass;

GType               tsc_xdmcp_provider_get_type      (void);
TSCXdmcpProvider   *tsc_xdmcp_provider_new           (void);

#endif
