
#include <config.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <libgnome/gnome-desktop-item.h>
#include "tsc-edit-dialog.h"
#include "tsc-provider.h"
#include "tsc-util.h"

enum {
	PROP_0,
	PROP_CONNECTION
};

static GtkDialogClass *parent_class;

static void
tsc_edit_dialog_finalize (GObject *obj)
{
	TSCEditDialog *dialog = TSC_EDIT_DIALOG (obj);

	if (dialog->connection) {
		g_object_unref (dialog->connection);
	}

	if (G_OBJECT_CLASS (parent_class)->finalize) {
		G_OBJECT_CLASS (parent_class)->finalize (obj);
	}
}

static void
tsc_edit_dialog_load (TSCEditDialog *dialog)
{
	TSCEditDialogClass *class;
	int policy_index;
	
	gtk_entry_set_text (GTK_ENTRY (dialog->name_entry),
			    tsc_connection_get_name (dialog->connection));

	switch (tsc_connection_get_reconnect_policy (dialog->connection)) {
	case TSC_RECONNECT_ALWAYS:
		policy_index = 0;
		break;
	case TSC_RECONNECT_NEVER:
		policy_index = 1;
		break;
	case TSC_RECONNECT_ERROR:
		policy_index = 2;
		break;
	case TSC_RECONNECT_PROMPT_ERROR:
	default:
		policy_index = 3;
		break;
	}

	gtk_combo_box_set_active (GTK_COMBO_BOX (dialog->policy_combo),
				  policy_index);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->autostart_checkbox),
				      tsc_connection_get_autostart (dialog->connection));

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->shortcut_checkbox),
			tsc_connection_get_favorite (dialog->connection));

	class = TSC_EDIT_DIALOG_GET_CLASS (dialog);
	if (class->load) {
		class->load (dialog);
	} else {
		g_warning ("No load implementation available");
	}
	
	if (class->verify) {
		gchar *reason = NULL; 
		gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK,
										   class->verify (dialog, &reason));
		if (reason) {
			gchar *markup = g_markup_printf_escaped ("<b>%s</b>", reason);
			gtk_label_set_markup (GTK_LABEL (dialog->error_label), markup);
			g_free (markup);
			g_free (reason);
		} else {
			gtk_label_set_text (GTK_LABEL (dialog->error_label), "");
		}
	}
}

static void
tsc_edit_dialog_save (TSCEditDialog *dialog)
{
	TSCReconnectPolicy policy;
	TSCEditDialogClass *class;
	
	tsc_connection_set_name (dialog->connection,
				 gtk_entry_get_text (GTK_ENTRY (dialog->name_entry)));

	switch (gtk_combo_box_get_active (GTK_COMBO_BOX (dialog->policy_combo))) {
	case 0:
		policy = TSC_RECONNECT_ALWAYS;
		break;
	case 1:
		policy = TSC_RECONNECT_NEVER;
		break;
	case 2:
		policy = TSC_RECONNECT_ERROR;
		break;
	case 3:
	default:
		policy = TSC_RECONNECT_PROMPT_ERROR;
		break;
	}

	tsc_connection_set_reconnect_policy (dialog->connection, policy);
	tsc_connection_set_autostart (dialog->connection,
				      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->autostart_checkbox)));
	tsc_connection_set_favorite (dialog->connection,
					  gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->shortcut_checkbox))); 

	class = TSC_EDIT_DIALOG_GET_CLASS (dialog);
	if (class->save) {
		class->save (dialog);
	} else {
		g_warning ("No save implementation available");
	}
}

static void
tsc_edit_dialog_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
	TSCEditDialog *dialog;

	dialog = TSC_EDIT_DIALOG (object);
	
	switch (prop_id) {
	case PROP_CONNECTION:
		if (dialog->connection) {
			g_object_unref (dialog->connection);
		}
		
		dialog->connection = TSC_CONNECTION (g_object_ref (g_value_get_object (value)));
		tsc_edit_dialog_load (dialog);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
tsc_edit_dialog_get_property (GObject      *object,
			      guint         prop_id,
			      GValue       *value,
			      GParamSpec   *pspec)
{
	TSCEditDialog *dialog;

	dialog = TSC_EDIT_DIALOG (object);

	switch (prop_id) {
	case PROP_CONNECTION:
		g_value_set_object (value, dialog->connection);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
tsc_edit_dialog_response_cb (TSCEditDialog *dialog, int response, gpointer user_data)
{
	if (response != GTK_RESPONSE_OK) {
		return;
	}

	tsc_edit_dialog_save (dialog);
}

static void
tsc_edit_dialog_verify (TSCEditDialog *dialog)
{
	TSCEditDialogClass *klass;

	if (!g_ascii_strcasecmp ("", gtk_entry_get_text (GTK_ENTRY (dialog->name_entry)))) {
		gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, FALSE);
		gtk_label_set_markup (GTK_LABEL (dialog->error_label), _("<b>This connection must have a name</b>"));
		return;
	} 

	klass = TSC_EDIT_DIALOG_GET_CLASS (dialog);
	if (!klass->verify) {
		gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, TRUE);
	} else {
		gchar *reason = NULL;
		gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK,
										   klass->verify (dialog, &reason));
		if (reason) {
			gchar *markup = g_markup_printf_escaped ("<b>%s</b>", reason);
			gtk_label_set_markup (GTK_LABEL (dialog->error_label), markup);
			g_free (markup);
			g_free (reason);
		} else {
			gtk_label_set_text (GTK_LABEL (dialog->error_label), "");
		}
	}
}

static gboolean
tsc_edit_dialog_verify_data (gpointer user_data, ...)
{
	tsc_edit_dialog_verify (user_data);
	return FALSE;
}

static void
tsc_edit_dialog_watch_children_cb (GtkContainer *container, GtkWidget *widget, gpointer user_data)
{
	gpointer *p;
	TSCEditDialog *dialog = user_data;
	GQueue *queue = g_queue_new ();
	g_queue_push_head (queue, container);

	while ((p = g_queue_pop_head (queue))) {
		if (GTK_IS_ENTRY (p)) {
			g_signal_connect_swapped (p, "changed", G_CALLBACK (tsc_edit_dialog_verify_data), user_data);
		} else if (GTK_IS_CHECK_BUTTON (p)) {
			g_signal_connect_swapped (p, "toggled", G_CALLBACK (tsc_edit_dialog_verify_data), user_data);
		} else if (GTK_IS_BUTTON (p)) {
			g_signal_connect_swapped (p, "clicked", G_CALLBACK (tsc_edit_dialog_verify_data), user_data);
		} else if (GTK_IS_COMBO_BOX (p)) {
			g_signal_connect_swapped (p, "changed", G_CALLBACK (tsc_edit_dialog_verify_data), user_data);
		} else if (GTK_IS_CONTAINER (p)) {
			GList *w, *kids = gtk_container_get_children (GTK_CONTAINER (p));
			for (w = kids; w; w = w->next) {
				g_queue_push_tail (queue, w->data);
			}
		} 
	}

	g_queue_free (queue);
	gtk_box_pack_end (GTK_BOX (GTK_DIALOG (dialog)->vbox), dialog->error_label, TRUE, TRUE, 0);
	gtk_widget_show_all (GTK_DIALOG (dialog)->vbox);
}

static void
tsc_edit_dialog_class_init (TSCEditDialogClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = tsc_edit_dialog_finalize;
	object_class->get_property = tsc_edit_dialog_get_property;
	object_class->set_property = tsc_edit_dialog_set_property;
	
	g_object_class_install_property (object_class,
					 PROP_CONNECTION,
					 g_param_spec_object ("connection",
							      _("Connection"),
							      _("The TSCConnection to edit"),
							      TSC_TYPE_CONNECTION,
							      G_PARAM_READWRITE));
}

static void
tsc_edit_dialog_init (TSCEditDialog *dialog)
{
	int i;
	gboolean enable_favorite = FALSE;
	const gchar * const *data_dirs = g_get_system_data_dirs ();
	
	gtk_dialog_add_buttons (GTK_DIALOG (dialog),
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
	gtk_window_set_title (GTK_WINDOW (dialog), _("Edit Connection"));
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

	dialog->xml = glade_xml_new (GLADEFILE, "edit_dialog_content", PACKAGE);
	if (!dialog->xml) {
		g_error ("Failed to load glade file.  Forgot 'make install'?");
	}
	
	dialog->error_label = gtk_label_new (""); 
	dialog->content = glade_xml_get_widget (dialog->xml, "edit_dialog_content");
	dialog->name_entry = glade_xml_get_widget (dialog->xml, "name_entry");
	dialog->policy_combo = glade_xml_get_widget (dialog->xml, "recon_policy_combo");
	dialog->autostart_checkbox = glade_xml_get_widget (dialog->xml,
							   "autostart_checkbox");
	dialog->shortcut_checkbox = glade_xml_get_widget (dialog->xml,
							   "shortcut_checkbox");

	/* set insensitive if gnome-main-menu isn't installed */
	for (i = 0; data_dirs && data_dirs[i]; i++) {
		gchar *p = g_build_filename (data_dirs[i], "gnome-main-menu", "places.xbel", NULL);
		if (!g_file_test (p, G_FILE_TEST_EXISTS)) {
			enable_favorite = TRUE;
			g_free (p);
			break;
		}
		g_free (p);
	}
	
	gtk_widget_set_sensitive (dialog->shortcut_checkbox, enable_favorite);

	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), dialog->content);

	gtk_widget_show_all (dialog->content);

	g_signal_connect (dialog, "response",
			  G_CALLBACK (tsc_edit_dialog_response_cb),
			  NULL);
	
	g_signal_connect (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), "add",
			  G_CALLBACK (tsc_edit_dialog_watch_children_cb), dialog);
}

GType
tsc_edit_dialog_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCEditDialogClass),
			NULL, NULL,
			(GClassInitFunc) tsc_edit_dialog_class_init,
			NULL, NULL,
			sizeof (TSCEditDialog),
			0,
			(GInstanceInitFunc) tsc_edit_dialog_init
		};
		
		type = g_type_register_static (GTK_TYPE_DIALOG,
					       "TSCEditDialog",
					       &type_info, 0);
	}
	
	return type;
}

TSCEditDialog *
tsc_edit_dialog_new (GtkWindow *parent, TSCConnection *connection)
{
	g_return_val_if_fail (connection != NULL, NULL);
	
	return TSC_EDIT_DIALOG (g_object_new (TSC_TYPE_EDIT_DIALOG, "connection", connection,
					      "transient-for", parent, NULL));
}

