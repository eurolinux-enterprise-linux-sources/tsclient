
#include <config.h>
#include <glib.h>
#include <glib/gi18n.h>
#include "tsc-xdmcp-edit-dialog.h"
#include "tsc-xdmcp-connection.h"
#include "tsc-util.h"

static TSCEditDialogClass *parent_class;

static void
tsc_xdmcp_edit_dialog_finalize (GObject *obj)
{
	TSCXdmcpEditDialog *dialog = TSC_XDMCP_EDIT_DIALOG (obj);

	if (dialog->xml) {
		g_object_unref (dialog->xml);
	}

	if (G_OBJECT_CLASS (parent_class)->finalize) {
		G_OBJECT_CLASS (parent_class)->finalize (obj);
	}
}

static void
custom_window_toggle_cb (GtkToggleButton  *button,
			 TSCXdmcpEditDialog *dialog)
{
	gtk_widget_set_sensitive (glade_xml_get_widget (dialog->xml,
							"xdmcp_custom_window_box"),
				  gtk_toggle_button_get_active (button));
}

static void
tsc_xdmcp_edit_dialog_load (TSCEditDialog *edit_dialog)
{
	TSCXdmcpEditDialog *dialog;
	TSCXdmcpConnection *connection;
	GdkScreen *screen;
	int width, height;
	
	dialog = TSC_XDMCP_EDIT_DIALOG (edit_dialog);
	connection = TSC_XDMCP_CONNECTION (edit_dialog->connection);

	if (connection->host) {
		gtk_entry_set_text (GTK_ENTRY (dialog->host_entry), connection->host);
	}

	if (connection->fullscreen) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (dialog->xml,
										       "xdmcp_fullscreen_radio")),
					      TRUE);
	} else {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (dialog->xml,
										       "xdmcp_custom_window_radio")),
					      TRUE);
	}
	
	custom_window_toggle_cb (GTK_TOGGLE_BUTTON (glade_xml_get_widget (dialog->xml,
				 									"xdmcp_custom_window_radio")), dialog);

	screen = gtk_widget_get_screen (GTK_WIDGET (dialog));
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (glade_xml_get_widget (dialog->xml,
									  "xdmcp_width_spin")),
				   1, gdk_screen_get_width (screen));
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (glade_xml_get_widget (dialog->xml,
									  "xdmcp_height_spin")),
				   1, gdk_screen_get_height (screen));


	if (connection->fullscreen || connection->width <= 0 ||
		connection->width > gdk_screen_get_width (screen)) {
		width = gdk_screen_get_width (screen);
	} else {
		width = connection->width;
	}

	if (connection->fullscreen || connection->height <= 0 ||
		connection->height > gdk_screen_get_height (screen)) {
		height = gdk_screen_get_height (screen);
	} else {
		height = connection->height;
	}

	gtk_spin_button_set_value (GTK_SPIN_BUTTON (glade_xml_get_widget (dialog->xml,
									  "xdmcp_width_spin")),
				   (gdouble) width);
	
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (glade_xml_get_widget (dialog->xml,
									  "xdmcp_height_spin")),
				   (gdouble) height);
}

static void
tsc_xdmcp_edit_dialog_save (TSCEditDialog *edit_dialog)
{

	TSCXdmcpEditDialog *dialog;
	TSCXdmcpConnection *connection;

	dialog = TSC_XDMCP_EDIT_DIALOG (edit_dialog);
	connection = TSC_XDMCP_CONNECTION (edit_dialog->connection);
	g_free (connection->host);
	
	connection->host = g_strdup (gtk_entry_get_text (GTK_ENTRY (dialog->host_entry)));
	connection->fullscreen = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (
								       glade_xml_get_widget (dialog->xml,
											     "xdmcp_fullscreen_radio")));
	connection->width = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (
								      glade_xml_get_widget (dialog->xml,
											    "xdmcp_width_spin")));
	connection->height = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (
								       glade_xml_get_widget (dialog->xml,
											     "xdmcp_height_spin")));
}

static gboolean
tsc_xdmcp_edit_dialog_verify (TSCEditDialog *edit_dialog, gchar **reason)
{
	TSCXdmcpEditDialog *dialog = TSC_XDMCP_EDIT_DIALOG (edit_dialog);
	
	if (!g_ascii_strcasecmp ("", gtk_entry_get_text (GTK_ENTRY (dialog->host_entry)))) {
		*reason = g_strdup (_("Host is a required field"));
		return FALSE;
	} 
	
	return TRUE;
}

static void
tsc_xdmcp_edit_dialog_class_init (TSCXdmcpEditDialogClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;
	TSCEditDialogClass *edit_dialog_class = (TSCEditDialogClass *) klass;
	
	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = tsc_xdmcp_edit_dialog_finalize;
	edit_dialog_class->load = tsc_xdmcp_edit_dialog_load;
	edit_dialog_class->save = tsc_xdmcp_edit_dialog_save;
	edit_dialog_class->verify = tsc_xdmcp_edit_dialog_verify;
}

static void
tsc_xdmcp_edit_dialog_init (TSCXdmcpEditDialog *dialog)
{
	GtkWidget *content;
	
	dialog->xml = glade_xml_new (GLADEFILE,
				     "xdmcp_edit_dialog_content",
				     PACKAGE);

	g_signal_connect (glade_xml_get_widget (dialog->xml,
						"xdmcp_custom_window_radio"),
			  "toggled",
			  G_CALLBACK (custom_window_toggle_cb),
			  dialog);

	dialog->host_entry = glade_xml_get_widget (dialog->xml, "host_entry");
	content = glade_xml_get_widget (dialog->xml, "xdmcp_edit_dialog_content");
	gtk_widget_show_all (content);
	
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), content);
}

GType
tsc_xdmcp_edit_dialog_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCXdmcpEditDialogClass),
			NULL, NULL,
			(GClassInitFunc) tsc_xdmcp_edit_dialog_class_init,
			NULL, NULL,
			sizeof (TSCXdmcpEditDialog),
			0,
			(GInstanceInitFunc) tsc_xdmcp_edit_dialog_init
		};
		
		type = g_type_register_static (TSC_TYPE_EDIT_DIALOG,
					       "TSCXdmcpEditDialog",
					       &type_info, 0);
	}
	
	return type;
}
