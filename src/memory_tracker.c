/* memory_tracker.c */

#include "memory_tracker.h"
#include "logger.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/* Structure to keep track of allocations */
typedef struct Allocation {
    void *ptr;
    size_t size;
    const char *file;
    int line;
    struct Allocation *next;
} Allocation;

/* Head of the allocations linked list */
static Allocation *allocations_head = NULL;

/* Current and peak memory usage */
static size_t current_memory = 0;
static size_t peak_memory = 0;

/* Mutex for thread safety */
static pthread_mutex_t mem_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Initialize the memory tracker */
int InitMemoryTracker(void) {
    pthread_mutex_lock(&mem_mutex);
    allocations_head = NULL;
    current_memory = 0;
    peak_memory = 0;
    pthread_mutex_unlock(&mem_mutex);
    LOG_INFO("Memory Tracker initialized.");
    return 1;
}

/* Shutdown the memory tracker and report leaks */
void ShutdownMemoryTracker(void) {
    pthread_mutex_lock(&mem_mutex);
    
    Allocation *current = allocations_head;
    int leak_count = 0;
    
    while (current) {
        LOG_ERROR("Memory Leak: %zu bytes allocated at %s:%d not freed.", current->size, current->file, current->line);
        leak_count++;
        current = current->next;
    }
    
    if (leak_count == 0) {
        LOG_INFO("No memory leaks detected.");
    } else {
        LOG_WARN("Total memory leaks detected: %d", leak_count);
    }
    
    /* Clean up the allocations list */
    current = allocations_head;
    while (current) {
        Allocation *temp = current;
        current = current->next;
        free(temp);
    }
    allocations_head = NULL;
    current_memory = 0;
    peak_memory = 0;
    
    pthread_mutex_unlock(&mem_mutex);
    pthread_mutex_destroy(&mem_mutex);
    LOG_INFO("Memory Tracker shutdown.");
}

/* Helper function to add an allocation to the tracker */
static void AddAllocation(void *ptr, size_t size, const char *file, int line) {
    Allocation *alloc = (Allocation*)malloc(sizeof(Allocation));
    if (!alloc) {
        LOG_FATAL("Failed to allocate memory for Allocation tracking.");
        exit(EXIT_FAILURE);
    }
    alloc->ptr = ptr;
    alloc->size = size;
    alloc->file = file;
    alloc->line = line;
    alloc->next = allocations_head;
    allocations_head = alloc;
    
    current_memory += size;
    if (current_memory > peak_memory) {
        peak_memory = current_memory;
    }
    
    LOG_DEBUG("Allocated %zu bytes at %p (%s:%d). Current memory: %zu bytes.", size, ptr, file, line, current_memory);
}

/* Helper function to remove an allocation from the tracker */
static void RemoveAllocation(void *ptr, const char *file, int line) {
    Allocation *current = allocations_head;
    Allocation *prev = NULL;
    
    while (current) {
        if (current->ptr == ptr) {
            if (prev) {
                prev->next = current->next;
            } else {
                allocations_head = current->next;
            }
            current_memory -= current->size;
            LOG_DEBUG("Freed %zu bytes from %p (%s:%d). Current memory: %zu bytes.", current->size, ptr, file, line, current_memory);
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
    
    LOG_WARN("Attempted to free untracked memory at %p (%s:%d).", ptr, file, line);
}

/* Override malloc */
void* MT_Malloc(size_t size, const char *file, int line) {
    void *ptr = malloc(size);
    if (ptr) {
        AddAllocation(ptr, size, file, line);
    } else {
        LOG_ERROR("malloc failed at %s:%d for size %zu bytes.", file, line, size);
    }
    return ptr;
}

/* Override calloc */
void* MT_Calloc(size_t num, size_t size, const char *file, int line) {
    void *ptr = calloc(num, size);
    if (ptr) {
        AddAllocation(ptr, num * size, file, line);
    } else {
        LOG_ERROR("calloc failed at %s:%d for %zu elements of size %zu bytes.", file, line, num, size);
    }
    return ptr;
}

/* Override realloc */
void* MT_Realloc(void *ptr, size_t size, const char *file, int line) {
    if (ptr) {
        RemoveAllocation(ptr, file, line);
    }
    void *new_ptr = realloc(ptr, size);
    if (new_ptr) {
        AddAllocation(new_ptr, size, file, line);
    } else {
        LOG_ERROR("realloc failed at %s:%d for size %zu bytes.", file, line, size);
    }
    return new_ptr;
}

/* Override free */
void MT_Free(void *ptr, const char *file, int line) {
    if (ptr) {
        RemoveAllocation(ptr, file, line);
        free(ptr);
    } else {
        LOG_WARN("Attempted to free a NULL pointer at %s:%d.", file, line);
    }
}

/* Get current memory usage */
size_t GetCurrentMemoryUsage(void) {
    pthread_mutex_lock(&mem_mutex);
    size_t usage = current_memory;
    pthread_mutex_unlock(&mem_mutex);
    return usage;
}

/* Get peak memory usage */
size_t GetPeakMemoryUsage(void) {
    pthread_mutex_lock(&mem_mutex);
    size_t usage = peak_memory;
    pthread_mutex_unlock(&mem_mutex);
    return usage;
}

/* Print memory usage statistics */
void PrintMemoryUsage(void) {
    pthread_mutex_lock(&mem_mutex);
    LOG_INFO("Current Memory Usage: %zu bytes", current_memory);
    LOG_INFO("Peak Memory Usage: %zu bytes", peak_memory);
    pthread_mutex_unlock(&mem_mutex);
}
