
#include <config.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gnome.h>
#include "tsc-window.h"
#include "tsc-manager.h"
#include "tsc-util.h"

static char *initial_connection = NULL;
static gboolean list_connections = FALSE;
static gboolean do_autostart = FALSE;
static char *import_file = NULL;
static char *export_file = NULL;
static char *connection_uri = NULL;

static gboolean no_gtk_main = FALSE;
static gboolean exit_at_idle = FALSE;
static gboolean logout_at_idle = FALSE;
static gboolean show_window = FALSE;
static gboolean chooser = FALSE;

static void
tsc_init (int argc, char **argv)
{
	GOptionContext *options;
	GOptionEntry entries[] = {
		{
			"start",
			's',
			0,
			G_OPTION_ARG_STRING,
			&initial_connection,
			_("Start a previously created connection"),
			_("name-or-file")
		},
		{
			"connect",
			'c',
			0,
			G_OPTION_ARG_STRING,
			&connection_uri,
			_("Start a new connection from a URI"),
			_("uri")
		},
		{
			"list-connections",
			'l',
			0,
			G_OPTION_ARG_NONE,
			&list_connections,
			_("List the configured connections"),
			NULL
		},
		{
			"autostart",
			'a',
			0,
			G_OPTION_ARG_NONE,
			&do_autostart,
			_("Start all connections configured for autostart"),
			NULL
		},
		{
			"logout",
			'o',
			0,
			G_OPTION_ARG_NONE,
			&logout_at_idle,
			_("Initiates a logout after all connections have ended"),
			NULL
		},
		{
			"import",
			'i',
			0,
			G_OPTION_ARG_STRING,
			&import_file,
			_("Import a set of previously exported connections"),
			NULL
		},
		{
			"export",
			'e',
			0,
			G_OPTION_ARG_STRING,
			&export_file,
			_("Export all connections to specified file"),
			NULL
		},
		{
			"chooser",
			0, 0,
			G_OPTION_ARG_NONE,
			&chooser,
			_("Only show a connection chooser"),
			NULL
		},
		{ NULL }
	};

	options = g_option_context_new (NULL);
#if GLIB_CHECK_VERSION (2, 12, 0)
	g_option_context_set_summary (options, _("Configure and initiate connections to remote machines"));
#endif
	g_option_context_add_main_entries (options, entries, NULL);
	g_option_context_set_help_enabled (options, TRUE);

#ifdef GNOME_PARAM_GOPTION_CONTEXT
	gnome_program_init(PACKAGE, VERSION,
			   LIBGNOMEUI_MODULE, argc, argv,
			   GNOME_PARAM_GOPTION_CONTEXT, options,
			   GNOME_PARAM_NONE);
#else
	g_type_init ();
	g_option_context_parse (options, &argc, &argv, NULL);
	gnome_program_init(PACKAGE, VERSION,
			   LIBGNOMEUI_MODULE, argc, argv,
			   GNOME_PARAM_NONE);
#endif
}

static gboolean
start_connection (TSCConnection *connection)
{
	GError *error = NULL;
	
	if (!tsc_connection_start (connection, &error)) {
		return FALSE;
	}

	return TRUE;
}

/* copy/pasted from libssui */
static void
do_logout (void)
{
        GnomeClient *client;

        if (!(client = gnome_master_client ()))
                return;

        /* Only request a Global save. We only want a Local
         * save if the user selects 'Save current setup'
         * from the dialog.
         */
        gnome_client_request_save (client,
				   GNOME_SAVE_GLOBAL,
				   TRUE,
				   GNOME_INTERACT_ANY,
				   TRUE, /* do not use the gnome-session gui */
				   TRUE);
}

static void
manager_idle_cb (TSCManager *manager)
{
	if (logout_at_idle) {
		do_logout ();
	}
	
	if (exit_at_idle) {
		gtk_main_quit ();
	}
}

static gboolean
prompt_autostart (TSCManager *manager)
{
	GtkWidget *dialog;
	int response;
	TSCUserLevel userlevel;
	
	userlevel = tsc_util_get_userlevel ();
	if (userlevel != TSC_USERLEVEL_HIGH)
		return TRUE;

	dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL,
					 GTK_MESSAGE_QUESTION,
					 GTK_BUTTONS_YES_NO,
					 _("Would you like to start automatically configured connections?"));
	response = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);

	return response == GTK_RESPONSE_YES;
}

static int
handle_cmdline (TSCManager *manager)
{
	TSCConnection *connection;
	GError *error = NULL;
	int retval = 0;

	/* By default, show just  a chooser if userlevel is < HIGH */
	chooser = ! tsc_util_check_userlevel (TSC_USERLEVEL_HIGH);

	if (list_connections) {
		const GList *connections, *l;

		connections = tsc_manager_get_connections (manager);
		if (connections == NULL) {
			g_print (_("No connections are currently available.\n"));
		} else {
			g_print (_("Available Connections:"));
			g_print ("\n\n");
			
			for (l = connections; l; l = l->next) {
				TSCConnection *connection = TSC_CONNECTION (l->data);
				
				g_print ("%s (%s)\n",
					 tsc_connection_get_name (connection),
					 tsc_provider_get_name (tsc_connection_get_provider (connection)));
			}
		}

		no_gtk_main = TRUE;
	} else if (initial_connection) {
		gchar *conn_file = NULL;

		if (!strncmp (initial_connection, "file://~/", 9)) {
			conn_file = g_build_filename (g_get_home_dir (), initial_connection + 9, NULL);
		} else if (initial_connection[0] == '~') {
			conn_file = g_build_filename (g_get_home_dir (), initial_connection + 1, NULL);
		} else if (!strncmp (initial_connection, "file://", 7)) {
			conn_file = g_strdup (initial_connection + 7);
		} else {
			conn_file = g_strdup (initial_connection);
		}

		if (g_file_test (conn_file, G_FILE_TEST_EXISTS)) {
			connection = tsc_manager_load_connection (manager, conn_file, NULL);
			if (connection) {
				tsc_manager_add_connection (manager, connection);
			}
		} else {
			connection = tsc_manager_lookup_connection (manager,
								    initial_connection);
		}

		g_free (conn_file);

		if (!connection) {
			retval = 1;
		} else if (start_connection (connection)) {
			exit_at_idle = TRUE;
		} else {
			retval = 1;
		}
	} else if (connection_uri) {
		const GList *l;

		connection = NULL;
		for (l = tsc_manager_get_providers (manager); l; l = l->next) {
			connection = tsc_provider_create_from_uri (TSC_PROVIDER (l->data),
								   connection_uri);
			if (connection)
				break;
		}

		if (!connection) {
			tsc_util_show_error (_("Unable to create connection"),
					     g_strdup_printf (_("Unable to handle URI '%s'"),
							      connection_uri));
			retval = 1;
		} else if (start_connection (connection)) {
			gboolean found_existing = FALSE;
			
			for (l = tsc_manager_get_connections (manager); l; l = l->next) {
				if (strcmp (tsc_connection_get_name (TSC_CONNECTION (l->data)),
					    tsc_connection_get_name (connection)) == 0) {
					found_existing = TRUE;
					break;
				}
			}

			if (!found_existing) {
				tsc_manager_add_connection (manager, connection);
			}
			
			exit_at_idle = TRUE;
		} else {
			retval = 1;
		}
	} else if (do_autostart) {
		const GList *l;
		gboolean prompted = FALSE;
		TSCUserLevel userlevel;

		exit_at_idle = TRUE;
		userlevel = tsc_util_get_userlevel ();
		
		for (l = tsc_manager_get_connections (manager); l; l = l->next) {
			connection = TSC_CONNECTION (l->data);
			
			if (tsc_connection_get_autostart (connection)) {
				if (!prompted) {
					prompted = TRUE;
					if (!prompt_autostart (manager)) {
						break;
					}
				}
				
				if (start_connection (connection)) {
					logout_at_idle = (userlevel == TSC_USERLEVEL_LOW);
				} else {
					retval = 1;
				}
			}
		}
	} else if (import_file) {
		no_gtk_main = TRUE;

		if (!tsc_manager_import_connections (manager, import_file, &error)) {
			tsc_util_show_error (_("Unable to import connections"),
					     error && error->message ? error->message : _("Unknown Error"));
			if (error) {
				g_error_free (error);
			}

			retval = 1;
		}
	} else if (export_file) {
		no_gtk_main = TRUE;
		
		if (!tsc_manager_export_connections (manager, NULL, export_file, &error)) {
			tsc_util_show_error (_("Unable to export connections"),
					     error && error->message ? error->message : _("Unknown Error"));
			if (error) {
				g_error_free (error);
			}
			
			retval = 1;
		}
	} else {
		show_window = TRUE;
	}
		

	return retval;
}

int
main (int argc, char *argv[])
{
	TSCManager *manager;
	GtkWidget *window = NULL;
	int retval;

	bindtextdomain (PACKAGE, DATADIR);
	tsc_init (argc, argv);

	manager = tsc_manager_new ();
	g_signal_connect (manager, "idle",
			  G_CALLBACK (manager_idle_cb),
			  NULL);
	
	retval = handle_cmdline (manager);

	if (show_window) {
		window = tsc_window_new (manager, chooser);
		g_signal_connect (window, "delete_event",
				  G_CALLBACK (gtk_main_quit),
				  NULL);
		
		gtk_widget_show (window);
	}

	if (exit_at_idle && tsc_manager_is_idle (manager)) {
		no_gtk_main = TRUE;

		if (logout_at_idle) {
			do_logout ();
		}
	}
	
	if (retval == 0 && !no_gtk_main) {
		gtk_main ();
	}

	g_object_unref (manager);
	return retval;
}
