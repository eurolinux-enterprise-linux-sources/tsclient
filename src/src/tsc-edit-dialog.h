
#ifndef __TSC_EDIT_DIALOG__
#define __TSC_EDIT_DIALOG__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include "tsc-connection.h"

#define TSC_TYPE_EDIT_DIALOG                   (tsc_edit_dialog_get_type ())
#define TSC_EDIT_DIALOG(dialog )               (G_TYPE_CHECK_INSTANCE_CAST ((dialog), TSC_TYPE_EDIT_DIALOG, TSCEditDialog))
#define TSC_EDIT_DIALOG_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_EDIT_DIALOG, TSCEditDialogClass))
#define TSC_IS_EDIT_DIALOG(dialog)             (G_TYPE_CHECK_INSTANCE_TYPE ((dialog), TSC_TYPE_EDIT_DIALOG))
#define TSC_IS_EDIT_DIALOG_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_EDIT_DIALOG))
#define TSC_EDIT_DIALOG_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_EDIT_DIALOG, TSCEditDialogClass))

typedef struct _TSCEditDialog {
	GtkDialog parent;

	GladeXML *xml;
	GtkWidget *content;
	GtkWidget *name_entry;
	GtkWidget *policy_combo;
	GtkWidget *autostart_checkbox;
	GtkWidget *shortcut_checkbox;
	GtkWidget *error_label;
	
	TSCConnection *connection;
} TSCEditDialog;

typedef struct _TSCEditDialogClass {
	GtkDialogClass parent_class;

	/* virtual methods */

	/* verify the data in UI elements */
	gboolean (* verify) (TSCEditDialog *dialog, gchar **reason);
	
	/* load settings from the connection into UI elements */
	void (* load) (TSCEditDialog *dialog);

	/* put settings from UI elements into connection */
	void (* save) (TSCEditDialog *dialog);
} TSCEditDialogClass;


GType          tsc_edit_dialog_get_type        (void);

TSCEditDialog *tsc_edit_dialog_new             (GtkWindow *parent,
						TSCConnection *connection);

#endif
