
#ifndef __TSC_GENERIC_PROVIDER__
#define __TSC_GENERIC_PROVIDER__

#include <glib.h>
#include <glib-object.h>
#include "tsc-provider.h"

#define TSC_TYPE_GENERIC_PROVIDER                   (tsc_generic_provider_get_type ())
#define TSC_GENERIC_PROVIDER(generic_provider)      (G_TYPE_CHECK_INSTANCE_CAST ((generic_provider), TSC_TYPE_GENERIC_PROVIDER, TSCGenericProvider))
#define TSC_GENERIC_PROVIDER_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_GENERIC_PROVIDER, TSCGenericProviderClass))
#define TSC_IS_GENERIC_PROVIDER(generic_provider)   (G_TYPE_CHECK_INSTANCE_TYPE ((generic_provider), TSC_TYPE_GENERIC_PROVIDER))
#define TSC_IS_GENERIC_PROVIDER_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_GENERIC_PROVIDER))
#define TSC_GENERIC_PROVIDER_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_GENERIC_PROVIDER, TSCGenericProviderClass))


typedef struct _TSCGenericProvider {
	TSCProvider parent;
} TSCGenericProvider;

typedef struct _TSCGenericProviderClass {
	TSCProviderClass parent_class;
} TSCGenericProviderClass;

GType               tsc_generic_provider_get_type      (void);
TSCGenericProvider *tsc_generic_provider_new           (void);

#endif
