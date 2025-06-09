/* logger.c */

#include "logger.h"
#include <time.h>
#include <string.h>
#include <pthread.h>

static FILE *log_file = NULL;
static LogLevel current_log_level = LOG_LEVEL_DEBUG;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Converts LogLevel to string */
static const char* LogLevelToString(LogLevel level) {
    switch(level) {
        case LOG_LEVEL_DEBUG: return "DEBUG";
        case LOG_LEVEL_INFO:  return "INFO";
        case LOG_LEVEL_WARN:  return "WARN";
        case LOG_LEVEL_ERROR: return "ERROR";
        case LOG_LEVEL_FATAL: return "FATAL";
        default:              return "UNKNOWN";
    }
}

/* Initializes the logger */
int InitLogger(const char *log_file_path, LogLevel level) {
    pthread_mutex_lock(&log_mutex);
    
    current_log_level = level;
    
    if (log_file_path) {
        log_file = fopen(log_file_path, "a");
        if (!log_file) {
            fprintf(stderr, "Logger: Failed to open log file: %s\n", log_file_path);
            pthread_mutex_unlock(&log_mutex);
            return 0;
        }
    }
    
    pthread_mutex_unlock(&log_mutex);
    return 1;
}

/* Shuts down the logger */
void ShutdownLogger(void) {
    pthread_mutex_lock(&log_mutex);
    
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
    
    pthread_mutex_unlock(&log_mutex);
    pthread_mutex_destroy(&log_mutex);
}

/* Logs a message with the specified level */
void LogMessage(LogLevel level, const char *format, ...) {
    if (level < current_log_level) {
        return;
    }
    
    pthread_mutex_lock(&log_mutex);
    
    /* Get current time */
    time_t raw_time;
    struct tm *time_info;
    char time_buffer[20];
    
    time(&raw_time);
    time_info = localtime(&raw_time);
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", time_info);
    
    /* Prepare the log message */
    fprintf(stdout, "[%s] [%s] ", time_buffer, LogLevelToString(level));
    
    if (log_file) {
        fprintf(log_file, "[%s] [%s] ", time_buffer, LogLevelToString(level));
    }
    
    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
    
    if (log_file) {
        va_start(args, format);
        vfprintf(log_file, format, args);
        va_end(args);
    }
    
    fprintf(stdout, "\n");
    if (log_file) {
        fprintf(log_file, "\n");
        fflush(log_file);
    }
    
    pthread_mutex_unlock(&log_mutex);
}
