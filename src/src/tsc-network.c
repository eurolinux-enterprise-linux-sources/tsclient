
#include <config.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "tsc-network.h"
#include <libnm_glib.h>

#define NETWORK_TIMEOUT_INTERVAL 1000*30 /* 30 seconds */

typedef struct _NetworkWaitData {
	GtkWidget *window;
	guint timeout;
} NetworkWaitData;

static void
dialog_response_cb (GtkDialog *dialog, int response, gpointer user_data)
{
	gtk_main_quit ();
}

static gboolean
timeout_cb (gpointer user_data)
{
	NetworkWaitData *data = user_data;
	
	gtk_main_quit ();
	data->timeout = 0;
	return FALSE;
}

static GtkWidget *
create_wait_window (void)
{
	GtkWidget *window;
	GtkWidget *label;

	window = gtk_dialog_new_with_buttons (_("Waiting For Network..."), NULL,
					      GTK_DIALOG_NO_SEPARATOR,
					      _("Continue Without Network"),
					      GTK_RESPONSE_CANCEL,
					      NULL);
	g_signal_connect (window, "response", G_CALLBACK (dialog_response_cb), NULL);
	label = gtk_label_new (_("<b>Waiting for network connection, standby...</b>"));
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
	gtk_misc_set_padding (GTK_MISC (label), 12, 12);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (window)->vbox), label);
	return window;
}

static void
connection_state_cb (libnm_glib_ctx *ctx, gpointer user_data)
{
	if (libnm_glib_get_network_state (ctx) == LIBNM_ACTIVE_NETWORK_CONNECTION) {
		gtk_main_quit ();
	}
}

static void
wait_for_state_change (libnm_glib_ctx *ctx)
{
	NetworkWaitData data;
	guint id;

	id = libnm_glib_register_callback (ctx, connection_state_cb, NULL, NULL);

	data.window = create_wait_window ();
	gtk_widget_show_all (data.window);
	data.timeout = g_timeout_add (NETWORK_TIMEOUT_INTERVAL, timeout_cb, &data);

	gtk_main ();

	if (data.timeout > 0) {
		g_source_remove (data.timeout);
	}
	
	gtk_widget_destroy (data.window);

	libnm_glib_unregister_callback (ctx, id);
}

gboolean
tsc_network_wait (void)
{
	libnm_glib_ctx *ctx;
	libnm_glib_state state;
	
	ctx = libnm_glib_init ();
	if (!ctx) {
		/* probably no NetworkManager, assume network is up */
		return TRUE;
	}

	state = libnm_glib_get_network_state (ctx);
	if (state == LIBNM_NO_NETWORK_CONNECTION) {
		wait_for_state_change (ctx);
		state = libnm_glib_get_network_state (ctx);
	}
		
	libnm_glib_shutdown (ctx);
	
	return state != LIBNM_NO_NETWORK_CONNECTION;
}
