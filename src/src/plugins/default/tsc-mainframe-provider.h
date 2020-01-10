
#ifndef __TSC_MAINFRAME_PROVIDER__
#define __TSC_MAINFRAME_PROVIDER__

#include <glib.h>
#include <glib-object.h>
#include "tsc-provider.h"

#define TSC_TYPE_MAINFRAME_PROVIDER                   (tsc_mainframe_provider_get_type ())
#define TSC_MAINFRAME_PROVIDER(mainframe_provider)      (G_TYPE_CHECK_INSTANCE_CAST ((mainframe_provider), TSC_TYPE_MAINFRAME_PROVIDER, TSCMainframeProvider))
#define TSC_MAINFRAME_PROVIDER_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_MAINFRAME_PROVIDER, TSCMainframeProviderClass))
#define TSC_IS_MAINFRAME_PROVIDER(mainframe_provider)   (G_TYPE_CHECK_INSTANCE_TYPE ((mainframe_provider), TSC_TYPE_MAINFRAME_PROVIDER))
#define TSC_IS_MAINFRAME_PROVIDER_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_MAINFRAME_PROVIDER))
#define TSC_MAINFRAME_PROVIDER_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_MAINFRAME_PROVIDER, TSCMainframeProviderClass))


typedef struct _TSCMainframeProvider {
	TSCProvider parent;
} TSCMainframeProvider;

typedef struct _TSCMainframeProviderClass {
	TSCProviderClass parent_class;
} TSCMainframeProviderClass;

GType                 tsc_mainframe_provider_get_type      (void);
TSCMainframeProvider *tsc_mainframe_provider_new           (void);

#endif
