#ifndef LOG_MONITOR_H
#define LOG_MONITOR_H

#include <limits.h>

/*
 * ABS_LOG_PATH is a dynamically allocated string (char *).
 * main.c allocates it and other source files use it via extern.
 */
extern char *ABS_LOG_PATH;

typedef struct {
    long long total;
    long long idle;
} cpu_stats_t;

/* real-time monitoring */
void read_cpu_jiffies(cpu_stats_t *stats);
int calculate_cpu_usage(void);
int get_memory_usage(void);

/* daemon helpers */
void daemonize(void);
void monitor_log(void);
void log_system_stats(void);

/* optional helper used for tagged messages (if present) */
void log_message_tagged(const char *tag, const char *payload);

#endif /* LOG_MONITOR_H */
