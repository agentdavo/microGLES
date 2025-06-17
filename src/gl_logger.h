#ifndef GL_LOGGER_H
#define GL_LOGGER_H
/**
 * @file gl_logger.h
 * @brief Simple logging utilities for the renderer.
 */

#include <stdarg.h>
#include "portable/c11threads.h"
#include <stdatomic.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_INFO,
	LOG_LEVEL_WARN,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_FATAL
} LogLevel;

/**
 * Initialize the logger.
 *
 * @param path  File to append logs to, or NULL for stdout only.
 * @param level Minimum severity to record.
 * @return 1 on success, 0 on failure.
 */
int logger_init(const char *path, LogLevel level);
void logger_shutdown(void);
void LogMessage(LogLevel level, const char *format, ...);

#define LOG_DEBUG(...) LogMessage(LOG_LEVEL_DEBUG, __VA_ARGS__)
#define LOG_INFO(...) LogMessage(LOG_LEVEL_INFO, __VA_ARGS__)
#define LOG_WARN(...) LogMessage(LOG_LEVEL_WARN, __VA_ARGS__)
#define LOG_ERROR(...) LogMessage(LOG_LEVEL_ERROR, __VA_ARGS__)
#define LOG_FATAL(...) LogMessage(LOG_LEVEL_FATAL, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* GL_LOGGER_H */
