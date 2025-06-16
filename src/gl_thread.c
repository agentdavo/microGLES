#include "gl_thread.h"
#include "gl_logger.h"
#include "function_profile.h"
#include <stdatomic.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "texture_cache.h"

#define MAX_TASKS 256
#define LOCAL_QUEUE_SIZE 128

typedef struct {
	task_function_t function;
	void *task_data;
	uint64_t token;
	stage_tag_t stage;
} task_t;

typedef struct {
	uint64_t task_count;
	uint64_t steal_attempts;
	uint64_t steal_successes;
	uint64_t contention_events;
	uint64_t task_cycles;
	uint64_t idle_cycles;
	uint64_t steal_cycles;
	uint64_t tile_jobs;
	uint64_t cache_hits;
	uint64_t cache_misses;
} stage_profile_t;

typedef struct {
	stage_profile_t stages[STAGE_COUNT];
} thread_profile_t;

typedef struct {
	task_t entries[LOCAL_QUEUE_SIZE];
	_Alignas(64) atomic_uint_fast64_t head;
	_Alignas(64) atomic_uint_fast64_t tail;
	thread_profile_t profile_data;
} task_queue_t;

static task_queue_t *g_local_queues;
static task_t g_global_queue[MAX_TASKS];
static _Alignas(64) atomic_uint_fast64_t g_global_head;
static _Alignas(64) atomic_uint_fast64_t g_global_tail;
static thrd_t *g_worker_threads;
static int g_num_threads;
static _Thread_local int tls_tid = -1;
static atomic_bool g_shutdown_flag = false;
static atomic_bool g_profiling_enabled = false;
static _Thread_local thread_profile_t g_thread_profile;
static texture_cache_t *g_texture_caches;
static _Thread_local texture_cache_t *tls_cache;
static cnd_t g_wakeup;
static mtx_t g_wakeup_mutex;

#if defined(__x86_64__)
#include <x86intrin.h>
#endif

static uint64_t get_cycles(void)
{
#if defined(__x86_64__)
	return __rdtsc();
#elif defined(__aarch64__)
	uint64_t val;
	asm volatile("mrs %0, cntvct_el0" : "=r"(val));
	return val;
#else
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000000000ull + ts.tv_nsec;
#endif
}

static const char *stage_names[STAGE_COUNT] = { "Vertex", "Primitive", "Raster",
						"Fragment", "Framebuffer" };

static bool steal_task(int thread_id, task_t *out, bool profiling)
{
	uint64_t start = profiling ? get_cycles() : 0;
	for (int i = 0; i < g_num_threads; ++i) {
		if (i == thread_id)
			continue;
		task_queue_t *victim = &g_local_queues[i];
		uint64_t head = atomic_load_explicit(&victim->head,
						     memory_order_relaxed);
		uint64_t tail = atomic_load_explicit(&victim->tail,
						     memory_order_acquire);
		if (head >= tail)
			continue;
		uint64_t steal_count = (tail - head) / 4;
		if (steal_count == 0)
			steal_count = 1;
		for (uint64_t j = 0; j < steal_count; ++j) {
			uint64_t slot = head % LOCAL_QUEUE_SIZE;
			task_t task = victim->entries[slot];
			if (task.token != head)
				break;
			if (atomic_compare_exchange_strong_explicit(
				    &victim->head, &head, head + 1,
				    memory_order_release,
				    memory_order_relaxed)) {
				*out = task;
				if (profiling) {
					g_thread_profile.stages[task.stage]
						.steal_successes++;
					g_thread_profile.stages[task.stage]
						.steal_cycles +=
						get_cycles() - start;
				}
				return true;
			}
			if (profiling) {
				g_thread_profile.stages[task.stage]
					.contention_events++;
				g_thread_profile.stages[task.stage]
					.steal_attempts++;
			}
		}
	}
	if (profiling)
		g_thread_profile.stages[STAGE_VERTEX].steal_cycles +=
			get_cycles() - start;
	return false;
}

static int worker_thread_main(void *arg)
{
	int thread_id = *(int *)arg;
	free(arg);
	tls_tid = thread_id;
	tls_cache = &g_texture_caches[thread_id];
	task_queue_t *local_queue = &g_local_queues[thread_id];
	int idle_loops = 0;
	while (!atomic_load_explicit(&g_shutdown_flag, memory_order_acquire)) {
		bool profiling = atomic_load_explicit(&g_profiling_enabled,
						      memory_order_acquire);
		uint64_t start_cycles = profiling ? get_cycles() : 0;
		uint64_t head = atomic_load_explicit(&local_queue->head,
						     memory_order_relaxed);
		uint64_t tail = atomic_load_explicit(&local_queue->tail,
						     memory_order_acquire);
		if (head < tail) {
			uint64_t new_tail = tail - 1;
			uint64_t slot = new_tail % LOCAL_QUEUE_SIZE;
			task_t task = local_queue->entries[slot];
			if (task.token == new_tail) {
				if (atomic_compare_exchange_strong_explicit(
					    &local_queue->tail, &tail, new_tail,
					    memory_order_release,
					    memory_order_relaxed)) {
					if (profiling) {
						g_thread_profile
							.stages[task.stage]
							.task_count++;
						if (task.stage ==
						    STAGE_FRAGMENT)
							g_thread_profile
								.stages[STAGE_FRAGMENT]
								.tile_jobs++;
					}
					uint64_t ts = profiling ? get_cycles() :
								  0;
					task.function(task.task_data);
					if (profiling)
						g_thread_profile
							.stages[task.stage]
							.task_cycles +=
							get_cycles() - ts;
					idle_loops = 0;
					continue;
				}
				if (profiling)
					g_thread_profile.stages[task.stage]
						.contention_events++;
			}
		}
		head = atomic_load_explicit(&g_global_head,
					    memory_order_relaxed);
		tail = atomic_load_explicit(&g_global_tail,
					    memory_order_acquire);
		if (head < tail) {
			uint64_t slot = head % MAX_TASKS;
			task_t task = g_global_queue[slot];
			if (task.token == head) {
				if (atomic_compare_exchange_strong_explicit(
					    &g_global_head, &head, head + 1,
					    memory_order_release,
					    memory_order_relaxed)) {
					if (profiling) {
						g_thread_profile
							.stages[task.stage]
							.task_count++;
						if (task.stage ==
						    STAGE_FRAGMENT)
							g_thread_profile
								.stages[STAGE_FRAGMENT]
								.tile_jobs++;
					}
					uint64_t ts = profiling ? get_cycles() :
								  0;
					task.function(task.task_data);
					if (profiling)
						g_thread_profile
							.stages[task.stage]
							.task_cycles +=
							get_cycles() - ts;
					idle_loops = 0;
					continue;
				}
				if (profiling)
					g_thread_profile.stages[task.stage]
						.contention_events++;
			}
		}
		task_t stolen;
		if (steal_task(thread_id, &stolen, profiling)) {
			if (profiling) {
				g_thread_profile.stages[stolen.stage]
					.task_count++;
				if (stolen.stage == STAGE_FRAGMENT)
					g_thread_profile.stages[STAGE_FRAGMENT]
						.tile_jobs++;
			}
			uint64_t ts = profiling ? get_cycles() : 0;
			stolen.function(stolen.task_data);
			if (profiling)
				g_thread_profile.stages[stolen.stage]
					.task_cycles += get_cycles() - ts;
			idle_loops = 0;
			continue;
		}
		if (profiling) {
			g_thread_profile.stages[STAGE_VERTEX].idle_cycles +=
				get_cycles() - start_cycles;
			mtx_lock(&g_wakeup_mutex);
			uint64_t lh = atomic_load_explicit(
				&local_queue->head, memory_order_relaxed);
			uint64_t lt = atomic_load_explicit(
				&local_queue->tail, memory_order_acquire);
			uint64_t gh = atomic_load_explicit(
				&g_global_head, memory_order_relaxed);
			uint64_t gt = atomic_load_explicit(
				&g_global_tail, memory_order_acquire);
			if (lh >= lt && gh >= gt &&
			    !atomic_load_explicit(&g_shutdown_flag,
						  memory_order_relaxed))
				cnd_wait(&g_wakeup, &g_wakeup_mutex);
			mtx_unlock(&g_wakeup_mutex);
			idle_loops = 0;
		} else {
			idle_loops++;
			if (idle_loops > 100) {
				struct timespec ts = { 0, 50 };
				thrd_sleep(&ts, NULL);
			} else {
				thrd_yield();
			}
		}
	}
	for (int s = 0; s < STAGE_COUNT; ++s)
		g_local_queues[thread_id].profile_data.stages[s] =
			g_thread_profile.stages[s];
	return 0;
}

void thread_pool_init(int num_threads)
{
	g_num_threads = num_threads > 0 ? num_threads : 1;
	g_worker_threads = malloc(sizeof(thrd_t) * g_num_threads);
	g_local_queues = calloc(g_num_threads, sizeof(task_queue_t));
	g_texture_caches = calloc(g_num_threads, sizeof(texture_cache_t));
	mtx_init(&g_wakeup_mutex, mtx_plain);
	cnd_init(&g_wakeup);
	for (int i = 0; i < g_num_threads; ++i)
		texture_cache_init(&g_texture_caches[i]);
	atomic_init(&g_global_head, 0);
	atomic_init(&g_global_tail, 0);
	atomic_store(&g_shutdown_flag, false);
	atomic_store(&g_profiling_enabled, false);
	for (int i = 0; i < g_num_threads; i++) {
		atomic_init(&g_local_queues[i].head, 0);
		atomic_init(&g_local_queues[i].tail, 0);
		int *tid = malloc(sizeof(int));
		*tid = i;
		thrd_create(&g_worker_threads[i], worker_thread_main, tid);
	}
}

void thread_pool_submit(task_function_t func, void *task_data,
			stage_tag_t stage)
{
	int thread_id = (tls_tid >= 0) ? tls_tid : 0;
	task_queue_t *local_queue = &g_local_queues[thread_id];
	uint64_t tail =
		atomic_load_explicit(&local_queue->tail, memory_order_relaxed);
	uint64_t head =
		atomic_load_explicit(&local_queue->head, memory_order_acquire);
	if (tail - head < LOCAL_QUEUE_SIZE) {
		uint64_t slot = tail % LOCAL_QUEUE_SIZE;
		local_queue->entries[slot] = (task_t){ .function = func,
						       .task_data = task_data,
						       .token = tail,
						       .stage = stage };
		atomic_store_explicit(&local_queue->tail, tail + 1,
				      memory_order_release);
	} else {
		tail = atomic_load_explicit(&g_global_tail,
					    memory_order_relaxed);
		uint64_t slot = tail % MAX_TASKS;
		g_global_queue[slot] = (task_t){ .function = func,
						 .task_data = task_data,
						 .token = tail,
						 .stage = stage };
		atomic_store_explicit(&g_global_tail, tail + 1,
				      memory_order_release);
	}
	mtx_lock(&g_wakeup_mutex);
	cnd_signal(&g_wakeup);
	mtx_unlock(&g_wakeup_mutex);
}

void thread_pool_wait(void)
{
	while (atomic_load_explicit(&g_global_head, memory_order_acquire) <
	       atomic_load_explicit(&g_global_tail, memory_order_relaxed))
		thrd_yield();
	for (int i = 0; i < g_num_threads; i++) {
		while (atomic_load_explicit(&g_local_queues[i].head,
					    memory_order_acquire) <
		       atomic_load_explicit(&g_local_queues[i].tail,
					    memory_order_relaxed))
			thrd_yield();
	}
}

int thread_pool_wait_timeout(uint32_t ms)
{
	struct timespec start;
	clock_gettime(CLOCK_MONOTONIC, &start);
	for (;;) {
		bool done = atomic_load_explicit(&g_global_head,
						 memory_order_acquire) >=
			    atomic_load_explicit(&g_global_tail,
						 memory_order_relaxed);
		if (done) {
			done = true;
			for (int i = 0; i < g_num_threads; i++) {
				if (atomic_load_explicit(&g_local_queues[i].head,
							 memory_order_acquire) <
				    atomic_load_explicit(
					    &g_local_queues[i].tail,
					    memory_order_relaxed)) {
					done = false;
					break;
				}
			}
			if (done)
				return 1;
		}
		struct timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		uint64_t elapsed =
			(uint64_t)(now.tv_sec - start.tv_sec) * 1000 +
			(uint64_t)(now.tv_nsec - start.tv_nsec) / 1000000;
		if (elapsed >= ms)
			return 0;
		thrd_yield();
	}
}

void thread_pool_shutdown(void)
{
	thread_pool_wait();
	thread_profile_stop();
	atomic_store_explicit(&g_shutdown_flag, true, memory_order_release);
	mtx_lock(&g_wakeup_mutex);
	cnd_broadcast(&g_wakeup);
	mtx_unlock(&g_wakeup_mutex);
	for (int i = 0; i < g_num_threads; ++i)
		thrd_join(g_worker_threads[i], NULL);
	thread_profile_report();
	free(g_worker_threads);
	free(g_local_queues);
	free(g_texture_caches);
	cnd_destroy(&g_wakeup);
	mtx_destroy(&g_wakeup_mutex);
}

bool thread_pool_active(void)
{
	return !atomic_load_explicit(&g_shutdown_flag, memory_order_acquire);
}

void thread_profile_start(void)
{
	atomic_store_explicit(&g_profiling_enabled, true, memory_order_release);
	g_thread_profile = (thread_profile_t){ 0 };
	for (int i = 0; i < g_num_threads; ++i)
		memset(&g_local_queues[i].profile_data, 0,
		       sizeof(thread_profile_t));
	function_profile_reset();
}

void thread_profile_stop(void)
{
	atomic_store_explicit(&g_profiling_enabled, false,
			      memory_order_release);
}

void thread_profile_report(void)
{
	LOG_INFO("Thread Pool Profiling Results:");
	for (int stage = 0; stage < STAGE_COUNT; stage++) {
		uint64_t t_tasks = 0, t_steals = 0, t_attempts = 0,
			 t_contention = 0, t_tiles = 0, t_hits = 0, t_miss = 0;
		uint64_t t_task_cycles = 0, t_idle_cycles = 0,
			 t_steal_cycles = 0;
		for (int i = 0; i < g_num_threads; i++) {
			stage_profile_t *pd =
				&g_local_queues[i].profile_data.stages[stage];
			t_tasks += pd->task_count;
			t_steals += pd->steal_successes;
			t_attempts += pd->steal_attempts;
			t_contention += pd->contention_events;
			t_task_cycles += pd->task_cycles;
			t_tiles += pd->tile_jobs;
			t_idle_cycles += pd->idle_cycles;
			t_steal_cycles += pd->steal_cycles;
			t_hits += pd->cache_hits;
			t_miss += pd->cache_misses;
		}
		LOG_INFO("Stage %s:", stage_names[stage]);
		LOG_INFO("  Total Tasks: %llu", t_tasks);
		LOG_INFO("  Total Steal Attempts: %llu", t_attempts);
		LOG_INFO("  Total Steal Successes: %llu", t_steals);
		LOG_INFO("  Total Contention: %llu", t_contention);
		if (stage == STAGE_FRAGMENT) {
			LOG_INFO("  Tile Jobs: %llu", t_tiles);
			LOG_INFO("  Cache Hits: %llu", t_hits);
			LOG_INFO("  Cache Misses: %llu", t_miss);
		}
		LOG_INFO("  Avg Task Cycles: %llu",
			 t_task_cycles / (t_tasks ? t_tasks : 1));
		LOG_INFO("  Total Idle Cycles: %llu", t_idle_cycles);
		LOG_INFO("  Total Steal Cycles: %llu", t_steal_cycles);
	}
	uint64_t g_tasks = 0, g_steals = 0, g_attempts = 0, g_contention = 0;
	uint64_t g_task_cycles = 0, g_idle_cycles = 0, g_steal_cycles = 0,
		 g_tiles = 0, g_hits = 0, g_miss = 0;
	for (int stage = 0; stage < STAGE_COUNT; stage++)
		for (int i = 0; i < g_num_threads; i++) {
			stage_profile_t *pd =
				&g_local_queues[i].profile_data.stages[stage];
			g_tasks += pd->task_count;
			g_steals += pd->steal_successes;
			g_attempts += pd->steal_attempts;
			g_contention += pd->contention_events;
			g_task_cycles += pd->task_cycles;
			g_idle_cycles += pd->idle_cycles;
			g_steal_cycles += pd->steal_cycles;
			g_tiles += pd->tile_jobs;
			g_hits += pd->cache_hits;
			g_miss += pd->cache_misses;
		}
	LOG_INFO("Global Summary:");
	LOG_INFO("  Total Tasks: %llu", g_tasks);
	LOG_INFO("  Total Steal Attempts: %llu", g_attempts);
	LOG_INFO("  Total Steal Successes: %llu", g_steals);
	LOG_INFO("  Total Contention: %llu", g_contention);
	LOG_INFO("  Avg Task Cycles: %llu",
		 g_task_cycles / (g_tasks ? g_tasks : 1));
	LOG_INFO("  Total Idle Cycles: %llu", g_idle_cycles);
	LOG_INFO("  Total Steal Cycles: %llu", g_steal_cycles);
	LOG_INFO("  Total Tile Jobs: %llu", g_tiles);
	LOG_INFO("  Total Cache Hits: %llu", g_hits);
	LOG_INFO("  Total Cache Misses: %llu", g_miss);
	function_profile_report();
}

texture_cache_t *thread_get_texture_cache(void)
{
	return tls_cache;
}

void thread_profile_cache_hit(void)
{
	if (atomic_load_explicit(&g_profiling_enabled, memory_order_relaxed))
		g_thread_profile.stages[STAGE_FRAGMENT].cache_hits++;
}

void thread_profile_cache_miss(void)
{
	if (atomic_load_explicit(&g_profiling_enabled, memory_order_relaxed))
		g_thread_profile.stages[STAGE_FRAGMENT].cache_misses++;
}

void thread_profile_get_cache_stats(uint64_t *hits, uint64_t *misses)
{
	uint64_t h = 0, m = 0;
	for (int i = 0; i < g_num_threads; ++i) {
		stage_profile_t *pd =
			&g_local_queues[i].profile_data.stages[STAGE_FRAGMENT];
		h += pd->cache_hits;
		m += pd->cache_misses;
	}
	if (hits)
		*hits = h;
	if (misses)
		*misses = m;
}

bool thread_profile_is_enabled(void)
{
	return atomic_load_explicit(&g_profiling_enabled, memory_order_relaxed);
}

uint64_t thread_get_cycles(void)
{
	return get_cycles();
}
