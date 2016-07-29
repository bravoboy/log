#ifndef C_LOG_H_
#define C_LOG_H_

#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>

#define gettid() syscall(SYS_gettid)

struct logger {
    char name[64];  /* log file name */
    int  level;  /* log level */
    int  fd;     /* log file descriptor */
    pthread_mutex_t *log_mutex;
};

#define LOG_ERR     0   /* error conditions */
#define LOG_WARN    1   /* warning conditions */
#define LOG_INFO    2   /* informational */
#define LOG_DEBUG   3   /* debug messages */

#define LOG_MAX_LEN 4096 /* max length of log message */

#define log_debug(...) do {                                                 \
    if (log_loggable(LOG_DEBUG) != 0) {                                     \
        _log(__FILE__,__func__,__LINE__,LOG_DEBUG,__VA_ARGS__);             \
    }                                                                       \
} while (0)

#define log_info(...) do {                                                 \
    if (log_loggable(LOG_INFO) != 0) {                                     \
        _log(__FILE__, __func__,__LINE__,LOG_INFO,__VA_ARGS__);            \
    }                                                                       \
} while (0)

#define log_error(...) do {                                                 \
    _log(__FILE__, __LINE__, __func__, __VA_ARGS__);                        \
} while (0)

#define log_warn(...) do {                                                  \
    _log(__FILE__, __LINE__, __func__, __VA_ARGS__);                        \
} while (0)

int log_init(int level, char *filename, int thread);
void log_reopen(void);
int log_loggable(int level);
void _log(const char *file, const char *func, int line, int level, const char *fmt, ...);
void log_stderr(const char *fmt, ...);
#endif
