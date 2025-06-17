#include "gl_memory_tracker.h"
#include "gl_logger.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "portable/c11threads.h"

static char *dup_cstring(const char *src)
{
	if (!src)
		return NULL;
	size_t len = strlen(src) + 1;
	char *dst = malloc(len);
	if (dst)
		memcpy(dst, src, len);
	return dst;
}

#define INITIAL_ALLOCATIONS 1024

typedef struct {
	void *ptr;
	size_t size;
	stage_tag_t stage;
	char *type;
	int line;
	bool active;
} allocation_t;

static allocation_t *g_allocs;
static size_t g_capacity;
static mtx_t g_mutex;
static size_t g_current;
static size_t g_peak;
static size_t g_stage_current[STAGE_COUNT];
static size_t g_stage_peak[STAGE_COUNT];

int memory_tracker_init(void)
{
	mtx_init(&g_mutex, mtx_plain);
	g_capacity = INITIAL_ALLOCATIONS;
	g_allocs = calloc(g_capacity, sizeof(allocation_t));
	if (!g_allocs)
		return 0;
	g_current = g_peak = 0;
	memset(g_stage_current, 0, sizeof(g_stage_current));
	memset(g_stage_peak, 0, sizeof(g_stage_peak));
	LogMessage(LOG_LEVEL_INFO, "Memory tracker initialized");
	return 1;
}

static void record_alloc(void *ptr, size_t size, stage_tag_t stage,
			 const char *type, int line)
{
	for (size_t i = 0; i < g_capacity; ++i) {
		if (!g_allocs[i].active) {
			g_allocs[i] = (allocation_t){ ptr,   size,
						      stage, dup_cstring(type),
						      line,  true };
			g_current += size;
			g_stage_current[stage] += size;
			if (g_current > g_peak)
				g_peak = g_current;
			if (g_stage_current[stage] > g_stage_peak[stage])
				g_stage_peak[stage] = g_stage_current[stage];
			return;
		}
	}
	size_t new_cap = g_capacity * 2;
	allocation_t *new_allocs =
		realloc(g_allocs, new_cap * sizeof(allocation_t));
	if (!new_allocs) {
		LogMessage(LOG_LEVEL_ERROR, "Memory tracker pool exhausted");
		return;
	}
	memset(new_allocs + g_capacity, 0,
	       (new_cap - g_capacity) * sizeof(allocation_t));
	g_allocs = new_allocs;
	size_t idx = g_capacity;
	g_capacity = new_cap;
	g_allocs[idx] = (allocation_t){ ptr,  size, stage, dup_cstring(type),
					line, true };
	g_current += size;
	g_stage_current[stage] += size;
	if (g_current > g_peak)
		g_peak = g_current;
	if (g_stage_current[stage] > g_stage_peak[stage])
		g_stage_peak[stage] = g_stage_current[stage];
}

void *memory_tracker_allocate(size_t size, stage_tag_t stage, const char *type,
			      int line)
{
	void *p = malloc(size);
	if (!p)
		return NULL;
	mtx_lock(&g_mutex);
	record_alloc(p, size, stage, type, line);
	mtx_unlock(&g_mutex);
	return p;
}

void *memory_tracker_calloc(size_t n, size_t size, stage_tag_t stage,
			    const char *type, int line)
{
	void *p = calloc(n, size);
	if (!p)
		return NULL;
	mtx_lock(&g_mutex);
	record_alloc(p, n * size, stage, type, line);
	mtx_unlock(&g_mutex);
	return p;
}

void *memory_tracker_realloc(void *ptr, size_t size, stage_tag_t stage,
			     const char *type, int line)
{
	mtx_lock(&g_mutex);
	allocation_t *rec = NULL;
	if (ptr) {
		for (size_t i = 0; i < g_capacity; ++i) {
			if (g_allocs[i].active && g_allocs[i].ptr == ptr) {
				rec = &g_allocs[i];
				break;
			}
		}
	}
	void *np = realloc(ptr, size);
	if (!np) {
		mtx_unlock(&g_mutex);
		return NULL;
	}
	if (rec) {
		g_current -= rec->size;
		g_stage_current[rec->stage] -= rec->size;
		free(rec->type);
		*rec = (allocation_t){ np,   size, stage, dup_cstring(type),
				       line, true };
	} else {
		record_alloc(np, size, stage, type, line);
		mtx_unlock(&g_mutex);
		return np;
	}
	g_current += size;
	g_stage_current[stage] += size;
	if (g_current > g_peak)
		g_peak = g_current;
	if (g_stage_current[stage] > g_stage_peak[stage])
		g_stage_peak[stage] = g_stage_current[stage];
	mtx_unlock(&g_mutex);
	return np;
}

void memory_tracker_free(void *ptr, stage_tag_t stage, const char *type,
			 int line)
{
	(void)stage;
	(void)type;
	(void)line;
	if (!ptr)
		return;
	mtx_lock(&g_mutex);
	for (size_t i = 0; i < g_capacity; ++i) {
		if (g_allocs[i].active && g_allocs[i].ptr == ptr) {
			g_current -= g_allocs[i].size;
			g_stage_current[g_allocs[i].stage] -= g_allocs[i].size;
			g_allocs[i].active = false;
			free(g_allocs[i].type);
			break;
		}
	}
	mtx_unlock(&g_mutex);
	free(ptr);
}

size_t memory_tracker_current(void)
{
	return g_current;
}

size_t memory_tracker_peak(void)
{
	return g_peak;
}

void memory_tracker_report(void)
{
	LogMessage(LOG_LEVEL_INFO, "Current memory %zu bytes", g_current);
	LogMessage(LOG_LEVEL_INFO, "Peak memory %zu bytes", g_peak);
	for (int s = 0; s < STAGE_COUNT; ++s) {
		LogMessage(LOG_LEVEL_INFO, "Stage %d peak %zu bytes", s,
			   g_stage_peak[s]);
	}
}

void memory_tracker_shutdown(void)
{
	mtx_lock(&g_mutex);
	for (size_t i = 0; i < g_capacity; ++i) {
		if (g_allocs[i].active) {
			LogMessage(LOG_LEVEL_WARN,
				   "Leaked %zu bytes from %s:%d",
				   g_allocs[i].size, g_allocs[i].type,
				   g_allocs[i].line);
			free(g_allocs[i].type);
		}
	}
	for (int s = 0; s < STAGE_COUNT; ++s) {
		LogMessage(LOG_LEVEL_INFO, "Stage %d peak memory %zu bytes", s,
			   g_stage_peak[s]);
	}
	LogMessage(LOG_LEVEL_INFO, "Peak memory usage %zu bytes", g_peak);
	mtx_unlock(&g_mutex);
	free(g_allocs);
	mtx_destroy(&g_mutex);
}
