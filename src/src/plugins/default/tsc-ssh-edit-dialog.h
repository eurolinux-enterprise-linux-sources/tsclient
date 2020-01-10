
#ifndef __TSC_SSH_EDIT_DIALOG__
#define __TSC_SSH_EDIT_DIALOG__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include "tsc-edit-dialog.h"

#define TSC_TYPE_SSH_EDIT_DIALOG                   (tsc_ssh_edit_dialog_get_type ())
#define TSC_SSH_EDIT_DIALOG(dialog )               (G_TYPE_CHECK_INSTANCE_CAST ((dialog), TSC_TYPE_SSH_EDIT_DIALOG, TSCSshEditDialog))
#define TSC_SSH_EDIT_DIALOG_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_SSH_EDIT_DIALOG, TSCSshEditDialogClass))
#define TSC_IS_SSH_EDIT_DIALOG(dialog)             (G_TYPE_CHECK_INSTANCE_TYPE ((dialog), TSC_TYPE_SSH_EDIT_DIALOG))
#define TSC_IS_SSH_EDIT_DIALOG_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_SSH_EDIT_DIALOG))
#define TSC_SSH_EDIT_DIALOG_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_SSH_EDIT_DIALOG, TSCSshEditDialogClass))

typedef struct _TSCSshEditDialog {
	TSCEditDialog parent;

	GladeXML *xml;
	GtkWidget *host_entry;
	GtkWidget *user_entry;
	GtkWidget *password_entry;
	GtkWidget *options_entry;
	GtkWidget *remote_entry;
	GtkWidget *terminal_checkbox;
	GtkWidget *x11_checkbox;
} TSCSshEditDialog;

typedef struct _TSCSshEditDialogClass {
	TSCEditDialogClass parent_class;
} TSCSshEditDialogClass;


GType          tsc_ssh_edit_dialog_get_type        (void);

#endif
