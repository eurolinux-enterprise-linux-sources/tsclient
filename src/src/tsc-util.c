
#include <config.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include "tsc-util.h"



GQuark
tsc_error_quark (void)
{
	static GQuark quark;

	if (!quark) {
		quark = g_quark_from_static_string ("tsc_error");
	}

	return quark;
}

void
tsc_util_show_error (const char *summary,
		     const char *details)
{
	GtkWidget *dialog;

	g_printerr (_("ERROR: %s: %s\n"), summary, details);
	
	dialog = gtk_message_dialog_new_with_markup (NULL, GTK_DIALOG_MODAL,
						     GTK_MESSAGE_ERROR,
						     GTK_BUTTONS_CLOSE,
						     "<span weight=\"bold\">%s</span>\n\n%s",
						     summary, details);
	gtk_window_set_title (GTK_WINDOW (dialog),
			      _("Terminal Server Client - Error"));
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}

GError *
tsc_util_get_process_error (int status, const char *possible_reason)
{
	GError *error = NULL;
	
	if (WIFEXITED (status) && WEXITSTATUS (status) != 0) {
		g_set_error (&error, TSC_ERROR, TSC_ERROR,
			     possible_reason ? possible_reason : _("Unknown Error"));
	} else if (WIFSIGNALED (status)) {
		int signum = WTERMSIG (status);

		if (signum == SIGSEGV || signum == SIGBUS) {
			g_set_error (&error, TSC_ERROR, TSC_ERROR,
				     _("Child process crashed"));
		} else {
			g_set_error (&error, TSC_ERROR, TSC_ERROR,
				     _("Child terminated by signal"));
		}
	}

	return error;
}

gboolean
tsc_util_copy_file (const char *src, const char *dest,
		    GError **error)
{
	GMappedFile *map;
	char *contents;
	gsize length;
	gboolean result;

	map = g_mapped_file_new (src, FALSE, error);
	if (!map)
		return FALSE;

	length = g_mapped_file_get_length (map);
	contents = g_mapped_file_get_contents (map);

	result = g_file_set_contents (dest, contents, length, error);
	g_mapped_file_free (map);

	return result;
}

gboolean
tsc_util_copy_directory (const char *src, const char *dest,
			 GError **error)
{
	GDir *dir;
	const char *item;
	char *src_path;
	char *dest_path;

	g_return_val_if_fail (src != NULL, FALSE);
	g_return_val_if_fail (dest != NULL, FALSE);

	
	dir = g_dir_open (src, 0, error);
	if (!dir) {
		return FALSE;
	}

	while ((item = g_dir_read_name (dir)) != NULL) {
		src_path = g_build_filename (src, item, NULL);
		dest_path = g_build_filename (dest, item, NULL);

		if (g_file_test (src_path, G_FILE_TEST_IS_DIR)) {
			g_mkdir_with_parents (dest_path, 0755);
			if (!tsc_util_copy_directory (src_path, dest_path, error)) {
				g_free (src_path);
				g_free (dest_path);
				g_dir_close (dir);
				return FALSE;
			}
		} else if (!tsc_util_copy_file (src_path, dest_path, error)) {
			g_free (src_path);
			g_free (dest_path);
			g_dir_close (dir);
			return FALSE;
		}

		g_free (src_path);
		g_free (dest_path);
	}

	g_dir_close (dir);
	
	return TRUE;
}

gboolean
tsc_util_delete_directory (const char *path, GError **error)
{
	GDir *dir;
	const char *item;
	char *item_path;

	g_return_val_if_fail (path != NULL, FALSE);
       
	dir = g_dir_open (path, 0, error);
	if (!dir) {
		return FALSE;
	}

	while ((item = g_dir_read_name (dir)) != NULL) {
		item_path = g_build_filename (path, item, NULL);

		if (g_file_test (item_path, G_FILE_TEST_IS_DIR)) {
			if (!tsc_util_delete_directory (item_path, error)) {
				g_free (item_path);
				g_dir_close (dir);
				return FALSE;
			}

		}

		unlink (item_path);
		g_free (item_path);
	}

	g_dir_close (dir);
	unlink (path);
	
	return TRUE;
	
}

char *
tsc_util_get_absolute_path (const char *path)
{
	if (g_path_is_absolute (path)) {
		return g_strdup (path);
	} else {
		return g_build_filename (g_get_current_dir (), path, NULL);
	}
}

GdkPixbuf *
tsc_util_get_icon (const char *name, int size)
{
	GtkIconTheme *theme;

	theme = gtk_icon_theme_get_default ();
	return gtk_icon_theme_load_icon (theme, name, size, 0, NULL);
}

TSCUserLevel
tsc_util_get_userlevel (void)
{
	char *child_stdout;
	TSCUserLevel level = TSC_USERLEVEL_UNKNOWN;
	
	if (g_spawn_command_line_sync ("userlevel", &child_stdout, NULL, NULL, NULL)) {
		child_stdout = g_strstrip (child_stdout);
		
		if (strcmp (child_stdout, "high") == 0) {
			level = TSC_USERLEVEL_HIGH;
		} else if (strcmp (child_stdout, "medium") == 0) {
			level = TSC_USERLEVEL_MEDIUM;
		} else if (strcmp (child_stdout, "low") == 0) {
			level = TSC_USERLEVEL_LOW;
		}

		g_free (child_stdout);
	}

	return level;
}

gboolean
tsc_util_check_userlevel (TSCUserLevel required_level)
{
	TSCUserLevel have_level;

	have_level = tsc_util_get_userlevel ();
	if (have_level == TSC_USERLEVEL_UNKNOWN)
		return TRUE;

	switch (required_level) {
	case TSC_USERLEVEL_HIGH:
		return have_level == TSC_USERLEVEL_HIGH;
	case TSC_USERLEVEL_MEDIUM:
		return have_level == TSC_USERLEVEL_HIGH ||
			have_level == TSC_USERLEVEL_MEDIUM;
	case TSC_USERLEVEL_LOW:
		return have_level == TSC_USERLEVEL_HIGH ||
			have_level == TSC_USERLEVEL_MEDIUM ||
			have_level == TSC_USERLEVEL_LOW;
	default:
		return TRUE; /* I guess? */
	}
}

gboolean
tsc_util_check_uri_scheme (const char *uri, const char *scheme)
{
	g_return_val_if_fail (uri != NULL, FALSE);
	g_return_val_if_fail (scheme != NULL, FALSE);

	return g_ascii_strncasecmp (scheme, uri, strlen (scheme)) == 0;
}

char *
tsc_util_get_uri_host (const char *uri)
{
	char **split_uri;
	char *host = NULL;
	int i;

	split_uri = g_strsplit_set (uri, ":/", -1);
	for (i = 1; split_uri[i]; i++) {
		if (strcmp (split_uri[i], "") != 0) {
			host = g_strdup (split_uri[i]);
			break;
		}
	}

	g_strfreev (split_uri);
	return host;
}

gboolean
tsc_util_program_exists (const char *program)
{
	gchar *p  = g_find_program_in_path (program);
	gboolean ret = p ? TRUE : FALSE;
	g_free (p);
	return ret;
}

char *
tsc_util_create_terminal_command (const char *command)
{
	/* FIXME: probably should check the gnome preferred terminal first */
	if (tsc_util_program_exists ("gnome-terminal")) {
		return g_strdup_printf ("gnome-terminal --disable-factory -x %s", command);
	} else if (tsc_util_program_exists ("urxvt")) {
		return g_strdup_printf ("urxvt -e %s", command);
	} else if (tsc_util_program_exists ("xterm")) {
		return g_strdup_printf ("xterm -e %s", command);
	} else {
		return NULL;
	}
}
