
#include <config.h>
#include <glib.h>
#include <glib/gi18n.h>
#include "tsc-ssh-edit-dialog.h"
#include "tsc-ssh-connection.h"
#include "tsc-util.h"

static TSCEditDialogClass *parent_class;

static void
tsc_ssh_edit_dialog_finalize (GObject *obj)
{
	TSCSshEditDialog *dialog = TSC_SSH_EDIT_DIALOG (obj);

	if (dialog->xml) {
		g_object_unref (dialog->xml);
	}

	if (G_OBJECT_CLASS (parent_class)->finalize) {
		G_OBJECT_CLASS (parent_class)->finalize (obj);
	}
}

static gboolean
tsc_ssh_edit_dialog_verify (TSCEditDialog *edit_dialog, gchar **reason)
{
	TSCSshEditDialog *dialog = TSC_SSH_EDIT_DIALOG (edit_dialog);

	if (!g_ascii_strcasecmp ("", gtk_entry_get_text (GTK_ENTRY (dialog->host_entry)))) {
		*reason = g_strdup (_("Host is a required field"));
		return FALSE;
	}

	if (!g_ascii_strcasecmp ("", gtk_entry_get_text (GTK_ENTRY (dialog->user_entry)))) {
		*reason = g_strdup (_("User is a required field"));
		return FALSE;
	} 

	if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->terminal_checkbox)) &&
		!g_ascii_strcasecmp ("", gtk_entry_get_text (GTK_ENTRY (dialog->remote_entry)))) {
		*reason = g_strdup (_("You must specify a command if you're not using a terminal"));
		return FALSE;
	}

	return TRUE;
}

static void
tsc_ssh_edit_dialog_load (TSCEditDialog *edit_dialog)
{
	TSCSshEditDialog *dialog;
	TSCSshConnection *connection;
	
	dialog = TSC_SSH_EDIT_DIALOG (edit_dialog);
	connection = TSC_SSH_CONNECTION (edit_dialog->connection);

	if (connection->host_line) {
		gtk_entry_set_text (GTK_ENTRY (dialog->host_entry),
				    connection->host_line);
	}

	if (connection->user_line) {
		gtk_entry_set_text (GTK_ENTRY (dialog->user_entry),
				    connection->user_line);
	}

	if (connection->password_line) {
		gtk_entry_set_text (GTK_ENTRY (dialog->password_entry),
				    connection->password_line);
	}
	
	if (connection->options_line) {
		gtk_entry_set_text (GTK_ENTRY (dialog->options_entry),
				    connection->options_line);
	}
	
	if (connection->remote_line) {
		gtk_entry_set_text (GTK_ENTRY (dialog->remote_entry),
				    connection->remote_line);
	}
	
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->x11_checkbox),
				      connection->use_x11);
	
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->terminal_checkbox),
				      connection->use_terminal);
}

static void
tsc_ssh_edit_dialog_save (TSCEditDialog *edit_dialog)
{

	TSCSshEditDialog *dialog;
	TSCSshConnection *connection;

	dialog = TSC_SSH_EDIT_DIALOG (edit_dialog);
	connection = TSC_SSH_CONNECTION (edit_dialog->connection);

	g_free (connection->host_line);
	connection->host_line = g_strdup (gtk_entry_get_text (GTK_ENTRY (dialog->host_entry)));

	g_free (connection->user_line);
	connection->user_line = g_strdup (gtk_entry_get_text (GTK_ENTRY (dialog->user_entry)));
	
	g_free (connection->password_line);
	connection->password_line = g_strdup (gtk_entry_get_text (GTK_ENTRY (dialog->password_entry)));
	
	g_free (connection->options_line);
	connection->options_line = g_strdup (gtk_entry_get_text (GTK_ENTRY (dialog->options_entry)));
	
	g_free (connection->remote_line);
	connection->remote_line = g_strdup (gtk_entry_get_text (GTK_ENTRY (dialog->remote_entry)));
	
	connection->use_x11 = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->x11_checkbox));
	connection->use_terminal = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->terminal_checkbox));
}

static void
tsc_ssh_edit_dialog_class_init (TSCSshEditDialogClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;
	TSCEditDialogClass *edit_dialog_class = (TSCEditDialogClass *) klass;
	
	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = tsc_ssh_edit_dialog_finalize;
	edit_dialog_class->load = tsc_ssh_edit_dialog_load;
	edit_dialog_class->save = tsc_ssh_edit_dialog_save;
	edit_dialog_class->verify = tsc_ssh_edit_dialog_verify;
}

static void
tsc_ssh_edit_dialog_init (TSCSshEditDialog *dialog)
{
	GtkWidget *content;
	
	dialog->xml = glade_xml_new (GLADEFILE,
				     "ssh_edit_dialog_content",
				     PACKAGE);

	dialog->host_entry = glade_xml_get_widget (dialog->xml, "host_entry");
	dialog->user_entry = glade_xml_get_widget (dialog->xml, "user_entry");
	dialog->password_entry = glade_xml_get_widget (dialog->xml, "password_entry");
	dialog->options_entry = glade_xml_get_widget (dialog->xml, "options_entry");
	dialog->remote_entry = glade_xml_get_widget (dialog->xml, "remote_entry");
	dialog->x11_checkbox = glade_xml_get_widget (dialog->xml, "x11_checkbox");
	dialog->terminal_checkbox = glade_xml_get_widget (dialog->xml, "terminal_checkbox");

	content = glade_xml_get_widget (dialog->xml, "ssh_edit_dialog_content");
	gtk_widget_show_all (content);
	
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), content);
}

GType
tsc_ssh_edit_dialog_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCSshEditDialogClass),
			NULL, NULL,
			(GClassInitFunc) tsc_ssh_edit_dialog_class_init,
			NULL, NULL,
			sizeof (TSCSshEditDialog),
			0,
			(GInstanceInitFunc) tsc_ssh_edit_dialog_init
		};
		
		type = g_type_register_static (TSC_TYPE_EDIT_DIALOG,
					       "TSCSshEditDialog",
					       &type_info, 0);
	}
	
	return type;

}
