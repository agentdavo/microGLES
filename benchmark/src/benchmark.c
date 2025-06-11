#include "benchmark.h"
#include "gl_memory_tracker.h"

void compute_result(clock_t start, clock_t end, BenchmarkResult *result)
{
	double secs = (double)(end - start) / CLOCKS_PER_SEC;
	result->fps = 100.0 / secs;
	result->cpu_time_ms = (secs / 100.0) * 1000.0;
	result->current_memory = memory_tracker_current();
	result->peak_memory = memory_tracker_peak();
}
