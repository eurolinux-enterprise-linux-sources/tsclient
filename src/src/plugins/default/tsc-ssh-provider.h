
#ifndef __TSC_SSH_PROVIDER__
#define __TSC_SSH_PROVIDER__

#include <glib.h>
#include <glib-object.h>
#include "tsc-provider.h"

#define TSC_TYPE_SSH_PROVIDER                   (tsc_ssh_provider_get_type ())
#define TSC_SSH_PROVIDER(ssh_provider)      (G_TYPE_CHECK_INSTANCE_CAST ((ssh_provider), TSC_TYPE_SSH_PROVIDER, TSCSshProvider))
#define TSC_SSH_PROVIDER_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_SSH_PROVIDER, TSCSshProviderClass))
#define TSC_IS_SSH_PROVIDER(ssh_provider)   (G_TYPE_CHECK_INSTANCE_TYPE ((ssh_provider), TSC_TYPE_SSH_PROVIDER))
#define TSC_IS_SSH_PROVIDER_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_SSH_PROVIDER))
#define TSC_SSH_PROVIDER_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_SSH_PROVIDER, TSCSshProviderClass))


typedef struct _TSCSshProvider {
	TSCProvider parent;
} TSCSshProvider;

typedef struct _TSCSshProviderClass {
	TSCProviderClass parent_class;
} TSCSshProviderClass;

GType               tsc_ssh_provider_get_type      (void);
TSCSshProvider *tsc_ssh_provider_new           (void);

#endif
