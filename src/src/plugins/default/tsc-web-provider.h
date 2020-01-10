
#ifndef __TSC_WEB_PROVIDER__
#define __TSC_WEB_PROVIDER__

#include <glib.h>
#include <glib-object.h>
#include "tsc-provider.h"

#define TSC_TYPE_WEB_PROVIDER                   (tsc_web_provider_get_type ())
#define TSC_WEB_PROVIDER(web_provider)          (G_TYPE_CHECK_INSTANCE_CAST ((web_provider), TSC_TYPE_WEB_PROVIDER, TSCWebProvider))
#define TSC_WEB_PROVIDER_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_WEB_PROVIDER, TSCWebProviderClass))
#define TSC_IS_WEB_PROVIDER(web_provider)       (G_TYPE_CHECK_INSTANCE_TYPE ((web_provider), TSC_TYPE_WEB_PROVIDER))
#define TSC_IS_WEB_PROVIDER_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_WEB_PROVIDER))
#define TSC_WEB_PROVIDER_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_WEB_PROVIDER, TSCWebProviderClass))


typedef struct _TSCWebProvider {
	TSCProvider parent;
} TSCWebProvider;

typedef struct _TSCWebProviderClass {
	TSCProviderClass parent_class;
} TSCWebProviderClass;

GType               tsc_web_provider_get_type      (void);
TSCWebProvider *tsc_web_provider_new               (void);

#endif
