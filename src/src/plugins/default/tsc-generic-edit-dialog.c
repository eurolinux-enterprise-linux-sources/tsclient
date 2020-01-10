
#include <config.h>
#include <glib.h>
#include <glib/gi18n.h>
#include "tsc-generic-edit-dialog.h"
#include "tsc-generic-connection.h"
#include "tsc-util.h"

static TSCEditDialogClass *parent_class;

static void
tsc_generic_edit_dialog_finalize (GObject *obj)
{
	TSCGenericEditDialog *dialog = TSC_GENERIC_EDIT_DIALOG (obj);

	if (dialog->xml) {
		g_object_unref (dialog->xml);
	}

	if (G_OBJECT_CLASS (parent_class)->finalize) {
		G_OBJECT_CLASS (parent_class)->finalize (obj);
	}
}

static void
tsc_generic_edit_dialog_load (TSCEditDialog *edit_dialog)
{
	TSCGenericEditDialog *dialog;
	TSCGenericConnection *connection;
	
	dialog = TSC_GENERIC_EDIT_DIALOG (edit_dialog);
	connection = TSC_GENERIC_CONNECTION (edit_dialog->connection);

	if (connection->command_line) {
		gtk_entry_set_text (GTK_ENTRY (dialog->command_entry),
				    connection->command_line);
	}

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->terminal_checkbox),
				      connection->use_terminal);
}

static void
tsc_generic_edit_dialog_save (TSCEditDialog *edit_dialog)
{

	TSCGenericEditDialog *dialog;
	TSCGenericConnection *connection;

	dialog = TSC_GENERIC_EDIT_DIALOG (edit_dialog);
	connection = TSC_GENERIC_CONNECTION (edit_dialog->connection);

	g_free (connection->command_line);
	connection->command_line = g_strdup (gtk_entry_get_text (GTK_ENTRY (dialog->command_entry)));

	connection->use_terminal = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->terminal_checkbox));
}

static gboolean
tsc_generic_edit_dialog_verify (TSCEditDialog *edit_dialog, gchar **reason)
{
	TSCGenericEditDialog *dialog = TSC_GENERIC_EDIT_DIALOG (edit_dialog);
	GtkEntry *command_entry = (GtkEntry *) glade_xml_get_widget (dialog->xml, "command_entry");
	
	if (!g_ascii_strcasecmp ("", gtk_entry_get_text (command_entry))) {
		*reason = g_strdup (_("Command is a required field"));
		return FALSE;
	}

	return TRUE;
}

static void
tsc_generic_edit_dialog_class_init (TSCGenericEditDialogClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;
	TSCEditDialogClass *edit_dialog_class = (TSCEditDialogClass *) klass;
	
	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = tsc_generic_edit_dialog_finalize;
	edit_dialog_class->load = tsc_generic_edit_dialog_load;
	edit_dialog_class->save = tsc_generic_edit_dialog_save;
	edit_dialog_class->verify = tsc_generic_edit_dialog_verify;
}

static void
tsc_generic_edit_dialog_init (TSCGenericEditDialog *dialog)
{
	GtkWidget *content;
	
	dialog->xml = glade_xml_new (GLADEFILE,
				     "generic_edit_dialog_content",
				     PACKAGE);

	dialog->command_entry = glade_xml_get_widget (dialog->xml, "command_entry");
	dialog->terminal_checkbox = glade_xml_get_widget (dialog->xml, "terminal_checkbox");
	content = glade_xml_get_widget (dialog->xml, "generic_edit_dialog_content");
	gtk_widget_show_all (content);
	
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), content);
}

GType
tsc_generic_edit_dialog_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCGenericEditDialogClass),
			NULL, NULL,
			(GClassInitFunc) tsc_generic_edit_dialog_class_init,
			NULL, NULL,
			sizeof (TSCGenericEditDialog),
			0,
			(GInstanceInitFunc) tsc_generic_edit_dialog_init
		};
		
		type = g_type_register_static (TSC_TYPE_EDIT_DIALOG,
					       "TSCGenericEditDialog",
					       &type_info, 0);
	}
	
	return type;

}
