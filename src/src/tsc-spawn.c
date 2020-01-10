
#include <config.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include "tsc-spawn.h"
#include "tsc-marshal.h"
#include "tsc-util.h"

#define TSC_SPAWN_BUFSIZE 512

enum {
	SPAWN_STOPPED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };
static GObjectClass *parent_class;

static void
tsc_spawn_reset (TSCSpawn *spawn, gboolean reset_command)
{
	if (reset_command) {
		int i;

		for (i = 0; i < spawn->last_arg && spawn->args[i]; i++ ) {
			g_free (spawn->args[i]);
			spawn->args[i] = NULL;
		}

		spawn->last_arg = 0;
	}
	
	if (spawn->stdin) {
		g_io_channel_unref (spawn->stdin);
		spawn->stdin = NULL;
	}

	if (spawn->stdout) {
		g_io_channel_unref (spawn->stdout);
		spawn->stdout = NULL;
	}

	if (spawn->stderr) {
		g_io_channel_unref (spawn->stderr);
		spawn->stderr = NULL;
	}

	if (spawn->stdout_data) {
		g_string_free (spawn->stdout_data, TRUE);
		spawn->stdout_data = NULL;
	}

	if (spawn->stderr_data) {
		g_string_free (spawn->stderr_data, TRUE);
		spawn->stderr_data = NULL;
	}

	spawn->child_pid = (GPid) 0;
	spawn->status = -1;
	spawn->stopped = TRUE;
	spawn->killing = FALSE;
}

static void
tsc_spawn_finalize (GObject *obj)
{
	TSCSpawn *spawn = TSC_SPAWN (obj);

	tsc_spawn_reset (spawn, TRUE);
	
	if (parent_class->finalize) {
		parent_class->finalize (obj);
	}
}

static void
tsc_spawn_class_init (TSCSpawnClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = tsc_spawn_finalize;

	signals[SPAWN_STOPPED] =
		g_signal_new ("stopped",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (TSCSpawnClass,
					       stopped),
			      NULL, NULL,
			      tsc_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);
}

static void
tsc_spawn_init (TSCSpawn *spawn)
{
	tsc_spawn_reset (spawn, TRUE);
}

GType
tsc_spawn_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo type_info = {
			sizeof (TSCSpawnClass),
			NULL, NULL,
			(GClassInitFunc) tsc_spawn_class_init,
			NULL, NULL,
			sizeof (TSCSpawn),
			0,
			(GInstanceInitFunc) tsc_spawn_init
		};
		
		type = g_type_register_static (G_TYPE_OBJECT,
					       "TSCSpawn",
					       &type_info, 0);
	}
	
	return type;

}

TSCSpawn *
tsc_spawn_new (void)
{
	return TSC_SPAWN (g_object_new (TSC_TYPE_SPAWN, NULL));
}

const char **
tsc_spawn_get_args (TSCSpawn *spawn)
{
	g_return_val_if_fail (spawn != NULL, NULL);

	return (const char **) spawn->args;
}

void
tsc_spawn_set_args (TSCSpawn *spawn, ...)
{
	va_list ap;
	char *arg;
	int i;
	
	g_return_if_fail (spawn != NULL);

	for (i = 0; i < spawn->last_arg && spawn->args[i]; i++) {
		g_free (spawn->args[i]);
		spawn->args[i] = NULL;
	}

	spawn->last_arg = 0;

	va_start (ap, spawn);
	while ((arg = va_arg (ap, char *)) != NULL) {
		spawn->args[spawn->last_arg++] = g_strdup (arg);
	}
	va_end (ap);
}

void
tsc_spawn_append_args (TSCSpawn *spawn, ...)
{
	va_list ap;
	char *arg;
	
	g_return_if_fail (spawn != NULL);

	va_start (ap, spawn);
	while ((arg = va_arg (ap, char *)) != NULL) {
		spawn->args[spawn->last_arg++] = g_strdup (arg);
	}
	va_end (ap);
}

static void
tsc_spawn_maybe_emit_stop (TSCSpawn *spawn)
{
	if (!spawn->stopped && spawn->status >= 0 &&
	    spawn->stdout == NULL && spawn->stderr == NULL) {

		spawn->stopped = TRUE;
		
		g_io_channel_unref (spawn->stdin);
		spawn->stdin = NULL;

		g_signal_emit (spawn, signals[SPAWN_STOPPED], 0, NULL);
	}
}

gboolean
tsc_spawn_io_watch_cb (GIOChannel *source,
		       GIOCondition condition,
		       gpointer user_data)
{
	TSCSpawn *spawn = user_data;
	GString *dest = NULL;
	char buf[TSC_SPAWN_BUFSIZE];
	gsize bytes_read;

	if (source == spawn->stdout) {
		dest = spawn->stdout_data;
	}  else if (source == spawn->stderr) {
		dest = spawn->stderr_data;
	} else {
		/* apparently this is an old watch or something, just drop it */
		return FALSE;
	}
	
	if (condition & G_IO_IN) {
		while (g_io_channel_read_chars (source, buf, TSC_SPAWN_BUFSIZE, &bytes_read, NULL) == G_IO_STATUS_NORMAL) {
			g_string_append_len (dest, buf, bytes_read);
		}
	}

	if ((condition & G_IO_HUP) || (condition & G_IO_ERR)) {
		g_io_channel_unref (source);
		
		if (source == spawn->stdout) {
			spawn->stdout = NULL;
		} else if (source == spawn->stderr) {
			spawn->stderr = NULL;
		}

		tsc_spawn_maybe_emit_stop (spawn);
		return FALSE;
	} else {
		return TRUE;
	}
}

void
tsc_spawn_child_watch_cb (GPid pid,
			  gint status,
			  gpointer user_data)
{
	TSCSpawn *spawn = user_data;

	if (pid == spawn->child_pid) {
		spawn->status = status;
		tsc_spawn_maybe_emit_stop (spawn);
	}
}

static void
show_command (TSCSpawn *spawn)
{
	GString *str;
	int i;

	str = g_string_new (NULL);
	
	for (i = 0; spawn->args[i] && i < TSC_SPAWN_MAX_ARGS; i++) {
		g_string_append_printf (str, "%s ", spawn->args[i]);
	}

	g_message ("Command: %s", str->str);
	g_string_free (str, TRUE);
}

gboolean
tsc_spawn_start (TSCSpawn *spawn, GError **error)
{
	return tsc_spawn_start_with_setup (spawn, NULL, NULL, error);
}

gboolean
tsc_spawn_start_with_setup (TSCSpawn *spawn, GSpawnChildSetupFunc func,
                            gpointer data, GError **error)
{
	int child_stdin, child_stdout, child_stderr;
	
	g_return_val_if_fail (spawn != NULL, FALSE);

	if (!spawn->stopped) {
		g_set_error (error, TSC_ERROR, TSC_ERROR,
			     _("Must stop current process before starting a new one"));
		return FALSE;
	}
	
	if (!spawn->args[0]) {
		g_set_error (error, TSC_ERROR, TSC_ERROR, _("No command is set"));
		return FALSE;
	}
	
	tsc_spawn_reset (spawn, FALSE);

	show_command (spawn);
	if (!g_spawn_async_with_pipes (NULL, spawn->args, NULL,
				       G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD,
				       func, data, &spawn->child_pid,
				       &child_stdin, &child_stdout, &child_stderr,
				       error)) {
		return FALSE;
	}

	spawn->stopped = FALSE;

	spawn->stdin = g_io_channel_unix_new (child_stdin);
	spawn->stdout = g_io_channel_unix_new (child_stdout);
	spawn->stderr = g_io_channel_unix_new (child_stderr);
	g_io_channel_set_flags (spawn->stdout, G_IO_FLAG_NONBLOCK, NULL);
	g_io_channel_set_flags (spawn->stderr, G_IO_FLAG_NONBLOCK, NULL);

	spawn->stdout_data = g_string_new (NULL);
	spawn->stderr_data = g_string_new (NULL);
	
	g_io_add_watch (spawn->stdout, G_IO_IN | G_IO_ERR | G_IO_HUP,
			tsc_spawn_io_watch_cb, spawn);
	g_io_add_watch (spawn->stderr, G_IO_IN | G_IO_ERR | G_IO_HUP,
			tsc_spawn_io_watch_cb, spawn);

	g_child_watch_add (spawn->child_pid,
			   tsc_spawn_child_watch_cb,
			   spawn);

	return TRUE;
}

gboolean
tsc_spawn_kill (TSCSpawn *spawn, GError **error)
{
	int result;
	
	g_return_val_if_fail (spawn != NULL, FALSE);

	if (spawn->stopped || !spawn->child_pid) {
		g_set_error (error, TSC_ERROR, TSC_ERROR,
			     _("No process is running"));
		return FALSE;
	}

	spawn->killing = TRUE;
	result = kill (spawn->child_pid, SIGTERM);
	if (result < 0) {
		g_set_error (error, TSC_ERROR, TSC_ERROR,
			     g_strerror (errno));
		return FALSE;
	}

	return TRUE;
}


GPid
tsc_spawn_get_pid (TSCSpawn *spawn)
{
	g_return_val_if_fail (spawn != NULL, (GPid) 0);

	return spawn->child_pid;
}

GIOChannel *
tsc_spawn_get_stdin (TSCSpawn *spawn)
{
	g_return_val_if_fail (spawn != NULL, NULL);

	return spawn->stdin;
}

const char *
tsc_spawn_get_stdout_data (TSCSpawn *spawn)
{
	g_return_val_if_fail (spawn != NULL, NULL);

	if (spawn->stdout_data) {
		return spawn->stdout_data->str;
	} else {
		return NULL;
	}
}

const char *
tsc_spawn_get_stderr_data (TSCSpawn *spawn)
{
	g_return_val_if_fail (spawn != NULL, NULL);

	if (spawn->stderr_data) {
		return spawn->stderr_data->str;
	} else {
		return NULL;
	}
}

int
tsc_spawn_get_status (TSCSpawn *spawn)
{
	g_return_val_if_fail (spawn != NULL, 0);

	return spawn->status;
}

GError *
tsc_spawn_get_error (TSCSpawn *spawn)
{
	GError *error = NULL;
	const char *error_str = NULL;

	g_return_val_if_fail (spawn != NULL, NULL);

	if (!spawn->stopped) {
		return NULL;
	}

	if (WIFEXITED (spawn->status) && WEXITSTATUS (spawn->status) != 0) {
		if (spawn->stderr_data && spawn->stderr_data->str) {
			error_str = spawn->stderr_data->str;
		} else {
			error_str = _("Unknown Error");
		}
	} else if (WIFSIGNALED (spawn->status)) {
		int signum = WTERMSIG (spawn->status);

		if (signum == SIGSEGV || signum == SIGBUS) {
			error_str = _("Child process crashed");
		} else {
			error_str = _("Child terminated by signal");
		}
	}

	if (error_str && !spawn->killing) {
		g_set_error (&error, TSC_ERROR, TSC_ERROR, error_str);
	}

	return error;
}
