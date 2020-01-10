
#ifndef __TSC_MAINFRAME_EDIT_DIALOG__
#define __TSC_MAINFRAME_EDIT_DIALOG__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include "tsc-edit-dialog.h"

#define TSC_TYPE_MAINFRAME_EDIT_DIALOG                   (tsc_mainframe_edit_dialog_get_type ())
#define TSC_MAINFRAME_EDIT_DIALOG(dialog )               (G_TYPE_CHECK_INSTANCE_CAST ((dialog), TSC_TYPE_MAINFRAME_EDIT_DIALOG, TSCMainframeEditDialog))
#define TSC_MAINFRAME_EDIT_DIALOG_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_MAINFRAME_EDIT_DIALOG, TSCMainframeEditDialogClass))
#define TSC_IS_MAINFRAME_EDIT_DIALOG(dialog)             (G_TYPE_CHECK_INSTANCE_TYPE ((dialog), TSC_TYPE_MAINFRAME_EDIT_DIALOG))
#define TSC_IS_MAINFRAME_EDIT_DIALOG_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_MAINFRAME_EDIT_DIALOG))
#define TSC_MAINFRAME_EDIT_DIALOG_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_MAINFRAME_EDIT_DIALOG, TSCMainframeEditDialogClass))

typedef struct _TSCMainframeEditDialog {
	TSCEditDialog parent;

	GladeXML *xml;
	GtkWidget *host_entry;
	GtkWidget *port_entry;
} TSCMainframeEditDialog;

typedef struct _TSCMainframeEditDialogClass {
	TSCEditDialogClass parent_class;
} TSCMainframeEditDialogClass;


GType          tsc_mainframe_edit_dialog_get_type        (void);

#endif
