
#include <config.h>
#include <unistd.h>
#include <string.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include "tsc-window.h"
#include "tsc-edit-dialog.h"
#include "tsc-connection-model.h"
#include "tsc-provider.h"
#include "tsc-util.h"

enum {
	PROP_0,
	PROP_MANAGER,
	PROP_CHOOSER_ONLY
};

enum {
	DRAG_TARGET_URI,
};

static const GtkTargetEntry drag_targets[] = {
	{ "text/uri-list", 0, DRAG_TARGET_URI },
};

static GtkWindowClass *parent_class;

static void
tsc_window_finalize (GObject *obj)
{
	TSCWindow *window = TSC_WINDOW (obj);

	if (window->xml) {
		g_object_unref (window->xml);
	}

	if (window->manager) {
		g_object_unref (window->manager);
	}

	if (window->ui_manager) {
		g_object_unref (window->ui_manager);
	}
	
	if (G_OBJECT_CLASS (parent_class)->finalize) {
		G_OBJECT_CLASS (parent_class)->finalize (obj);
	}
}

static GList *
tsc_window_get_selected (TSCWindow *window)
{
	GList *selected, *l, *ret;
	GtkTreeIter iter;
	TSCConnection *connection;

	ret = NULL;
	selected = gtk_icon_view_get_selected_items (GTK_ICON_VIEW (window->icon_view));
	for (l = selected; l; l = l->next) {
		gtk_tree_model_get_iter (window->model, &iter,
					 (GtkTreePath *) l->data);
		gtk_tree_model_get (window->model, &iter,
				    TSC_MODEL_COLUMN_CONNECTION,
				    &connection, -1);

		ret = g_list_prepend (ret, connection);
	}
	g_list_free (selected);

	return ret;
}

static void
tsc_window_update_sensitive (TSCWindow *window)
{
	TSCConnection *connection;
	GtkAction *action;
	GList *selected;

	selected = tsc_window_get_selected (window);
	if (selected) {
		connection = TSC_CONNECTION (selected->data);

		action = gtk_ui_manager_get_action (window->ui_manager,
						    "/ui/MainPopup/EditConnection");
		gtk_action_set_sensitive (action, g_list_length (selected) == 1 &&
					  tsc_provider_can_edit (tsc_connection_get_provider (connection)));

		action = gtk_ui_manager_get_action (window->ui_manager,
						    "/ui/MainPopup/StartConnection");
		gtk_action_set_sensitive (action, g_list_length (selected) == 1);

		g_list_free (selected);

		gtk_action_group_set_sensitive (window->connection_actions, TRUE);
	} else {
		gtk_action_group_set_sensitive (window->connection_actions, FALSE);
	}
}

static void
tsc_window_set_property (GObject      *object,
			 guint         prop_id,
			 const GValue *value,
			 GParamSpec   *pspec)
{
	TSCWindow *window;

	window = TSC_WINDOW (object);
	
	switch (prop_id) {
	case PROP_MANAGER:
		if (window->manager) {
			g_object_unref (window->manager);
		}

		if (window->model) {
			g_object_unref (window->model);
		}
		
		window->manager = TSC_MANAGER (g_object_ref (g_value_get_object (value)));
		
		window->model = tsc_connection_model_new (window->manager);
		gtk_icon_view_set_model (GTK_ICON_VIEW (window->icon_view), window->model);
		gtk_icon_view_set_markup_column (GTK_ICON_VIEW (window->icon_view), 0);
		gtk_icon_view_set_pixbuf_column (GTK_ICON_VIEW (window->icon_view), 1);
		tsc_window_update_sensitive (window);
		break;
	case PROP_CHOOSER_ONLY:
		window->chooser_only = g_value_get_boolean (value);
		if (window->chooser_only) {
			gtk_widget_hide (glade_xml_get_widget (window->xml, "main_toolbar"));
		} else {
			gtk_widget_show (glade_xml_get_widget (window->xml, "main_toolbar"));
		}
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
tsc_window_get_property (GObject      *object,
			 guint         prop_id,
			 GValue       *value,
			 GParamSpec   *pspec)
{
	TSCWindow *window;
	
	window = TSC_WINDOW (object);

	switch (prop_id) {
	case PROP_MANAGER:
		g_value_set_object (value, window->manager);
		break;
	case PROP_CHOOSER_ONLY:
		g_value_set_boolean (value, window->chooser_only);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
tsc_window_class_init (TSCWindowClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = tsc_window_finalize;
	object_class->get_property = tsc_window_get_property;
	object_class->set_property = tsc_window_set_property;
	
	g_object_class_install_property (object_class,
					 PROP_MANAGER,
					 g_param_spec_object ("manager",
							      _("Manager"),
							      _("The TSCManager to use"),
							      TSC_TYPE_MANAGER,
							      G_PARAM_READWRITE));

	g_object_class_install_property (object_class,
					 PROP_CHOOSER_ONLY,
					 g_param_spec_boolean ("chooser-only",
							       _("chooser"),
							       _("Whether or not the window should only be a chooser"),
							       FALSE,
							       G_PARAM_READWRITE));
}



static void
tsc_window_item_activated_cb (GtkIconView *icon_view,
			      GtkTreePath *path,
			      TSCWindow   *window)
{
	GtkTreeIter iter;
	TSCConnection *connection;
	
	if (!gtk_tree_model_get_iter (window->model, &iter, path)) {
		return;
	}

	gtk_tree_model_get (window->model, &iter,
			    TSC_MODEL_COLUMN_CONNECTION,
			    &connection, -1);

	tsc_connection_start (connection, NULL);
}

static void
tsc_window_add_menu_activate_cb (GtkMenuItem *item, TSCWindow *window)
{
	TSCProvider *provider;
	TSCConnection *connection;
	TSCEditDialog *dialog;
	int response;
	
	provider = TSC_PROVIDER (g_object_get_data (G_OBJECT (item), "provider"));
	g_return_if_fail (provider != NULL);

	connection = tsc_provider_create_connection (provider);
	g_return_if_fail (connection != NULL);

	dialog = tsc_provider_create_edit_dialog (provider, connection);
	if (!dialog)
		return;
	
	gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (window));
	response = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (GTK_WIDGET (dialog));
	if (response == GTK_RESPONSE_OK) {
		tsc_manager_add_connection (window->manager, connection);
	}
	
}


static GtkWidget *
tsc_window_create_connection_menu (TSCWindow *window)
{
	TSCProvider *provider;
	GtkWidget *menu;
	GtkWidget *item;
	GtkImage *image;
	const GList *l;
	
	menu = gtk_menu_new ();

	for (l = tsc_manager_get_providers (window->manager); l; l = l->next) {
		provider = TSC_PROVIDER (l->data);
		if (!tsc_provider_can_create (provider))
			continue;

		if (strcmp (tsc_provider_get_name (provider), "Generic") == 0) {
			/* add a separator item here, this one is kind of different */
			item = gtk_separator_menu_item_new ();
			gtk_widget_show (item);
			gtk_menu_append (GTK_MENU (menu), item);
		}
		
		item = gtk_image_menu_item_new_with_label (tsc_provider_get_display_name (provider));
		image = GTK_IMAGE (gtk_image_new_from_icon_name (tsc_provider_get_icon_name (provider),
						      GTK_ICON_SIZE_MENU));

		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), GTK_WIDGET (image));
		
		g_object_set_data_full (G_OBJECT (item), "provider",
					g_object_ref (provider),
					g_object_unref);
		g_signal_connect (item, "activate",
				  G_CALLBACK (tsc_window_add_menu_activate_cb),
				  window);
		gtk_widget_set_sensitive (item, tsc_provider_is_enabled (provider));
		
		gtk_widget_show_all (item);
		gtk_menu_append (GTK_MENU (menu), item);
	}

	return menu;
}


static void
tsc_window_do_popup (TSCWindow *window, GdkEventButton *event)
{
	GtkWidget *menu, *add_menu;
	guint button;
	guint32 event_time;

	menu = gtk_ui_manager_get_widget (window->ui_manager,
					  "/ui/MainPopup");
	add_menu = gtk_ui_manager_get_widget (window->ui_manager,
					      "/ui/MainPopup/AddConnection");
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (add_menu),
				   tsc_window_create_connection_menu (window));

	button = 0;
	event_time = gtk_get_current_event_time ();
	if (event) {
		button = event->button;
		event_time = event->time;
	}
	
	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
			button, event_time);
}

static gboolean
tsc_window_button_press_cb (GtkIconView *icon_view, GdkEventButton *event,
			    TSCWindow *window)
{
	/* Ignore double-clicks and triple-clicks */
	if (event->button == 3 && event->type == GDK_BUTTON_PRESS &&
	    !window->chooser_only) {
		tsc_window_do_popup (window, event);
		return TRUE;
	}

	return FALSE;
}


static gboolean
tsc_window_popup_cb (GtkIconView *icon_view, TSCWindow *window)
{
	tsc_window_do_popup (window, NULL);
	return TRUE;
}

static void
tsc_window_selection_changed_cb (GtkIconView *icon_view,
				 TSCWindow   *window)
{
	tsc_window_update_sensitive (window);
}



static void
tsc_window_remove_connection_cb (GtkAction *action, TSCWindow *window)
{
	GList *selected, *l;

	selected = tsc_window_get_selected (window);
	if (selected) {
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (window), GTK_DIALOG_MODAL,
							    GTK_MESSAGE_QUESTION,
							    GTK_BUTTONS_YES_NO,
							    _("Are you sure you want to remove the selected connections?  This operation cannot be reversed."));
		int response = gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		if (response == GTK_RESPONSE_NO) {
			goto cleanup;
		}
	}
	
	for (l = selected; l; l = l->next) {
		tsc_manager_remove_connection (window->manager,
					       TSC_CONNECTION (l->data));
	}

cleanup:
	g_list_free (selected);
}

static void
tsc_window_start_connection_cb (GtkAction *action, TSCWindow *window)
{
	GList *selected, *l;

	selected = tsc_window_get_selected (window);
	for (l = selected; l; l = l->next) {
		tsc_connection_start (TSC_CONNECTION (l->data), NULL);
	}

	g_list_free (selected);
}

static void
tsc_window_edit_connection_cb (GtkAction *action, TSCWindow *window)
{
	TSCEditDialog *dialog;
	TSCConnection *connection;
	GList *selected, *l;
	int response;

	selected = tsc_window_get_selected (window);
	for (l = selected; l; l = l->next) {
		connection = TSC_CONNECTION (l->data);
		
		/* we only edit one connection, so just pick the first one */
		g_message ("Editing connection: %s", tsc_connection_get_name (connection));
		dialog = tsc_provider_create_edit_dialog (tsc_connection_get_provider (connection),
							  connection);
		if (!dialog)
			return;

		gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (window));
		response = gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (GTK_WIDGET (dialog));
		if (response == GTK_RESPONSE_OK) {
			tsc_manager_save_connection (window->manager, connection);
		}
		break;
	}

	g_list_free (selected);
}



static void
tsc_window_quit_cb (GtkAction *action, TSCWindow *window)
{
	gtk_main_quit ();
}

static void
tsc_window_menu_unmap_cb (GtkWidget *menu, TSCWindow *window)
{
	/* gtk_widget_destroy (menu); */
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (glade_xml_get_widget (window->xml, "add_button")),
					   FALSE);
}

static void
tsc_window_position_popup_cb (GtkMenu *menu, int *x, int *y,
			      gboolean *push_in, gpointer user_data)
{
	TSCWindow *window = user_data;
	GtkWidget *button;

	button = glade_xml_get_widget (window->xml, "add_button");
	gdk_window_get_origin (button->window, x, y);
	*x = *x + button->allocation.x;
	*y = *y + button->allocation.height;
}

static void
tsc_window_add_button_cb (GtkToggleToolButton *button, TSCWindow *window)
{
	GtkWidget *menu;

	if (!gtk_toggle_tool_button_get_active (button))
		return;
	
	menu = tsc_window_create_connection_menu (window);
	g_signal_connect (menu, "unmap", G_CALLBACK (tsc_window_menu_unmap_cb), window);
	
	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, tsc_window_position_popup_cb, window,
			0, gtk_get_current_event_time ());
}

static void
tsc_window_import_connections_cb (GtkAction *action, TSCWindow *window)
{
	GtkWidget *chooser;
	char *filename;
	int response;

	chooser = gtk_file_chooser_dialog_new (_("Choose Connection File"),
					       GTK_WINDOW (window),
					       GTK_FILE_CHOOSER_ACTION_OPEN,
					       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					       GTK_STOCK_OPEN, GTK_RESPONSE_OK,
					       NULL);

	response = gtk_dialog_run (GTK_DIALOG (chooser));
	filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
	gtk_widget_destroy (chooser);
	
	if (response == GTK_RESPONSE_OK) {
		tsc_manager_import_connections (window->manager, filename, NULL);
	}

	g_free (filename);
}

static void
tsc_window_export_connections_cb (GtkAction *action, TSCWindow *window)
{
	GtkWidget *chooser;
	char *filename;
	int response;

	chooser = gtk_file_chooser_dialog_new (_("Choose Connection File"),
					       GTK_WINDOW (window),
					       GTK_FILE_CHOOSER_ACTION_SAVE,
					       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					       GTK_STOCK_SAVE, GTK_RESPONSE_OK,
					       NULL);

	response = gtk_dialog_run (GTK_DIALOG (chooser));
	filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
	gtk_widget_destroy (chooser);
	
	if (response == GTK_RESPONSE_OK) {
		tsc_manager_export_connections (window->manager,
						tsc_window_get_selected (window),
						filename, NULL);
	}

	g_free (filename);
}

static void
tsc_window_drag_begin_cb (GtkWidget *widget,
			  GdkDragContext *drag_context,
			  TSCWindow *window)
{
	GList *selected, *l;
	
	window->drag_files = NULL;
	selected = tsc_window_get_selected (window);
	for (l = selected; l; l = l->next) {
		TSCConnection *connection = TSC_CONNECTION (l->data);
		gchar *location = g_strconcat (tsc_connection_get_filename (connection), ".desktop", NULL);
		
		if (tsc_connection_create_shortcut (connection, location))
			window->drag_files = g_list_append (window->drag_files, location); 
	}
	g_list_free (selected);
}

static void
tsc_window_drag_data_get_cb (GtkWidget        *widget,
			     GdkDragContext   *drag_context,
			     GtkSelectionData *data,
			     guint             info,
			     guint             time,
			     TSCWindow        *window)
{
	GList *l;
	GString *uribuf;

	uribuf = g_string_new (NULL);
	for (l = window->drag_files; l; l = l->next) {
		g_string_append_printf (uribuf, "%s\r\n", (char *) l->data);
	}

	if (uribuf->str != NULL) {
		gtk_selection_data_set (data, data->target, 8, (guchar *) uribuf->str, uribuf->len);
	}

	g_string_free (uribuf, TRUE);
}

static void
tsc_window_drag_end_cb (GtkWidget      *widget,
			GdkDragContext *drag_context,
			TSCWindow      *window)
{
	g_list_foreach (window->drag_files, (GFunc) g_free, NULL);
	g_list_free (window->drag_files);
	window->drag_files = NULL;
}

static void
tsc_window_init (TSCWindow *window)
{
	GtkWidget *main_content;
	GtkActionGroup *main_actions;
	GtkActionGroup *connection_actions;
	GtkActionEntry connection_entries[] = {
		{
			"RemoveConnectionAction",
			GTK_STOCK_REMOVE,
			_("Remove Connection"),
			NULL,
			_("Remove an existing connection"),
			G_CALLBACK (tsc_window_remove_connection_cb)
		},
		{
			"EditConnectionAction",
			GTK_STOCK_PROPERTIES,
			_("Edit Connection"),
			NULL,
			_("Edit an existing connection"),
			G_CALLBACK (tsc_window_edit_connection_cb)
		},
		{
			"StartConnectionAction",
			GTK_STOCK_EXECUTE,
			_("Start Connection"),
			NULL,
			_("Start a previously configured connection"),
			G_CALLBACK (tsc_window_start_connection_cb)
		},
		{
			"ExportConnectionsAction",
			GTK_STOCK_SAVE,
			_("Export Connections"),
			NULL,
			_("Export connections to a file"),
			G_CALLBACK (tsc_window_export_connections_cb)
		},
		
		
	};
	GtkActionEntry main_entries[] = {
		{
			"AddConnectionAction",
			GTK_STOCK_ADD,
			_("Add Connection"),
			NULL,
			_("Add a new connection"),
			NULL
		},
		{
			"ImportConnectionsAction",
			GTK_STOCK_OPEN,
			_("Import Connections"),
			NULL,
			_("Import connections from a file"),
			G_CALLBACK (tsc_window_import_connections_cb)
		},
		{
			"QuitAction",
			GTK_STOCK_QUIT,
			_("Quit"),
			NULL,
			_("Quit the application"),
			G_CALLBACK (tsc_window_quit_cb)
		}
	};

	gtk_window_set_icon (GTK_WINDOW (window),
			     tsc_util_get_icon ("tsclient", 48));

	window->xml = glade_xml_new (GLADEFILE, "main_content", PACKAGE);
	if (!window->xml) {
		g_error ("Failed to load glade file.  Forgot 'make install'?");
	}

	main_content = glade_xml_get_widget (window->xml, "main_content");
	gtk_container_add (GTK_CONTAINER (window), main_content);
	gtk_widget_show_all (main_content);

	window->icon_view = glade_xml_get_widget (window->xml, "icon_view");
	
	gtk_icon_view_enable_model_drag_source (GTK_ICON_VIEW (window->icon_view),
						GDK_BUTTON1_MASK,
						drag_targets,
						G_N_ELEMENTS (drag_targets),
						GDK_ACTION_COPY);
	g_signal_connect (window->icon_view,
			  "drag-begin",
			  G_CALLBACK (tsc_window_drag_begin_cb),
			  window);
	g_signal_connect (window->icon_view,
			  "drag-data-get",
			  G_CALLBACK (tsc_window_drag_data_get_cb),
			  window);
	g_signal_connect (window->icon_view,
			  "drag-end",
			  G_CALLBACK (tsc_window_drag_end_cb),
			  window);
	g_signal_connect (window->icon_view,
			  "item-activated",
			  G_CALLBACK (tsc_window_item_activated_cb),
			  window);
	g_signal_connect (window->icon_view,
			  "selection-changed",
			  G_CALLBACK (tsc_window_selection_changed_cb),
			  window);
	g_signal_connect (window->icon_view, "popup-menu",
			  G_CALLBACK (tsc_window_popup_cb),
			  window);
	g_signal_connect_after (window->icon_view, "button-press-event",
				G_CALLBACK (tsc_window_button_press_cb),
				window);

	window->ui_manager = gtk_ui_manager_new ();
	
	main_actions = gtk_action_group_new ("MainActions");
	connection_actions = gtk_action_group_new ("ConnectionActions");
	
	gtk_action_group_add_actions (main_actions, main_entries,
				      G_N_ELEMENTS (main_entries),
				      window);
	gtk_action_group_add_actions (connection_actions, connection_entries,
				      G_N_ELEMENTS (connection_entries),
				      window);
	
	gtk_ui_manager_insert_action_group (window->ui_manager,
					    main_actions, 0);
	gtk_ui_manager_insert_action_group (window->ui_manager,
					    connection_actions, 1);
	
	gtk_action_connect_proxy (gtk_action_group_get_action (connection_actions,
							       "RemoveConnectionAction"),
				  glade_xml_get_widget (window->xml, "remove_button"));
	gtk_action_connect_proxy (gtk_action_group_get_action (connection_actions,
							       "EditConnectionAction"),
				  glade_xml_get_widget (window->xml, "edit_button"));

	gtk_action_connect_proxy (gtk_action_group_get_action (main_actions,
							       "QuitAction"),
				  glade_xml_get_widget (window->xml, "quit_button"));

	gtk_ui_manager_add_ui_from_file (window->ui_manager, UIFILE,
					 NULL);

	g_signal_connect (glade_xml_get_widget (window->xml, "add_button"),
			  "clicked", G_CALLBACK (tsc_window_add_button_cb), window);

	window->main_actions = main_actions;
	window->connection_actions = connection_actions;

}

GType
tsc_window_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCWindowClass),
			NULL, NULL,
			(GClassInitFunc) tsc_window_class_init,
			NULL, NULL,
			sizeof (TSCWindow),
			0,
			(GInstanceInitFunc) tsc_window_init
		};
		
		type = g_type_register_static (GTK_TYPE_WINDOW,
					       "TSCWindow",
					       &type_info, 0);
	}
	
	return type;

}

GtkWidget *
tsc_window_new (TSCManager *manager, gboolean chooser_only)
{
	TSCWindow *window;

	window = g_object_new (TSC_TYPE_WINDOW, "title", _("Terminal Server Client"),
			       "default-width", 400, "default-height", 200,
			       "manager", manager, "chooser-only", chooser_only,
			       NULL);
	
	
	return GTK_WIDGET (window);
}
