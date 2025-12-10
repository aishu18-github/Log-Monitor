// src/daemon.c
#define _POSIX_C_SOURCE 200809L
#include "log_monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
 Typical daemonize routine:
  - fork and exit parent
  - setsid
  - optionally chdir("/")
  - redirect std fds to /dev/null
*/
void daemonize(void) {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS); /* parent exits */

    /* child */
    if (setsid() < 0) exit(EXIT_FAILURE);

    /* optional: ignore SIGHUP if needed (left default) */

    /* change working directory to root (safe because we'll use absolute paths) */
    if (chdir("/") < 0) {
        /* best-effort: continuing even if chdir fails */
    }

    /* set file mode mask to 0 */
    umask(0);

    /* close standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    /* reopen standard fds to /dev/null */
    open("/dev/null", O_RDONLY);  /* stdin */
    open("/dev/null", O_WRONLY);  /* stdout */
    open("/dev/null", O_WRONLY);  /* stderr */
}
