#ifndef FUNCTION_PROFILE_H
#define FUNCTION_PROFILE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	const char *name;
	uint64_t call_count;
	uint64_t total_cycles;
} function_profile_t;

void function_profile_reset(void);
void function_profile_record(const char *name, uint64_t cycles);
void function_profile_report(void);

#define PROFILE_START(name)                              \
	bool __pf_enabled = thread_profile_is_enabled(); \
	uint64_t __pf_start = __pf_enabled ? thread_get_cycles() : 0;

#define PROFILE_END(name)                                                   \
	do {                                                                \
		if (__pf_enabled)                                           \
			function_profile_record(name, thread_get_cycles() - \
							      __pf_start);  \
	} while (0)

#ifdef __cplusplus
}
#endif

#endif /* FUNCTION_PROFILE_H */
