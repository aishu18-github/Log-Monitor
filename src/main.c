// src/main.c
#define _GNU_SOURCE   /* for asprintf */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include "log_monitor.h"

/* ABS_LOG_PATH is allocated dynamically here and used by log_monitor.c */
char *ABS_LOG_PATH = NULL;

int main(int argc, char *argv[]) {
    /* Resolve current working directory (project root) */
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        return EXIT_FAILURE;
    }

    /* Build logs directory path with asprintf (safe, dynamic) */
    char *logs_dir = NULL;
    if (asprintf(&logs_dir, "%s/logs", cwd) < 0 || logs_dir == NULL) {
        fprintf(stderr, "Error allocating logs_dir\n");
        return EXIT_FAILURE;
    }

    /* Ensure logs directory exists */
    struct stat st;
    if (stat(logs_dir, &st) == -1) {
        if (mkdir(logs_dir, 0755) != 0) {
            fprintf(stderr, "Error: unable to create logs directory: %s\n", logs_dir);
            free(logs_dir);
            return EXIT_FAILURE;
        }
    }

    /* Build absolute path to monitor.log */
    if (asprintf(&ABS_LOG_PATH, "%s/monitor.log", logs_dir) < 0 || ABS_LOG_PATH == NULL) {
        fprintf(stderr, "Error allocating ABS_LOG_PATH\n");
        free(logs_dir);
        return EXIT_FAILURE;
    }

    /* Optionally override with env var (keeps Option A default) */
    char *env_path = getenv("MONITOR_LOG_PATH");
    if (env_path && strlen(env_path) > 0) {
        /* replace ABS_LOG_PATH with env value */
        free(ABS_LOG_PATH);
        if (asprintf(&ABS_LOG_PATH, "%s", env_path) < 0) {
            fprintf(stderr, "Error setting ABS_LOG_PATH from env\n");
            free(logs_dir);
            return EXIT_FAILURE;
        }
    }

    /* Foreground mode useful for debugging */
    int foreground = 0;
    if (argc > 1 && (strcmp(argv[1], "--foreground") == 0 || strcmp(argv[1], "-f") == 0)) {
        foreground = 1;
        printf("Running in FOREGROUND mode. Logs are written to: %s\n", ABS_LOG_PATH);
    }

    /* Free logs_dir as we no longer need it */
    free(logs_dir);

    if (!foreground) {
        daemonize();
    }

    /* Begin monitoring loop (this will call log_system_stats which uses ABS_LOG_PATH) */
    monitor_log();

    /* never reached in normal daemon, but tidy up if it returns */
    if (ABS_LOG_PATH) free(ABS_LOG_PATH);
    return EXIT_SUCCESS;
}
