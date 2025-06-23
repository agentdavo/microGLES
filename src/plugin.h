#ifndef PLUGIN_H
#define PLUGIN_H

#include "gl_thread.h" /* stage_tag_t */

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*stage_plugin_fn)(void *job);

void plugin_register(stage_tag_t stage, stage_plugin_fn fn);
void plugin_invoke(stage_tag_t stage, void *job);

#ifdef __cplusplus
}
#endif

#endif /* PLUGIN_H */
