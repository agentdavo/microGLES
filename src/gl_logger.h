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
void logger_set_indent(unsigned level);

#define LOG_DEBUG(...) LogMessage(LOG_LEVEL_DEBUG, __VA_ARGS__)
#define LOG_INFO(...) LogMessage(LOG_LEVEL_INFO, __VA_ARGS__)
#define LOG_WARN(...) LogMessage(LOG_LEVEL_WARN, __VA_ARGS__)
#define LOG_ERROR(...) LogMessage(LOG_LEVEL_ERROR, __VA_ARGS__)
#define LOG_FATAL(...) LogMessage(LOG_LEVEL_FATAL, __VA_ARGS__)

#ifndef NDEBUG
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_GREEN "\x1b[32m"
#define ANSI_RESET "\x1b[0m"
#define LOG_FRAME_START(idx)                                                  \
	do {                                                                  \
		logger_set_indent(0);                                         \
		LogMessage(LOG_LEVEL_INFO, ANSI_YELLOW "Frame %d" ANSI_RESET, \
			   (idx));                                            \
	} while (0)
#define LOG_STAGE_START(name)                                          \
	do {                                                           \
		LogMessage(LOG_LEVEL_INFO, ANSI_GREEN "%s" ANSI_RESET, \
			   (name));                                    \
		logger_set_indent(1);                                  \
	} while (0)
#else
#define LOG_FRAME_START(idx) ((void)0)
#define LOG_STAGE_START(name) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* GL_LOGGER_H */
