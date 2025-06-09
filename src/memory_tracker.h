/* memory_tracker.h */

#ifndef MEMORY_TRACKER_H
#define MEMORY_TRACKER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* Initialize the memory tracker
 * Returns 1 on success, 0 on failure.
 */
int InitMemoryTracker(void);

/* Shutdown the memory tracker and report leaks if any */
void ShutdownMemoryTracker(void);

/* Override malloc, calloc, realloc, and free with tracking */
void* MT_Malloc(size_t size, const char *file, int line);
void* MT_Calloc(size_t num, size_t size, const char *file, int line);
void* MT_Realloc(void *ptr, size_t size, const char *file, int line);
void  MT_Free(void *ptr, const char *file, int line);

/* Macros to automatically capture file and line number */
#define malloc(size) MT_Malloc(size, __FILE__, __LINE__)
#define calloc(num, size) MT_Calloc(num, size, __FILE__, __LINE__)
#define realloc(ptr, size) MT_Realloc(ptr, size, __FILE__, __LINE__)
#define free(ptr) MT_Free(ptr, __FILE__, __LINE__)

/* Function to get current memory usage */
size_t GetCurrentMemoryUsage(void);

/* Function to get peak memory usage */
size_t GetPeakMemoryUsage(void);

/* Function to print memory usage statistics */
void PrintMemoryUsage(void);

#ifdef __cplusplus
}
#endif

#endif /* MEMORY_TRACKER_H */
