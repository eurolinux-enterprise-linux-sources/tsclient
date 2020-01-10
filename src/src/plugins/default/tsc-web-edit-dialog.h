
#ifndef __TSC_WEB_EDIT_DIALOG__
#define __TSC_WEB_EDIT_DIALOG__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include "tsc-edit-dialog.h"

#define TSC_TYPE_WEB_EDIT_DIALOG                   (tsc_web_edit_dialog_get_type ())
#define TSC_WEB_EDIT_DIALOG(dialog)                (G_TYPE_CHECK_INSTANCE_CAST ((dialog), TSC_TYPE_WEB_EDIT_DIALOG, TSCWebEditDialog))
#define TSC_WEB_EDIT_DIALOG_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_WEB_EDIT_DIALOG, TSCWebEditDialogClass))
#define TSC_IS_WEB_EDIT_DIALOG(dialog)             (G_TYPE_CHECK_INSTANCE_TYPE ((dialog), TSC_TYPE_WEB_EDIT_DIALOG))
#define TSC_IS_WEB_EDIT_DIALOG_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_WEB_EDIT_DIALOG))
#define TSC_WEB_EDIT_DIALOG_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_WEB_EDIT_DIALOG, TSCWebEditDialogClass))

typedef struct _TSCWebEditDialog {
	TSCEditDialog parent;

	GladeXML *xml;
	GtkWidget *url_entry;
} TSCWebEditDialog;

typedef struct _TSCWebEditDialogClass {
	TSCEditDialogClass parent_class;
} TSCWebEditDialogClass;


GType          tsc_web_edit_dialog_get_type        (void);

#endif
