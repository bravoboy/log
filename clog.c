#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "clog.h"

static struct logger logger;
static struct logger logger_wf;

static char log_info[4][10] = {"ERR","WARN","INFO","DEBUG"};
static pthread_mutex_t log_mutex; 
char * translate(int level) {
    return log_info[level];
}

int log_init(int level, char *name,int thread) {
    struct logger *l = &logger;
    struct logger *l_wf = &logger_wf;

    l->level = level;
    memcpy(l->name,name,strlen(name));
    memcpy(l_wf->name,name,strlen(name));
    memcpy(l_wf->name + strlen(name),".wf",3);
    
    if (name == NULL || !strlen(name)) {
        l->fd = STDERR_FILENO;
        l_wf->fd = STDERR_FILENO;
    } else {
        l->fd = open(l->name, O_WRONLY | O_APPEND | O_CREAT, 0644);
        if (l->fd < 0) {
            log_stderr("opening log file '%s' failed: %s", name,strerror(errno));
            return -1; 
        }
        l_wf->fd = open(l_wf->name, O_WRONLY | O_APPEND | O_CREAT, 0644); 
        if (l->fd < 0) {
            log_stderr("opening log file '%s' failed: %s", l_wf->name,strerror(errno));
            return -1;
        }
    }
    if (thread) { 
        l->log_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t)); 
        pthread_mutex_init (l->log_mutex, NULL);
        l_wf->log_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t)); 
        pthread_mutex_init (l_wf->log_mutex, NULL);
    } else {
        l->log_mutex = NULL;
        l_wf->log_mutex = NULL;
    }
    return 0;
}

void log_reopen(void) {
    struct logger *l = &logger;
    struct logger *l_wf = &logger_wf;

    if (l->fd != STDERR_FILENO) {
        if (l->log_mutex) {
            pthread_mutex_lock(l->log_mutex);
        }
        close(l->fd);
        l->fd = open(l->name, O_WRONLY | O_APPEND | O_CREAT, 0644);
        if (l->fd < 0) {
            log_stderr("reopening log file '%s' failed, ignored: %s", l->name,strerror(errno));
        }
        if (l->log_mutex) {
            pthread_mutex_unlock(l->log_mutex);
        }
        if (l_wf->log_mutex) {
            pthread_mutex_lock(l_wf->log_mutex);
        }
        close(l_wf->fd);
        l_wf->fd = open(l_wf->name, O_WRONLY | O_APPEND | O_CREAT, 0644);
        if (l_wf->fd < 0) {
            log_stderr("reopening log file '%s' failed, ignored: %s", l_wf->name,strerror(errno));
        }
        if (l_wf->log_mutex) {
            pthread_mutex_unlock(l_wf->log_mutex);
        }
    }   
}

int log_loggable(int level) {
    struct logger *l = &logger;

    if (level > l->level) {
        return 0;
    }

    return 1;
}

int myvprintf(char *buf, size_t size, const char *fmt, ...) {
    va_list args;
    int n;

    va_start(args, fmt);
    n = vsnprintf(buf, size, fmt, args);
    if (n <= 0) {
        n = 0;
    } else if (n >= (int) size) {
        n = (int)(size - 1);
    }   
    va_end(args);

    return n;
}
void _log(const char *file, const char* func,int line, int level, const char *fmt, ...) {
    struct logger *l = NULL;
    if (level < LOG_WARN) {
        l  = &logger_wf;
    } else {
        l  = &logger;
    }
    int len, size, errno_save;
    char buf[LOG_MAX_LEN];
    va_list args;
    ssize_t n;
    struct timeval tv; 

    if (l->fd < 0) {
        return;
    } 

    len = 0;            /* length of output buffer */
    size = LOG_MAX_LEN; /* size of output buffer */

    gettimeofday(&tv, NULL);
    buf[len++] = '[';
    struct tm now = {0};
    localtime_r(&tv.tv_sec, &now);
    len += strftime(buf + len, size - len, "%Y-%m-%d %H:%M:%S.", &now);
    len += myvprintf(buf + len, size - len, "%03ld", tv.tv_usec/1000);
    len += myvprintf(buf + len, size - len, "] %d %s %s %s:%d ", gettid(), translate(level), file, func, line);

    va_start(args, fmt);
    len += myvprintf(buf + len, size - len, fmt, args);
    va_end(args);

    buf[len++] = '\n';

    if (l->log_mutex) {
        pthread_mutex_lock(l->log_mutex);
    }
    n = write(l->fd, buf, len);
    if (l->log_mutex) {
        pthread_mutex_unlock(l->log_mutex);
    }
}

void log_stderr(const char *fmt, ...) {
    int len, size;
    char buf[LOG_MAX_LEN];
    va_list args;
    ssize_t n;

    len = 0;            /* length of output buffer */
    size = LOG_MAX_LEN; /* size of output buffer */

    va_start(args, fmt);
    len += myvprintf(buf + len, size - len, fmt, args);
    va_end(args);

    buf[len++] = '\n';

    n = write(STDERR_FILENO, buf, len);
}

