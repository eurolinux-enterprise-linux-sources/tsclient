
#include <config.h>
#include <glib.h>
#include <glib/gi18n.h>
#include "tsc-rdp-edit-dialog.h"
#include "tsc-rdp-connection.h"
#include "tsc-util.h"

static TSCEditDialogClass *parent_class;

static void
tsc_rdp_edit_dialog_finalize (GObject *obj)
{
	TSCRdpEditDialog *dialog = TSC_RDP_EDIT_DIALOG (obj);

	if (dialog->xml) {
		g_object_unref (dialog->xml);
	}

	if (G_OBJECT_CLASS (parent_class)->finalize) {
		G_OBJECT_CLASS (parent_class)->finalize (obj);
	}
}

static void
custom_window_toggle_cb (GtkToggleButton  *button,
			 TSCRdpEditDialog *dialog)
{
	gtk_widget_set_sensitive (glade_xml_get_widget (dialog->xml,
							"rdp_custom_window_box"),
				  gtk_toggle_button_get_active (button));
}

static void
tsc_rdp_edit_dialog_load (TSCEditDialog *edit_dialog)
{
	TSCRdpEditDialog *dialog;
	TSCRdpConnection *connection;
	int experience_index = 0, width, height, bpp_index;
	GdkScreen *screen;
	
	dialog = TSC_RDP_EDIT_DIALOG (edit_dialog);
	connection = TSC_RDP_CONNECTION (edit_dialog->connection);

	if (connection->host) {
		gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (dialog->xml, "host_entry")),
				    connection->host);
	}

	if (connection->username) {
		gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (dialog->xml, "username_entry")),
				    connection->username);
	}

	if (connection->password) {
		gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (dialog->xml, "password_entry")),
				    connection->password);
	}

	if (connection->domain) {
		gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (dialog->xml, "domain_entry")),
				    connection->domain);
	}

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (dialog->xml, "sound_check")),
				      connection->sound);

	switch (connection->experience) {
	case TSC_RDP_EXPERIENCE_DEFAULT:
		experience_index = 0;
		break;
	case TSC_RDP_EXPERIENCE_MODEM:
		experience_index = 1;
		break;
	case TSC_RDP_EXPERIENCE_BROADBAND:
		experience_index = 2;
		break;
	case TSC_RDP_EXPERIENCE_LAN:
		experience_index = 3;
		break;
	}

	gtk_combo_box_set_active (GTK_COMBO_BOX (glade_xml_get_widget (dialog->xml,
								       "experience_combo")),
				  experience_index);

	if (connection->fullscreen) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (dialog->xml,
										       "rdp_fullscreen_radio")),
					      TRUE);
	} else {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (dialog->xml,
										       "rdp_custom_window_radio")),
					      TRUE);
		
	}

	/* ugh, why isn't this triggered when we do _set_active above? */
	custom_window_toggle_cb (GTK_TOGGLE_BUTTON (glade_xml_get_widget (dialog->xml,
									  "rdp_custom_window_radio")),
				 dialog);


	/* setup ranges for width/height spin buttons */
	screen = gtk_widget_get_screen (GTK_WIDGET (dialog));
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (glade_xml_get_widget (dialog->xml,
									  "rdp_window_width_spin")),
				   1, gdk_screen_get_width (screen));
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (glade_xml_get_widget (dialog->xml,
									  "rdp_window_height_spin")),
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
									  "rdp_window_width_spin")),
				   (gdouble) width);
	
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (glade_xml_get_widget (dialog->xml,
									  "rdp_window_height_spin")),
				   (gdouble) height);

	switch (connection->bpp) {
	case 24:
		bpp_index = 0;
		break;
	case 16:
		bpp_index = 1;
		break;
	case 15:
		bpp_index = 2;
		break;
	case 8:
		bpp_index = 3;
		break;
	default:
		bpp_index = 0;
		break;
	}

	gtk_combo_box_set_active (GTK_COMBO_BOX (glade_xml_get_widget (dialog->xml,
								       "bitdepth_combo")),
				  bpp_index);
}

static void
tsc_rdp_edit_dialog_save (TSCEditDialog *edit_dialog)
{

	TSCRdpEditDialog *dialog;
	TSCRdpConnection *connection;

	dialog = TSC_RDP_EDIT_DIALOG (edit_dialog);
	connection = TSC_RDP_CONNECTION (edit_dialog->connection);

	g_free (connection->host);
	connection->host = g_strdup (gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (dialog->xml,
											  "host_entry"))));

	g_free (connection->username);
	connection->username = g_strdup (gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (dialog->xml,
											      "username_entry"))));
	NULLIFY_IF_EMPTY (connection->username);
	
	g_free (connection->password);
	connection->password = g_strdup (gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (dialog->xml,
											      "password_entry"))));
	NULLIFY_IF_EMPTY (connection->password);

	g_free (connection->domain);
	connection->domain = g_strdup (gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (dialog->xml,
											    "domain_entry"))));
	NULLIFY_IF_EMPTY (connection->domain);

	connection->sound = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (dialog->xml,
												   "sound_check")));

	switch (gtk_combo_box_get_active (GTK_COMBO_BOX (glade_xml_get_widget (dialog->xml,
									       "experience_combo")))) {
	case 0:
		connection->experience = TSC_RDP_EXPERIENCE_DEFAULT;
		break;
	case 1:
		connection->experience = TSC_RDP_EXPERIENCE_MODEM;
		break;
	case 2:
		connection->experience = TSC_RDP_EXPERIENCE_BROADBAND;
		break;
	case 3:
		connection->experience = TSC_RDP_EXPERIENCE_LAN;
		break;
	default:
		connection->experience = TSC_RDP_EXPERIENCE_DEFAULT;
		break;
	}

	connection->fullscreen = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (dialog->xml,
													"rdp_fullscreen_radio")));
	connection->width = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (glade_xml_get_widget (dialog->xml, "rdp_window_width_spin")));
	connection->height = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (glade_xml_get_widget (dialog->xml, "rdp_window_height_spin")));
}

static gboolean
tsc_rdp_edit_dialog_verify (TSCEditDialog *edit_dialog, gchar **reason)
{
	TSCRdpEditDialog *dialog = TSC_RDP_EDIT_DIALOG (edit_dialog);
	GtkEntry *host_entry = GTK_ENTRY (glade_xml_get_widget (dialog->xml, "host_entry"));
	
	if (!g_ascii_strcasecmp ("", gtk_entry_get_text (host_entry))) {
		*reason = g_strdup (_("Host is a required field"));
		return FALSE;
	} 
	
	return TRUE;
}

static void
tsc_rdp_edit_dialog_class_init (TSCRdpEditDialogClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;
	TSCEditDialogClass *edit_dialog_class = (TSCEditDialogClass *) klass;
	
	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = tsc_rdp_edit_dialog_finalize;
	edit_dialog_class->load = tsc_rdp_edit_dialog_load;
	edit_dialog_class->save = tsc_rdp_edit_dialog_save;
	edit_dialog_class->verify = tsc_rdp_edit_dialog_verify;
}

static void
tsc_rdp_edit_dialog_init (TSCRdpEditDialog *dialog)
{
	GtkWidget *content;
	
	dialog->xml = glade_xml_new (GLADEFILE, "rdp_edit_dialog_content",
				     PACKAGE);

	g_signal_connect (glade_xml_get_widget (dialog->xml,
						"rdp_custom_window_radio"),
			  "toggled",
			  G_CALLBACK (custom_window_toggle_cb),
			  dialog);
	
	content = glade_xml_get_widget (dialog->xml, "rdp_edit_dialog_content");
	gtk_widget_show_all (content);

	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), content);
}

GType
tsc_rdp_edit_dialog_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCRdpEditDialogClass),
			NULL, NULL,
			(GClassInitFunc) tsc_rdp_edit_dialog_class_init,
			NULL, NULL,
			sizeof (TSCRdpEditDialog),
			0,
			(GInstanceInitFunc) tsc_rdp_edit_dialog_init
		};
		
		type = g_type_register_static (TSC_TYPE_EDIT_DIALOG,
					       "TSCRdpEditDialog",
					       &type_info, 0);
	}
	
	return type;

}
