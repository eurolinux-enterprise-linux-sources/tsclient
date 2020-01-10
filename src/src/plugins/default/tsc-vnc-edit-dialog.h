
#ifndef __TSC_VNC_EDIT_DIALOG__
#define __TSC_VNC_EDIT_DIALOG__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include "tsc-edit-dialog.h"

#define TSC_TYPE_VNC_EDIT_DIALOG                   (tsc_vnc_edit_dialog_get_type ())
#define TSC_VNC_EDIT_DIALOG(dialog )               (G_TYPE_CHECK_INSTANCE_CAST ((dialog), TSC_TYPE_VNC_EDIT_DIALOG, TSCVncEditDialog))
#define TSC_VNC_EDIT_DIALOG_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_VNC_EDIT_DIALOG, TSCVncEditDialogClass))
#define TSC_IS_VNC_EDIT_DIALOG(dialog)             (G_TYPE_CHECK_INSTANCE_TYPE ((dialog), TSC_TYPE_VNC_EDIT_DIALOG))
#define TSC_IS_VNC_EDIT_DIALOG_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_VNC_EDIT_DIALOG))
#define TSC_VNC_EDIT_DIALOG_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_VNC_EDIT_DIALOG, TSCVncEditDialogClass))

typedef struct _TSCVncEditDialog {
	TSCEditDialog parent;

	GladeXML *xml;
} TSCVncEditDialog;

typedef struct _TSCVncEditDialogClass {
	TSCEditDialogClass parent_class;
} TSCVncEditDialogClass;


GType          tsc_vnc_edit_dialog_get_type        (void);

#endif
