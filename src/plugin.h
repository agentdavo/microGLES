#ifndef PLUGIN_H
#define PLUGIN_H

#include "gl_thread.h" /* stage_tag_t */
#include <GLES/gl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*stage_plugin_fn)(void *job);

const char *plugin_list(void);
void plugin_register(stage_tag_t stage, stage_plugin_fn fn, const char *name);
void plugin_invoke(stage_tag_t stage, void *job);

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
		plugin_register(stage, func, #func);                    \
	}

#ifdef __cplusplus
}
#endif

#endif /* PLUGIN_H */
