
#ifndef __TSC_XDMCP_EDIT_DIALOG__
#define __TSC_XDMCP_EDIT_DIALOG__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include "tsc-edit-dialog.h"

#define TSC_TYPE_XDMCP_EDIT_DIALOG                   (tsc_xdmcp_edit_dialog_get_type ())
#define TSC_XDMCP_EDIT_DIALOG(dialog )               (G_TYPE_CHECK_INSTANCE_CAST ((dialog), TSC_TYPE_XDMCP_EDIT_DIALOG, TSCXdmcpEditDialog))
#define TSC_XDMCP_EDIT_DIALOG_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_XDMCP_EDIT_DIALOG, TSCXdmcpEditDialogClass))
#define TSC_IS_XDMCP_EDIT_DIALOG(dialog)             (G_TYPE_CHECK_INSTANCE_TYPE ((dialog), TSC_TYPE_XDMCP_EDIT_DIALOG))
#define TSC_IS_XDMCP_EDIT_DIALOG_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_XDMCP_EDIT_DIALOG))
#define TSC_XDMCP_EDIT_DIALOG_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_XDMCP_EDIT_DIALOG, TSCXdmcpEditDialogClass))

typedef struct _TSCXdmcpEditDialog {
	TSCEditDialog parent;

	GladeXML *xml;
	GtkWidget *host_entry;
} TSCXdmcpEditDialog;

typedef struct _TSCXdmcpEditDialogClass {
	TSCEditDialogClass parent_class;
} TSCXdmcpEditDialogClass;


GType          tsc_xdmcp_edit_dialog_get_type        (void);

#endif
