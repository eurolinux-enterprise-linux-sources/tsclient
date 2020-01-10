
#ifndef __TSC_RDP_EDIT_DIALOG__
#define __TSC_RDP_EDIT_DIALOG__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include "tsc-edit-dialog.h"

#define TSC_TYPE_RDP_EDIT_DIALOG                   (tsc_rdp_edit_dialog_get_type ())
#define TSC_RDP_EDIT_DIALOG(dialog )               (G_TYPE_CHECK_INSTANCE_CAST ((dialog), TSC_TYPE_RDP_EDIT_DIALOG, TSCRdpEditDialog))
#define TSC_RDP_EDIT_DIALOG_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_RDP_EDIT_DIALOG, TSCRdpEditDialogClass))
#define TSC_IS_RDP_EDIT_DIALOG(dialog)             (G_TYPE_CHECK_INSTANCE_TYPE ((dialog), TSC_TYPE_RDP_EDIT_DIALOG))
#define TSC_IS_RDP_EDIT_DIALOG_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_RDP_EDIT_DIALOG))
#define TSC_RDP_EDIT_DIALOG_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_RDP_EDIT_DIALOG, TSCRdpEditDialogClass))

typedef struct _TSCRdpEditDialog {
	TSCEditDialog parent;

	GladeXML *xml;
} TSCRdpEditDialog;

typedef struct _TSCRdpEditDialogClass {
	TSCEditDialogClass parent_class;
} TSCRdpEditDialogClass;


GType          tsc_rdp_edit_dialog_get_type        (void);

#endif
