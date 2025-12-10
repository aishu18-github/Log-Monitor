// src/log_monitor.c
#define _POSIX_C_SOURCE 200809L
#include "log_monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

/* ABS_LOG_PATH allocated in main.c */
extern char *ABS_LOG_PATH;

#define LOG_INTERVAL_SECONDS 2

/* keep previous stats for CPU delta */
static cpu_stats_t prev_stats = {0, 0};

void read_cpu_jiffies(cpu_stats_t *stats) {
    if (!stats) return;
    FILE *f = fopen("/proc/stat", "r");
    if (!f) return;

    long long user = 0, nice = 0, system = 0, idle = 0, iowait = 0, irq = 0, softirq = 0, steal = 0;
    /* Try to parse the cpu line; at minimum we expect user,nice,system,idle */
    if (fscanf(f, "cpu %lld %lld %lld %lld %lld %lld %lld %lld",
               &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal) < 4) {
        /* fall back: set zeros and return */
        fclose(f);
        stats->total = 0;
        stats->idle = 0;
        return;
    }
    fclose(f);

    stats->total = user + nice + system + idle + iowait + irq + softirq + steal;
    stats->idle = idle;
}

int calculate_cpu_usage(void) {
    cpu_stats_t current = {0,0};
    read_cpu_jiffies(&current);

    if (prev_stats.total == 0) {
        /* first call: initialize prev and sleep a tiny bit to get delta */
        prev_stats = current;
        sleep(1);
        read_cpu_jiffies(&current);
    }

    long long total_delta = current.total - prev_stats.total;
    long long idle_delta  = current.idle - prev_stats.idle;

    int cpu_pct = 0;
    if (total_delta > 0) {
        double usage = 100.0 * (1.0 - ((double)idle_delta / (double)total_delta));
        if (usage < 0.0) usage = 0.0;
        if (usage > 100.0) usage = 100.0;
        cpu_pct = (int)(usage + 0.5);
    }

    prev_stats = current;
    return cpu_pct;
}

int get_memory_usage(void) {
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) return 0;

    char line[256];
    long total_kb = 0;
    long avail_kb  = 0;

    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "MemTotal:", 9) == 0) {
            sscanf(line, "MemTotal: %ld kB", &total_kb);
        } else if (strncmp(line, "MemAvailable:", 13) == 0) {
            sscanf(line, "MemAvailable: %ld kB", &avail_kb);
        }
        if (total_kb > 0 && avail_kb > 0) break;
    }
    fclose(f);

    if (total_kb == 0) return 0;
    long used_kb = total_kb - avail_kb;
    double pct = ((double)used_kb / (double)total_kb) * 100.0;
    if (pct < 0.0) pct = 0.0;
    if (pct > 100.0) pct = 100.0;
    return (int)(pct + 0.5);
}

void log_system_stats(void) {
    if (ABS_LOG_PATH == NULL) {
        /* nothing to do */
        return;
    }

    FILE *f = fopen(ABS_LOG_PATH, "a");
    if (!f) {
        /* If running in foreground and stderr is available, print error */
        /* Note: after daemonize, stderr may be /dev/null */
        return;
    }

    int cpu = calculate_cpu_usage();
    int mem = get_memory_usage();

    time_t now = time(NULL);
    char tsbuf[32];
    struct tm tm_now;
    localtime_r(&now, &tm_now);
    strftime(tsbuf, sizeof(tsbuf), "%Y-%m-%d %H:%M:%S", &tm_now);

    /* canonical log line */
    fprintf(f, "[SYS] %s cpu=%d mem=%d\n", tsbuf, cpu, mem);
    fflush(f);
    fclose(f);
}

void monitor_log(void) {
    /* main loop that periodically writes system stats */
    while (1) {
        log_system_stats();
        sleep(LOG_INTERVAL_SECONDS);
    }
}

/* optional helper if you want to log custom tagged messages */
void log_message_tagged(const char *tag, const char *payload) {
    if (!tag || !payload || ABS_LOG_PATH == NULL) return;
    FILE *f = fopen(ABS_LOG_PATH, "a");
    if (!f) return;
    time_t now = time(NULL);
    char tsbuf[32];
    struct tm tm_now;
    localtime_r(&now, &tm_now);
    strftime(tsbuf, sizeof(tsbuf), "%Y-%m-%d %H:%M:%S", &tm_now);
    fprintf(f, "[%s] %s %s\n", tag, tsbuf, payload);
    fflush(f);
    fclose(f);
}
