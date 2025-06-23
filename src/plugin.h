#ifndef PLUGIN_H
#define PLUGIN_H

#include "gl_thread.h" /* stage_tag_t */
#include <GLES/gl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*stage_plugin_fn)(void *job);

void plugin_register(stage_tag_t stage, stage_plugin_fn fn);
void plugin_invoke(stage_tag_t stage, void *job);

/**
 * Submit a task to the internal thread pool from a plugin.
 *
 * Stage plugins may call this helper to spawn additional work items.
 * It simply forwards the request to `thread_pool_submit`.
 */
void plugin_submit(task_function_t fn, void *data, stage_tag_t stage);

typedef int (*texture_decoder_fn)(const char *file, GLuint *tex);
void texture_decoder_register(texture_decoder_fn fn);
GLuint texture_decode(const char *file);

extern int ktx_decoder_link;

/**
 * Convenience macro to auto-register a plugin at load time.
 *
 * Example:
 * @code
 * static void my_plugin(void *job) { do_work(job); }
 * PLUGIN_REGISTER(STAGE_VERTEX, my_plugin);
 * @endcode
 */
#define PLUGIN_REGISTER(stage, func)                                    \
	static void __attribute__((constructor)) _register_##func(void) \
	{                                                               \
		plugin_register(stage, func);                           \
	}

#ifdef __cplusplus
}
#endif

#endif /* PLUGIN_H */
