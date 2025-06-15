/* gl_state.h */

#ifndef GL_STATE_H
#define GL_STATE_H
/**
 * @file gl_state.h
 * @brief Legacy global state structure (to be removed).
 */

#include "gl_api_fbo.h" /* For FramebufferOES and RenderbufferOES */
#include "matrix_utils.h" /* For mat4 */
#include <GLES/gl.h>
#include <GLES/glext.h>
#include "gl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations to avoid circular dependencies */
struct RenderbufferOES;
struct FramebufferOES;

typedef struct {
	GLfloat ambient[4];
	GLfloat diffuse[4];
	GLfloat specular[4];
	GLfloat position[4];
	GLfloat spot_direction[3];
	GLfloat spot_exponent;
	GLfloat spot_cutoff;
	GLfloat constant_attenuation;
	GLfloat linear_attenuation;
	GLfloat quadratic_attenuation;
} Light;

typedef struct BufferObject {
	GLuint id;
	GLsizeiptr size;
	GLenum usage;
	void *data;
} BufferObject;

typedef struct VertexArrayObject {
	GLuint id;
	GLboolean vertex_array_enabled;
	GLint vertex_array_size;
	GLenum vertex_array_type;
	GLsizei vertex_array_stride;
	const void *vertex_array_pointer;

	GLboolean color_array_enabled;
	GLint color_array_size;
	GLenum color_array_type;
	GLsizei color_array_stride;
	const void *color_array_pointer;

	GLboolean normal_array_enabled;
	GLenum normal_array_type;
	GLsizei normal_array_stride;
	const void *normal_array_pointer;

	GLboolean texcoord_array_enabled;
	GLint texcoord_array_size;
	GLenum texcoord_array_type;
	GLsizei texcoord_array_stride;
	const void *texcoord_array_pointer;

	GLenum point_size_array_type;
	GLsizei point_size_array_stride;
	const void *point_size_array_pointer;

	GLint matrix_index_array_size;
	GLenum matrix_index_array_type;
	GLsizei matrix_index_array_stride;
	const void *matrix_index_array_pointer;

	GLint weight_array_size;
	GLenum weight_array_type;
	GLsizei weight_array_stride;
	const void *weight_array_pointer;
} VertexArrayObject;

typedef struct {
	GLfloat ambient[4];
	GLfloat diffuse[4];
	GLfloat specular[4];
	GLfloat emission[4];
	GLfloat shininess;
} Material;

/* Maximum number of renderbuffers, framebuffers, textures, and buffers */
#define MAX_RENDERBUFFERS 1024
#define MAX_FRAMEBUFFERS 1024
#define MAX_TEXTURES 1024
#define MAX_BUFFERS 1024
#define MAX_VERTEX_ARRAYS 256

/* GLState structure to manage OpenGL ES state */
typedef struct {
	GLuint next_renderbuffer_id; // Next available renderbuffer ID
	struct RenderbufferOES
		*renderbuffers[MAX_RENDERBUFFERS]; // Array of renderbuffers
	GLint renderbuffer_count; // Number of active renderbuffers
	struct RenderbufferOES
		*bound_renderbuffer; // Currently bound renderbuffer

	GLuint next_framebuffer_id; // Next available framebuffer ID
	struct FramebufferOES
		*framebuffers[MAX_FRAMEBUFFERS]; // Array of framebuffers
	GLint framebuffer_count; // Number of active framebuffers
	struct FramebufferOES *bound_framebuffer; // Currently bound framebuffer
	struct FramebufferOES default_framebuffer; // Default framebuffer (ID 0)

	/* Texture management moved to RenderContext */

	/* Buffer objects */
	GLuint next_buffer_id; // Next available buffer object ID
	GLuint array_buffer_binding; // Buffer bound to GL_ARRAY_BUFFER
	GLuint element_array_buffer_binding; // Buffer bound to GL_ELEMENT_ARRAY_BUFFER
	struct BufferObject *buffers[MAX_BUFFERS];
	GLint buffer_count;

	/* Vertex array objects */
	GLuint next_vertex_array_id;
	struct VertexArrayObject *vaos[MAX_VERTEX_ARRAYS];
	GLint vao_count;
	struct VertexArrayObject *bound_vao;

	mat4 modelview_matrix;
	mat4 projection_matrix;
	mat4 texture_matrix;
	mat4 modelview_stack[32];
	mat4 projection_stack[32];
	mat4 texture_stack[32];
	GLint modelview_stack_depth;
	GLint projection_stack_depth;
	GLint texture_stack_depth;
	GLenum matrix_mode;

	/* Tracked state for core functions */
	GLfloat clear_color[4];
	GLfloat clear_depth;
	GLfloat depth_range_near;
	GLfloat depth_range_far;
	GLint clear_stencil;
	GLboolean color_mask[4];
	GLboolean depth_mask;
	GLenum alpha_func;
	GLfloat alpha_ref;
	GLenum blend_sfactor;
	GLenum blend_dfactor;
	GLenum blend_sfactor_alpha;
	GLenum blend_dfactor_alpha;
	GLenum blend_equation_rgb;
	GLenum blend_equation_alpha;
	GLenum depth_func;
	GLenum cull_face_mode;
	GLenum front_face;
	/* Client active texture and bindings moved to RenderContext */
	GLint viewport[4];
	/* Additional misc state */
	GLfloat point_size;
	GLfloat line_width;
	GLfloat polygon_offset_factor;
	GLfloat polygon_offset_units;
	GLenum shade_model;
	GLint scissor_box[4];
	GLint pack_alignment;
	GLint unpack_alignment;

	/* Clip plane equations */
	GLfloat clip_planes[6][4];

	/* Global lighting model */
	GLfloat light_model_ambient[4];
	GLboolean light_model_two_side;

	/* Logical operation */
	GLenum logic_op_mode;

	/* Sample coverage state */
	GLfloat sample_coverage_value;
	GLboolean sample_coverage_invert;

	/* Stencil state */
	GLenum stencil_func;
	GLint stencil_ref;
	GLuint stencil_value_mask;
	GLenum stencil_fail;
	GLenum stencil_zfail;
	GLenum stencil_zpass;
	GLuint stencil_writemask;

	/* Point parameters */
	GLfloat point_fade_threshold_size;
	GLfloat point_size_min;
	GLfloat point_size_max;

	/* Texture environment per texture unit */
	GLenum tex_env_combine_rgb[8];
	GLenum tex_env_combine_alpha[8];
	GLenum tex_env_src_rgb[8][3];
	GLenum tex_env_src_alpha[8][3];
	GLenum tex_env_operand_rgb[8][3];
	GLenum tex_env_operand_alpha[8][3];
	GLfloat tex_env_rgb_scale[8];
	GLfloat tex_env_alpha_scale[8];
	GLboolean tex_env_coord_replace[8];

	/* TexGen state */
	GLenum tex_gen_mode[4];
	GLfloat tex_gen_plane[4][4];

	/* Material parameters for front and back faces */
	Material material[2];

	/* Capability flags moved to RenderContext */
	Light lights[8];

	/* Fog parameters */
	GLfloat fog_color[4];
	GLfloat fog_density;
	GLfloat fog_start;
	GLfloat fog_end;
	GLenum fog_mode;
	/* Hint values */
	GLenum fog_hint;
	GLenum generate_mipmap_hint;
	GLenum line_smooth_hint;
	GLenum perspective_correction_hint;
	GLenum point_smooth_hint;

	/* Vertex array state removed in favor of RenderContext */

	/* Point size array state (OES_point_size_array) */
	GLenum point_size_array_type;
	GLsizei point_size_array_stride;
	const void *point_size_array_pointer;

	/* Matrix palette state (OES_matrix_palette) */
	GLint matrix_index_array_size;
	GLenum matrix_index_array_type;
	GLsizei matrix_index_array_stride;
	const void *matrix_index_array_pointer;
	GLint weight_array_size;
	GLenum weight_array_type;
	GLsizei weight_array_stride;
	const void *weight_array_pointer;
	GLuint current_palette_matrix;
	mat4 palette_matrices[32];
} GLState;

/* Global GLState instance */
extern GLState gl_state;

/* Function prototypes */
void InitGLState(GLState *state);
void CleanupGLState(GLState *state);

#ifdef __cplusplus
}
#endif

#endif /* GL_STATE_H */
