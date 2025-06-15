#ifndef KTX_LOADER_H
#define KTX_LOADER_H
/**
 * @file ktx_loader.h
 * @brief Loader for KTX texture files.
 */

#include <GLES/gl.h>
#include <GLES/glext.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	KTX_SUCCESS,
	KTX_FILE_NOT_FOUND,
	KTX_INVALID_HEADER,
	KTX_UNSUPPORTED_TEXTURE_TYPE,
	KTX_INVALID_METADATA,
	KTX_INVALID_DATA,
	KTX_OPENGL_ERROR
} KTXError;

KTXError load_ktx_texture(const char *filename, GLuint *texture_id);

#ifdef __cplusplus
}
#endif

#endif /* KTX_LOADER_H */
