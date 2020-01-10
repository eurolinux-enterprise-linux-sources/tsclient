
#ifndef __TSC_SPAWN__
#define __TSC_SPAWN__

#include <glib.h>
#include <glib-object.h>

#define TSC_TYPE_SPAWN                   (tsc_spawn_get_type ())
#define TSC_SPAWN(spawn)                 (G_TYPE_CHECK_INSTANCE_CAST ((spawn), TSC_TYPE_SPAWN, TSCSpawn))
#define TSC_SPAWN_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSC_TYPE_SPAWN, TSCSpawnClass))
#define TSC_IS_SPAWN(spawn)              (G_TYPE_CHECK_INSTANCE_TYPE ((spawn), TSC_TYPE_SPAWN))
#define TSC_IS_SPAWN_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSC_TYPE_SPAWN))
#define TSC_SPAWN_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSC_TYPE_SPAWN, TSCSpawnClass))

#define TSC_SPAWN_MAX_ARGS 256

typedef struct _TSCSpawn {
	GObject parent;

	char *args[TSC_SPAWN_MAX_ARGS];
	int last_arg;

	GPid child_pid;
	GIOChannel *stderr;
	GIOChannel *stdout;
	GIOChannel *stdin;

	GString *stderr_data;
	GString *stdout_data;

	gboolean stopped;
	int status;

	gboolean killing;
} TSCSpawn;

typedef struct _TSCSpawnClass {
	GObjectClass parent_class;

	/* signals */
	void (* stopped) (TSCSpawn *spawn);
} TSCSpawnClass;

GType          tsc_spawn_get_type             (void);

TSCSpawn      *tsc_spawn_new                  (void);

const char   **tsc_spawn_get_args             (TSCSpawn *spawn);

void           tsc_spawn_set_args             (TSCSpawn *spawn, ...);
void           tsc_spawn_append_args          (TSCSpawn *spawn, ...);

gboolean       tsc_spawn_start                (TSCSpawn *spawn, GError **error);
gboolean       tsc_spawn_start_with_setup     (TSCSpawn *spawn, GSpawnChildSetupFunc func,
                                               gpointer data, GError **error);
gboolean       tsc_spawn_kill                 (TSCSpawn *spawn, GError **error);

GPid           tsc_spawn_get_pid              (TSCSpawn *spawn);
GIOChannel    *tsc_spawn_get_stdin            (TSCSpawn *spawn);

const char    *tsc_spawn_get_stdout_data      (TSCSpawn *spawn);
const char    *tsc_spawn_get_stderr_data      (TSCSpawn *spawn);

int            tsc_spawn_get_status           (TSCSpawn *spawn);
GError        *tsc_spawn_get_error            (TSCSpawn *spawn);

#endif
