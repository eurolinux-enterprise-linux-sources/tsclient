
#include <config.h>
#include <glib.h>
#include <glib/gi18n.h>
#include "tsc-vnc-edit-dialog.h"
#include "tsc-vnc-connection.h"
#include "tsc-util.h"

static TSCEditDialogClass *parent_class;

static void
tsc_vnc_edit_dialog_finalize (GObject *obj)
{
	TSCVncEditDialog *dialog = TSC_VNC_EDIT_DIALOG (obj);

	if (dialog->xml) {
		g_object_unref (dialog->xml);
	}

	if (G_OBJECT_CLASS (parent_class)->finalize) {
		G_OBJECT_CLASS (parent_class)->finalize (obj);
	}
}

static void
custom_window_toggle_cb (GtkToggleButton  *button,
			 TSCVncEditDialog *dialog)
{
	gtk_widget_set_sensitive (glade_xml_get_widget (dialog->xml,
							"vnc_custom_window_box"),
				  gtk_toggle_button_get_active (button));
}

static void
tsc_vnc_edit_dialog_load (TSCEditDialog *edit_dialog)
{
	TSCVncEditDialog *dialog;
	TSCVncConnection *connection;
	GdkScreen *screen;
	int width, height;
	
	dialog = TSC_VNC_EDIT_DIALOG (edit_dialog);
	connection = TSC_VNC_CONNECTION (edit_dialog->connection);

	if (connection->host) {
		gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (dialog->xml, "host_entry")),
				    connection->host);
	}

	if (connection->password) {
		gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (dialog->xml, "password_entry")),
				    connection->password);
	}

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (dialog->xml,
									       "shared_checkbox")),
				      connection->shared);
	
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (dialog->xml,
									       "viewonly_checkbox")),
				      connection->viewonly);

	if (connection->fullscreen ) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (dialog->xml,
										       "vnc_fullscreen_radio")),
					      TRUE);
	} else {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (dialog->xml,
										       "vnc_custom_window_radio")),
					      TRUE);
	}

	/* ugh, why isn't this triggered when we do _set_active above? */
	custom_window_toggle_cb (GTK_TOGGLE_BUTTON (glade_xml_get_widget (dialog->xml,
									  "vnc_custom_window_radio")),
				 dialog);

	/* setup ranges for width/height spin buttons */
	screen = gtk_widget_get_screen (GTK_WIDGET (dialog));
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (glade_xml_get_widget (dialog->xml,
									  "vnc_width_spin")),
				   1, gdk_screen_get_width (screen));
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (glade_xml_get_widget (dialog->xml,
									  "vnc_height_spin")),
				   1, gdk_screen_get_height (screen));


	if (connection->width <= 0 || connection->width > gdk_screen_get_width (screen)) {
		width = gdk_screen_get_width (screen);
	} else {
		width = connection->width;
	}

	if (connection->height <= 0 || connection->height > gdk_screen_get_height (screen)) {
		height = gdk_screen_get_height (screen);
	} else {
		height = connection->height;
	}

	gtk_spin_button_set_value (GTK_SPIN_BUTTON (glade_xml_get_widget (dialog->xml,
									  "vnc_width_spin")),
				   (gdouble) width);
	
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (glade_xml_get_widget (dialog->xml,
									  "vnc_height_spin")),
				   (gdouble) height);
}

static void
tsc_vnc_edit_dialog_save (TSCEditDialog *edit_dialog)
{

	TSCVncEditDialog *dialog;
	TSCVncConnection *connection;

	dialog = TSC_VNC_EDIT_DIALOG (edit_dialog);
	connection = TSC_VNC_CONNECTION (edit_dialog->connection);

	g_free (connection->host);
	connection->host = g_strdup (gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (dialog->xml,
											  "host_entry"))));
	g_free (connection->password);
	connection->password = g_strdup (gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (dialog->xml,
											  "password_entry"))));
	NULLIFY_IF_EMPTY (connection->password);

	connection->shared = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (
								   glade_xml_get_widget (dialog->xml,
											 "shared_checkbox")));
	connection->viewonly = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (
								     glade_xml_get_widget (dialog->xml,
											   "viewonly_checkbox")));
	connection->fullscreen = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (
								       glade_xml_get_widget (dialog->xml,
											     "vnc_fullscreen_radio")));
	connection->width = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (
								      glade_xml_get_widget (dialog->xml,
											    "vnc_width_spin")));
	connection->height = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (
								       glade_xml_get_widget (dialog->xml,
											     "vnc_height_spin")));
}

static gboolean
tsc_vnc_edit_dialog_verify (TSCEditDialog *edit_dialog, gchar **reason)
{
	TSCVncEditDialog *dialog = TSC_VNC_EDIT_DIALOG (edit_dialog);
	GtkEntry *host_entry = GTK_ENTRY (glade_xml_get_widget (dialog->xml, "host_entry"));
	GtkEntry *password_entry = GTK_ENTRY (glade_xml_get_widget (dialog->xml, "password_entry"));
	
	if (!g_ascii_strcasecmp ("", gtk_entry_get_text (host_entry))) {
		*reason = g_strdup (_("Host is a required field"));
		 return FALSE;
	} 
	
	if (!g_ascii_strcasecmp ("", gtk_entry_get_text (password_entry))) {
		*reason = g_strdup (_("Password is a required field"));
		return FALSE;
	}

	return TRUE;
}

static void
tsc_vnc_edit_dialog_class_init (TSCVncEditDialogClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;
	TSCEditDialogClass *edit_dialog_class = (TSCEditDialogClass *) klass;
	
	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = tsc_vnc_edit_dialog_finalize;
	edit_dialog_class->load = tsc_vnc_edit_dialog_load;
	edit_dialog_class->save = tsc_vnc_edit_dialog_save;
	edit_dialog_class->verify = tsc_vnc_edit_dialog_verify;
}

static void
tsc_vnc_edit_dialog_init (TSCVncEditDialog *dialog)
{
	GtkWidget *content;

	dialog->xml = glade_xml_new (GLADEFILE,
				     "vnc_edit_dialog_content",
				     PACKAGE);

	g_signal_connect (glade_xml_get_widget (dialog->xml,
						"vnc_custom_window_radio"),
			  "toggled",
			  G_CALLBACK (custom_window_toggle_cb),
			  dialog);
	
	content = glade_xml_get_widget (dialog->xml, "vnc_edit_dialog_content");
	gtk_widget_show_all (content);
	
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), content);
}

GType
tsc_vnc_edit_dialog_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCVncEditDialogClass),
			NULL, NULL,
			(GClassInitFunc) tsc_vnc_edit_dialog_class_init,
			NULL, NULL,
			sizeof (TSCVncEditDialog),
			0,
			(GInstanceInitFunc) tsc_vnc_edit_dialog_init
		};
		
		type = g_type_register_static (TSC_TYPE_EDIT_DIALOG,
					       "TSCVncEditDialog",
					       &type_info, 0);
	}
	
	return type;

}
