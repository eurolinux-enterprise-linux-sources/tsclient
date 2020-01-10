
#ifndef __TSC_ICA_PROVIDER__
#define __TSC_ICA_PROVIDER__

#include <glib.h>
#include <glib-object.h>
#include "tsc-provider.h"
#include "tsc-manager.h"

#define TSC_TYPE_ICA_PROVIDER                   (tsc_ica_provider_get_type ())
#define TSC_ICA_PROVIDER(ica_provider)          (G_TYPE_CHECK_INSTANCE_CAST ((ica_provider), TSC_TYPE_ICA_PROVIDER, TSCIcaProvider))
#define TSC_ICA_PROVIDER_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_ICA_PROVIDER, TSCIcaProviderClass))
#define TSC_IS_ICA_PROVIDER(ica_provider)       (G_TYPE_CHECK_INSTANCE_TYPE ((ica_provider), TSC_TYPE_ICA_PROVIDER))
#define TSC_IS_ICA_PROVIDER_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_ICA_PROVIDER))
#define TSC_ICA_PROVIDER_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_ICA_PROVIDER, TSCIcaProviderClass))

typedef struct _TSCIcaProvider {
	TSCProvider parent;
} TSCIcaProvider;

typedef struct _TSCIcaProviderClass {
	TSCProviderClass parent_class;
} TSCIcaProviderClass;

GType           tsc_ica_provider_get_type         (void);
TSCIcaProvider *tsc_ica_provider_new              (void);
void            tsc_ica_provider_load_connections (TSCIcaProvider *provider, TSCManager *manager);
void            tsc_ica_provider_appsrv_removed   (TSCManager *manager, TSCConnection *con, gpointer user_data);
#endif
