
#ifndef __TSC_PROVIDER__
#define __TSC_PROVIDER__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include "tsc-connection.h"
#include "tsc-edit-dialog.h"

#define TSC_TYPE_PROVIDER                   (tsc_provider_get_type ())
#define TSC_PROVIDER(provider)                (G_TYPE_CHECK_INSTANCE_CAST ((provider), TSC_TYPE_PROVIDER, TSCProvider))
#define TSC_PROVIDER_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_PROVIDER, TSCProviderClass))
#define TSC_IS_PROVIDER(provider)             (G_TYPE_CHECK_INSTANCE_TYPE ((provider), TSC_TYPE_PROVIDER))
#define TSC_IS_PROVIDER_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_PROVIDER))
#define TSC_PROVIDER_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_PROVIDER, TSCProviderClass))

#define TSC_PROVIDER_ICON_SIZE 48

typedef struct _TSCProvider {
	GObject parent;

	const char *name;
	const char *display_name;
	const char *description;
	const char *icon_name;
	
	gboolean enabled;
} TSCProvider;

typedef struct _TSCProviderClass {
	GObjectClass parent_class;

	/* virtual methods */
	TSCConnection * (* create_connection)  (TSCProvider *provider);
	TSCConnection * (* create_from_uri)    (TSCProvider *provider,
						const char *uri);
	TSCEditDialog * (* create_edit_dialog) (TSCProvider   *provider,
						TSCConnection *connection);
} TSCProviderClass;

GType          tsc_provider_get_type             (void);

const char    *tsc_provider_get_name             (TSCProvider *provider);
const char    *tsc_provider_get_display_name     (TSCProvider *provider);
const char    *tsc_provider_get_description      (TSCProvider *provider);
gboolean       tsc_provider_is_enabled           (TSCProvider *provider);

const char    *tsc_provider_get_icon_name        (TSCProvider *provider);

TSCConnection *tsc_provider_create_connection    (TSCProvider *provider);
TSCConnection *tsc_provider_create_from_uri      (TSCProvider *provider,
						  const char  *uri);
TSCEditDialog *tsc_provider_create_edit_dialog   (TSCProvider *provider,
						  TSCConnection *connection);

gboolean       tsc_provider_can_create           (TSCProvider *provider);
gboolean       tsc_provider_can_create_from_uri  (TSCProvider *provider);
gboolean       tsc_provider_can_edit             (TSCProvider *provider);

#endif
