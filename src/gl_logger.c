#include "gl_logger.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

#define LOG_BUFFER_SIZE 8192
#define MESSAGE_SIZE 256

static atomic_bool g_running = ATOMIC_VAR_INIT(false);
static thrd_t g_thread;
static LogLevel g_level = LOG_LEVEL_DEBUG;
static char g_buffer[LOG_BUFFER_SIZE][MESSAGE_SIZE];
static _Alignas(64) atomic_uint_fast64_t g_head;
static _Alignas(64) atomic_uint_fast64_t g_tail;

static void log_output_char(char c)
{
	fputc(c, stdout);
}

static const char *level_string(LogLevel level)
{
	switch (level) {
	case LOG_LEVEL_DEBUG:
		return "DEBUG";
	case LOG_LEVEL_INFO:
		return "INFO";
	case LOG_LEVEL_WARN:
		return "WARN";
	case LOG_LEVEL_ERROR:
		return "ERROR";
	case LOG_LEVEL_FATAL:
		return "FATAL";
	default:
		return "UNKNOWN";
	}
}

static void get_timestamp(char *buf, size_t size)
{
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	strftime(buf, size, "%Y-%m-%d %H:%M:%S", tm);
}

static int logger_thread_main(void *arg)
{
	(void)arg;
	while (atomic_load_explicit(&g_running, memory_order_acquire)) {
		uint64_t head =
			atomic_load_explicit(&g_head, memory_order_relaxed);
		uint64_t tail =
			atomic_load_explicit(&g_tail, memory_order_acquire);
		if (head == tail) {
			thrd_yield();
			continue;
		}
		char *msg = g_buffer[head % LOG_BUFFER_SIZE];
		for (char *c = msg; *c; ++c)
			log_output_char(*c);
		log_output_char('\n');
		atomic_store_explicit(&g_head, head + 1, memory_order_release);
	}
	return 0;
}

int logger_init(const char *path, LogLevel level)
{
	g_level = level;
	atomic_init(&g_head, 0);
	atomic_init(&g_tail, 0);
	atomic_store_explicit(&g_running, true, memory_order_release);
	(void)path; /* file output not implemented */
	thrd_create(&g_thread, logger_thread_main, NULL);
	return 1;
}

void logger_shutdown(void)
{
	atomic_store_explicit(&g_running, false, memory_order_release);
	thrd_join(g_thread, NULL);
}

void LogMessage(LogLevel level, const char *format, ...)
{
	if (level < g_level)
		return;

	uint64_t tail = atomic_load_explicit(&g_tail, memory_order_relaxed);
	uint64_t head = atomic_load_explicit(&g_head, memory_order_acquire);
	if (tail - head >= LOG_BUFFER_SIZE)
		return; /* drop */

	char *msg = g_buffer[tail % LOG_BUFFER_SIZE];
	char ts[20];
	get_timestamp(ts, sizeof(ts));
	int off = snprintf(msg, MESSAGE_SIZE, "[%s] [%s] ", ts,
			   level_string(level));
	va_list args;
	va_start(args, format);
	vsnprintf(msg + off, MESSAGE_SIZE - off, format, args);
	va_end(args);
	atomic_store_explicit(&g_tail, tail + 1, memory_order_release);
}
