
#include <config.h>
#include <glib.h>
#include <glib/gi18n.h>
#include "tsc-web-edit-dialog.h"
#include "tsc-web-connection.h"
#include "tsc-util.h"

static TSCEditDialogClass *parent_class;

static void
tsc_web_edit_dialog_finalize (GObject *obj)
{
	TSCWebEditDialog *dialog = TSC_WEB_EDIT_DIALOG (obj);

	if (dialog->xml) {
		g_object_unref (dialog->xml);
	}

	if (G_OBJECT_CLASS (parent_class)->finalize) {
		G_OBJECT_CLASS (parent_class)->finalize (obj);
	}
}

static void
tsc_web_edit_dialog_load (TSCEditDialog *edit_dialog)
{
	TSCWebEditDialog *dialog;
	TSCWebConnection *connection;
	
	dialog = TSC_WEB_EDIT_DIALOG (edit_dialog);
	connection = TSC_WEB_CONNECTION (edit_dialog->connection);

	if (connection->url) {
		gtk_entry_set_text (GTK_ENTRY (dialog->url_entry), connection->url);
	}
}

static void
tsc_web_edit_dialog_save (TSCEditDialog *edit_dialog)
{

	TSCWebEditDialog *dialog;
	TSCWebConnection *connection;

	dialog = TSC_WEB_EDIT_DIALOG (edit_dialog);
	connection = TSC_WEB_CONNECTION (edit_dialog->connection);

	g_free (connection->url);
	connection->url = g_strdup (gtk_entry_get_text (GTK_ENTRY (dialog->url_entry)));
}

static gboolean
tsc_web_edit_dialog_verify (TSCEditDialog *edit_dialog, gchar **reason)
{
	TSCWebEditDialog *dialog = TSC_WEB_EDIT_DIALOG (edit_dialog);
	
	if (!g_ascii_strcasecmp ("", gtk_entry_get_text (GTK_ENTRY (dialog->url_entry)))) {
		*reason = g_strdup (_("URL is a required field"));
		return FALSE;
	} 
	
	return TRUE;
}

static void
tsc_web_edit_dialog_class_init (TSCWebEditDialogClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;
	TSCEditDialogClass *edit_dialog_class = (TSCEditDialogClass *) klass;
	
	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = tsc_web_edit_dialog_finalize;
	edit_dialog_class->load = tsc_web_edit_dialog_load;
	edit_dialog_class->save = tsc_web_edit_dialog_save;
	edit_dialog_class->verify = tsc_web_edit_dialog_verify;
}

static void
tsc_web_edit_dialog_init (TSCWebEditDialog *dialog)
{
	GtkWidget *content;
	
	dialog->xml = glade_xml_new (GLADEFILE,
				     "web_edit_dialog_content",
				     PACKAGE);

	dialog->url_entry = glade_xml_get_widget (dialog->xml, "url_entry");
	content = glade_xml_get_widget (dialog->xml, "web_edit_dialog_content");
	gtk_widget_show_all (content);
	
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), content);
}

GType
tsc_web_edit_dialog_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCWebEditDialogClass),
			NULL, NULL,
			(GClassInitFunc) tsc_web_edit_dialog_class_init,
			NULL, NULL,
			sizeof (TSCWebEditDialog),
			0,
			(GInstanceInitFunc) tsc_web_edit_dialog_init
		};
		
		type = g_type_register_static (TSC_TYPE_EDIT_DIALOG,
					       "TSCWebEditDialog",
					       &type_info, 0);
	}
	
	return type;

}
