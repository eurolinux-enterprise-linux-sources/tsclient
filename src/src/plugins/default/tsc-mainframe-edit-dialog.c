
#include <config.h>
#include <glib.h>
#include <glib/gi18n.h>
#include "tsc-mainframe-edit-dialog.h"
#include "tsc-mainframe-connection.h"
#include "tsc-util.h"

static TSCEditDialogClass *parent_class;

static void
tsc_mainframe_edit_dialog_finalize (GObject *obj)
{
	TSCMainframeEditDialog *dialog = TSC_MAINFRAME_EDIT_DIALOG (obj);

	if (dialog->xml) {
		g_object_unref (dialog->xml);
	}

	if (G_OBJECT_CLASS (parent_class)->finalize) {
		G_OBJECT_CLASS (parent_class)->finalize (obj);
	}
}

static void
tsc_mainframe_edit_dialog_load (TSCEditDialog *edit_dialog)
{
	TSCMainframeEditDialog *dialog;
	TSCMainframeConnection *connection;
	
	dialog = TSC_MAINFRAME_EDIT_DIALOG (edit_dialog);
	connection = TSC_MAINFRAME_CONNECTION (edit_dialog->connection);

	if (connection->host) {
		gtk_entry_set_text (GTK_ENTRY (dialog->host_entry),
				    connection->host);
	}
}

static void
tsc_mainframe_edit_dialog_save (TSCEditDialog *edit_dialog)
{

	TSCMainframeEditDialog *dialog;
	TSCMainframeConnection *connection;

	dialog = TSC_MAINFRAME_EDIT_DIALOG (edit_dialog);
	connection = TSC_MAINFRAME_CONNECTION (edit_dialog->connection);

	g_free (connection->host);
	connection->host = g_strdup (gtk_entry_get_text (GTK_ENTRY (dialog->host_entry)));
}

static gboolean
tsc_mainframe_edit_dialog_verify (TSCEditDialog *edit_dialog, gchar **reason)
{
	TSCMainframeEditDialog *dialog = TSC_MAINFRAME_EDIT_DIALOG (edit_dialog);
	GtkEntry *host_entry = GTK_ENTRY (glade_xml_get_widget (dialog->xml, "host_entry"));
	
	if (!g_ascii_strcasecmp ("", gtk_entry_get_text (host_entry))) {
		*reason = g_strdup (_("Host is a required field"));
		return FALSE;
	} 
	
	return TRUE;
}

static void
tsc_mainframe_edit_dialog_class_init (TSCMainframeEditDialogClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;
	TSCEditDialogClass *edit_dialog_class = (TSCEditDialogClass *) klass;
	
	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = tsc_mainframe_edit_dialog_finalize;
	edit_dialog_class->load = tsc_mainframe_edit_dialog_load;
	edit_dialog_class->save = tsc_mainframe_edit_dialog_save;
	edit_dialog_class->verify = tsc_mainframe_edit_dialog_verify;
}

static void
tsc_mainframe_edit_dialog_init (TSCMainframeEditDialog *dialog)
{
	GtkWidget *content;
	
	dialog->xml = glade_xml_new (GLADEFILE,
				     "mainframe_edit_dialog_content",
				     PACKAGE);

	dialog->host_entry = glade_xml_get_widget (dialog->xml, "host_entry");
	content = glade_xml_get_widget (dialog->xml, "mainframe_edit_dialog_content");
	gtk_widget_show_all (content);
	
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), content);
}

GType
tsc_mainframe_edit_dialog_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCMainframeEditDialogClass),
			NULL, NULL,
			(GClassInitFunc) tsc_mainframe_edit_dialog_class_init,
			NULL, NULL,
			sizeof (TSCMainframeEditDialog),
			0,
			(GInstanceInitFunc) tsc_mainframe_edit_dialog_init
		};
		
		type = g_type_register_static (TSC_TYPE_EDIT_DIALOG,
					       "TSCMainframeEditDialog",
					       &type_info, 0);
	}
	
	return type;

}
