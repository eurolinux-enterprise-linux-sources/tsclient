
#ifndef __TSC_UTIL__
#define __TSC_UTIL__

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#define NULLIFY_IF_EMPTY(x) if (g_ascii_strcasecmp (x, "") == 0) { g_free (x); x = NULL; }

typedef enum {
	TSC_USERLEVEL_LOW,
	TSC_USERLEVEL_MEDIUM,
	TSC_USERLEVEL_HIGH,
	TSC_USERLEVEL_UNKNOWN
} TSCUserLevel;

#define GENERAL_CONFIG_GROUP "General"

#define TSC_ERROR tsc_error_quark()
GQuark tsc_error_quark (void);

void              tsc_util_show_error        (const char *summary, const char *details);
GError           *tsc_util_get_process_error (int status, const char *possible_reason);

gboolean          tsc_util_copy_file         (const char *src, const char *dest,
					      GError **error);
gboolean          tsc_util_copy_directory    (const char *src, const char *dest,
					      GError **error);
gboolean          tsc_util_delete_directory  (const char *path, GError **error);

char             *tsc_util_get_absolute_path (const char *path);
GdkPixbuf        *tsc_util_get_icon          (const char *name, int size);

TSCUserLevel      tsc_util_get_userlevel     (void);
gboolean          tsc_util_check_userlevel   (TSCUserLevel required_level);

gboolean          tsc_util_check_uri_scheme  (const char *uri, const char *scheme);
char             *tsc_util_get_uri_host      (const char *uri);
gboolean		  tsc_util_program_exists    (const char *program);
char 			 *tsc_util_create_terminal_command (const char *command);
#endif
