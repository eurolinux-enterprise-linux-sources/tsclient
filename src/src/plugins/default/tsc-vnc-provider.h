
#ifndef __TSC_VNC_PROVIDER__
#define __TSC_VNC_PROVIDER__

#include <glib.h>
#include <glib-object.h>
#include "tsc-provider.h"

#define TSC_TYPE_VNC_PROVIDER                   (tsc_vnc_provider_get_type ())
#define TSC_VNC_PROVIDER(generic_provider)      (G_TYPE_CHECK_INSTANCE_CAST ((generic_provider), TSC_TYPE_VNC_PROVIDER, TSCVncProvider))
#define TSC_VNC_PROVIDER_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_VNC_PROVIDER, TSCVncProviderClass))
#define TSC_IS_VNC_PROVIDER(generic_provider)   (G_TYPE_CHECK_INSTANCE_TYPE ((generic_provider), TSC_TYPE_VNC_PROVIDER))
#define TSC_IS_VNC_PROVIDER_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_VNC_PROVIDER))
#define TSC_VNC_PROVIDER_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_VNC_PROVIDER, TSCVncProviderClass))


typedef struct _TSCVncProvider {
	TSCProvider parent;
} TSCVncProvider;

typedef struct _TSCVncProviderClass {
	TSCProviderClass parent_class;
} TSCVncProviderClass;

GType               tsc_vnc_provider_get_type      (void);
TSCVncProvider *tsc_vnc_provider_new           (void);

#endif
