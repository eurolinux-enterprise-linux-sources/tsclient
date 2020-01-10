
#ifndef __TSC_WINDOW__
#define __TSC_WINDOW__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include "tsc-manager.h"

#define TSC_TYPE_WINDOW                   (tsc_window_get_type ())
#define TSC_WINDOW(window)                (G_TYPE_CHECK_INSTANCE_CAST ((window), TSC_TYPE_WINDOW, TSCWindow))
#define TSC_WINDOW_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_WINDOW, TSCWindowClass))
#define TSC_IS_WINDOW(window)             (G_TYPE_CHECK_INSTANCE_TYPE ((window), TSC_TYPE_WINDOW))
#define TSC_IS_WINDOW_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_WINDOW))
#define TSC_WINDOW_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_WINDOW, TSCWindowClass))

typedef struct _TSCWindow {
	GtkWindow parent;

	TSCManager *manager;
	
	GladeXML *xml;
	GtkWidget *icon_view;

	GtkTreeModel *model;

	GtkUIManager *ui_manager;
	GtkActionGroup *connection_actions;
	GtkActionGroup *main_actions;
	gboolean chooser_only;

	GList *drag_files;
} TSCWindow;

typedef struct _TSCWindowClass {
	GtkWindowClass parent_class;
} TSCWindowClass;


GType       tsc_window_get_type        (void);

GtkWidget  *tsc_window_new             (TSCManager *manager,
					gboolean    chooser_only);

#endif
