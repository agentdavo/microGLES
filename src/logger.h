/* logger.h */

#ifndef LOGGER_H
#define LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* Log Levels */
typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
} LogLevel;

/* Initialize the logger
 * - log_file_path: Path to the log file. If NULL, logs will only be printed to the console.
 * - level: Minimum log level to record.
 * Returns 1 on success, 0 on failure.
 */
int InitLogger(const char *log_file_path, LogLevel level);

/* Shutdown the logger and release resources */
void ShutdownLogger(void);

/* Log a message with the specified log level */
void LogMessage(LogLevel level, const char *format, ...);

/* Convenience macros for logging at different levels */
#define LOG_DEBUG(...) LogMessage(LOG_LEVEL_DEBUG, __VA_ARGS__)
#define LOG_INFO(...)  LogMessage(LOG_LEVEL_INFO,  __VA_ARGS__)
#define LOG_WARN(...)  LogMessage(LOG_LEVEL_WARN,  __VA_ARGS__)
#define LOG_ERROR(...) LogMessage(LOG_LEVEL_ERROR, __VA_ARGS__)
#define LOG_FATAL(...) LogMessage(LOG_LEVEL_FATAL, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* LOGGER_H */
