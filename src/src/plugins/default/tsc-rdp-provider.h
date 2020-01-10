
#ifndef __TSC_RDP_PROVIDER__
#define __TSC_RDP_PROVIDER__

#include <glib.h>
#include <glib-object.h>
#include "tsc-provider.h"

#define TSC_TYPE_RDP_PROVIDER                   (tsc_rdp_provider_get_type ())
#define TSC_RDP_PROVIDER(rdp_provider)                (G_TYPE_CHECK_INSTANCE_CAST ((rdp_provider), TSC_TYPE_RDP_PROVIDER, TSCRdpProvider))
#define TSC_RDP_PROVIDER_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_RDP_PROVIDER, TSCRdpProviderClass))
#define TSC_IS_RDP_PROVIDER(rdp_provider)             (G_TYPE_CHECK_INSTANCE_TYPE ((rdp_provider), TSC_TYPE_RDP_PROVIDER))
#define TSC_IS_RDP_PROVIDER_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_RDP_PROVIDER))
#define TSC_RDP_PROVIDER_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_RDP_PROVIDER, TSCRdpProviderClass))


typedef struct _TSCRdpProvider {
	TSCProvider parent;
} TSCRdpProvider;

typedef struct _TSCRdpProviderClass {
	TSCProviderClass parent_class;
} TSCRdpProviderClass;

GType           tsc_rdp_provider_get_type      (void);
TSCRdpProvider *tsc_rdp_provider_new           (void);

#endif
