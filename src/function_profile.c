#include "function_profile.h"
#include "gl_logger.h"
#include "gl_thread.h"
#include <stdlib.h>
#include <string.h>

#define MAX_FUNC_PROFILES 128

static function_profile_t g_profiles[MAX_FUNC_PROFILES];
static int g_profile_count = 0;

void function_profile_reset(void)
{
	g_profile_count = 0;
	memset(g_profiles, 0, sizeof(g_profiles));
}

void function_profile_record(const char *name, uint64_t cycles)
{
	for (int i = 0; i < g_profile_count; ++i) {
		if (g_profiles[i].name == name ||
		    (g_profiles[i].name && name &&
		     strcmp(g_profiles[i].name, name) == 0)) {
			g_profiles[i].call_count++;
			g_profiles[i].total_cycles += cycles;
			return;
		}
	}
	if (g_profile_count < MAX_FUNC_PROFILES) {
		g_profiles[g_profile_count].name = name;
		g_profiles[g_profile_count].call_count = 1;
		g_profiles[g_profile_count].total_cycles = cycles;
		g_profile_count++;
	}
}

static int compare_profile(const void *a, const void *b)
{
	const function_profile_t *pa = a;
	const function_profile_t *pb = b;
	if (pb->total_cycles > pa->total_cycles)
		return 1;
	if (pb->total_cycles < pa->total_cycles)
		return -1;
	return 0;
}

void function_profile_report(void)
{
	if (g_profile_count == 0)
		return;
	qsort(g_profiles, g_profile_count, sizeof(function_profile_t),
	      compare_profile);
	LOG_INFO("GL Function Profiling Results:");
	for (int i = 0; i < g_profile_count; ++i) {
		LOG_INFO("  %s: calls=%llu time=%lluus",
			 g_profiles[i].name ? g_profiles[i].name : "(null)",
			 (unsigned long long)g_profiles[i].call_count,
			 (unsigned long long)thread_cycles_to_us(
				 g_profiles[i].total_cycles));
	}
}
