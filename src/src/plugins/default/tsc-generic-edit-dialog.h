
#ifndef __TSC_GENERIC_EDIT_DIALOG__
#define __TSC_GENERIC_EDIT_DIALOG__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include "tsc-edit-dialog.h"

#define TSC_TYPE_GENERIC_EDIT_DIALOG                   (tsc_generic_edit_dialog_get_type ())
#define TSC_GENERIC_EDIT_DIALOG(dialog )               (G_TYPE_CHECK_INSTANCE_CAST ((dialog), TSC_TYPE_GENERIC_EDIT_DIALOG, TSCGenericEditDialog))
#define TSC_GENERIC_EDIT_DIALOG_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_GENERIC_EDIT_DIALOG, TSCGenericEditDialogClass))
#define TSC_IS_GENERIC_EDIT_DIALOG(dialog)             (G_TYPE_CHECK_INSTANCE_TYPE ((dialog), TSC_TYPE_GENERIC_EDIT_DIALOG))
#define TSC_IS_GENERIC_EDIT_DIALOG_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_GENERIC_EDIT_DIALOG))
#define TSC_GENERIC_EDIT_DIALOG_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_GENERIC_EDIT_DIALOG, TSCGenericEditDialogClass))

typedef struct _TSCGenericEditDialog {
	TSCEditDialog parent;

	GladeXML *xml;
	GtkWidget *command_entry;
	GtkWidget *terminal_checkbox;
} TSCGenericEditDialog;

typedef struct _TSCGenericEditDialogClass {
	TSCEditDialogClass parent_class;
} TSCGenericEditDialogClass;


GType          tsc_generic_edit_dialog_get_type        (void);

#endif
