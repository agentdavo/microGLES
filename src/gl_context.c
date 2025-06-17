#include "gl_context.h"
#include "gl_logger.h"
#include "gl_memory_tracker.h"
#include "gl_thread.h"
#include "gl_state.h"
#include <string.h>

static RenderContext g_render_context;
static RenderContext *g_current_context = &g_render_context;
static _Thread_local GLenum thread_error = GL_NO_ERROR;

static void init_defaults(RenderContext *ctx)
{
	mat4_identity(&ctx->modelview_matrix);
	mat4_identity(&ctx->projection_matrix);
	mat4_identity(&ctx->texture_matrix);
	ctx->modelview_stack_depth = 1;
	ctx->projection_stack_depth = 1;
	ctx->texture_stack_depth = 1;
	ctx->current_color[0] = 1.0f;
	ctx->current_color[1] = 1.0f;
	ctx->current_color[2] = 1.0f;
	ctx->current_color[3] = 1.0f;
	ctx->clear_color[0] = 0.0f;
	ctx->clear_color[1] = 0.0f;
	ctx->clear_color[2] = 0.0f;
	ctx->clear_color[3] = 1.0f;
	ctx->depth_test_enabled = GL_TRUE;
	ctx->depth_func = GL_LESS;
	atomic_init(&ctx->version_depth, 0);
	ctx->current_normal[0] = 0.0f;
	ctx->current_normal[1] = 0.0f;
	ctx->current_normal[2] = 1.0f;
	ctx->stencil_test_enabled = GL_FALSE;
	ctx->clear_stencil = 0;
	ctx->stencil.func = GL_ALWAYS;
	ctx->stencil.ref = 0;
	ctx->stencil.mask = 0xFFFFFFFFu;
	ctx->stencil.sfail = GL_KEEP;
	ctx->stencil.zfail = GL_KEEP;
	ctx->stencil.zpass = GL_KEEP;
	ctx->stencil.writemask = 0xFFFFFFFFu;
	atomic_init(&ctx->stencil.version, 0);
	atomic_init(&ctx->version_modelview, 0);
	atomic_init(&ctx->version_projection, 0);
	atomic_init(&ctx->version_texture, 0);
	for (int i = 0; i < 2; ++i) {
		ctx->texture_env[i].env_mode = GL_MODULATE;
		ctx->texture_env[i].env_color[0] = 0.f;
		ctx->texture_env[i].env_color[1] = 0.f;
		ctx->texture_env[i].env_color[2] = 0.f;
		ctx->texture_env[i].env_color[3] = 0.f;
		ctx->texture_env[i].bound_texture = 0;
		ctx->texture_env[i].wrap_s = GL_REPEAT;
		ctx->texture_env[i].wrap_t = GL_REPEAT;
		ctx->texture_env[i].min_filter = GL_NEAREST;
		ctx->texture_env[i].mag_filter = GL_NEAREST;
		atomic_init(&ctx->texture_env[i].version, 0);
	}
	for (int u = 0; u < 8; ++u) {
		ctx->current_texcoord[u][0] = 0.0f;
		ctx->current_texcoord[u][1] = 0.0f;
		ctx->current_texcoord[u][2] = 0.0f;
		ctx->current_texcoord[u][3] = 1.0f;
	}
	ctx->blend.src_factor = GL_ONE;
	ctx->blend.dst_factor = GL_ZERO;
	atomic_init(&ctx->blend.version, 0);
	ctx->blend_enabled = GL_FALSE;

	ctx->alpha_test.enabled = GL_FALSE;
	ctx->alpha_test.func = GL_ALWAYS;
	ctx->alpha_test.ref = 0.0f;
	atomic_init(&ctx->alpha_test.version, 0);

	for (int i = 0; i < 8; ++i) {
		ctx->lights[i].ambient[0] = 0.2f;
		ctx->lights[i].ambient[1] = 0.2f;
		ctx->lights[i].ambient[2] = 0.2f;
		ctx->lights[i].ambient[3] = 1.0f;
		ctx->lights[i].diffuse[0] = 1.0f;
		ctx->lights[i].diffuse[1] = 1.0f;
		ctx->lights[i].diffuse[2] = 1.0f;
		ctx->lights[i].diffuse[3] = 1.0f;
		ctx->lights[i].specular[0] = 1.0f;
		ctx->lights[i].specular[1] = 1.0f;
		ctx->lights[i].specular[2] = 1.0f;
		ctx->lights[i].specular[3] = 1.0f;
		ctx->lights[i].position[0] = 0.0f;
		ctx->lights[i].position[1] = 0.0f;
		ctx->lights[i].position[2] = 1.0f;
		ctx->lights[i].position[3] = 0.0f; /* directional */
		ctx->lights[i].spot_direction[0] = 0.0f;
		ctx->lights[i].spot_direction[1] = 0.0f;
		ctx->lights[i].spot_direction[2] = -1.0f;
		ctx->lights[i].spot_exponent = 0.0f;
		ctx->lights[i].spot_cutoff = 180.0f;
		ctx->lights[i].constant_attenuation = 1.0f;
		ctx->lights[i].linear_attenuation = 0.0f;
		ctx->lights[i].quadratic_attenuation = 0.0f;
		ctx->lights[i].enabled = GL_FALSE;
		atomic_init(&ctx->lights[i].version, 0);
	}
	ctx->texture_2d_enabled = GL_FALSE;
	atomic_init(&ctx->version_tex_enable, 0);
	ctx->cull_face_enabled = GL_FALSE;
	ctx->cull_face_mode = GL_BACK;
	ctx->front_face = GL_CCW;
	atomic_init(&ctx->version_cull, 0);
	ctx->scissor_test_enabled = GL_FALSE;
	ctx->scissor_box[0] = ctx->scissor_box[1] = 0;
	ctx->scissor_box[2] = ctx->scissor_box[3] = 0;
	atomic_init(&ctx->version_scissor, 0);
	ctx->color_mask[0] = ctx->color_mask[1] = ctx->color_mask[2] =
		ctx->color_mask[3] = GL_TRUE;
	ctx->depth_mask = GL_TRUE;
	atomic_init(&ctx->version_mask, 0);
	memset(&ctx->vertex_array, 0, sizeof(ArrayState));
	atomic_init(&ctx->vertex_array.version, 0);
	memset(&ctx->color_array, 0, sizeof(ArrayState));
	atomic_init(&ctx->color_array.version, 0);
	memset(&ctx->normal_array, 0, sizeof(ArrayState));
	atomic_init(&ctx->normal_array.version, 0);
	memset(&ctx->texcoord_array, 0, sizeof(ArrayState));
	atomic_init(&ctx->texcoord_array.version, 0);
	ctx->point_size_array_enabled = GL_FALSE;
	ctx->point_size_array_type = GL_FLOAT;
	ctx->point_size_array_stride = 0;
	ctx->point_size_array_pointer = NULL;
	memcpy(ctx->material.ambient, ctx->current_color, sizeof(GLfloat) * 4);
	ctx->material.diffuse[0] = 1.0f;
	ctx->material.diffuse[1] = 1.0f;
	ctx->material.diffuse[2] = 1.0f;
	ctx->material.diffuse[3] = 1.0f;
	ctx->material.specular[0] = 0.0f;
	ctx->material.specular[1] = 0.0f;
	ctx->material.specular[2] = 0.0f;
	ctx->material.specular[3] = 1.0f;
	ctx->material.emission[0] = 0.0f;
	ctx->material.emission[1] = 0.0f;
	ctx->material.emission[2] = 0.0f;
	ctx->material.emission[3] = 1.0f;
	ctx->material.shininess = 0.0f;
	atomic_init(&ctx->material.version, 0);

	ctx->fog.enabled = GL_FALSE;
	ctx->fog.mode = GL_LINEAR;
	ctx->fog.density = 1.0f;
	ctx->fog.start = 0.0f;
	ctx->fog.end = 1.0f;
	ctx->fog.color[0] = ctx->clear_color[0];
	ctx->fog.color[1] = ctx->clear_color[1];
	ctx->fog.color[2] = ctx->clear_color[2];
	ctx->fog.color[3] = ctx->clear_color[3];
	atomic_init(&ctx->fog.version, 0);

	ctx->color_logic_op_enabled = GL_FALSE;
	atomic_init(&ctx->version_color_logic_op, 0);
	ctx->color_material_enabled = GL_FALSE;
	atomic_init(&ctx->version_color_material, 0);
	ctx->dither_enabled = GL_TRUE;
	atomic_init(&ctx->version_dither, 0);
	ctx->lighting_enabled = GL_FALSE;
	atomic_init(&ctx->version_lighting, 0);
	ctx->line_smooth_enabled = GL_FALSE;
	atomic_init(&ctx->version_line_smooth, 0);
	ctx->multisample_enabled = GL_TRUE;
	atomic_init(&ctx->version_multisample, 0);
	ctx->normalize_enabled = GL_FALSE;
	atomic_init(&ctx->version_normalize, 0);
	ctx->point_smooth_enabled = GL_FALSE;
	atomic_init(&ctx->version_point_smooth, 0);
	ctx->point_sprite_enabled = GL_FALSE;
	atomic_init(&ctx->version_point_sprite, 0);
	ctx->polygon_offset_fill_enabled = GL_FALSE;
	atomic_init(&ctx->version_polygon_offset_fill, 0);
	ctx->rescale_normal_enabled = GL_FALSE;
	atomic_init(&ctx->version_rescale_normal, 0);
	ctx->sample_alpha_to_coverage_enabled = GL_FALSE;
	atomic_init(&ctx->version_sample_alpha_to_coverage, 0);
	ctx->sample_alpha_to_one_enabled = GL_FALSE;
	atomic_init(&ctx->version_sample_alpha_to_one, 0);
	ctx->sample_coverage_enabled = GL_FALSE;
	atomic_init(&ctx->version_sample_coverage, 0);
	for (int i = 0; i < 6; ++i)
		ctx->clip_plane_enabled[i] = GL_FALSE;
	atomic_init(&ctx->version_clip_plane, 0);

	ctx->validated_blend_version = 0;
	ctx->validated_depth_version = 0;
	ctx->validated_fog_version = 0;
	ctx->validated_cull_version = 0;

	ctx->texture_count = 0;
	ctx->next_texture_id = 1;
	ctx->active_texture = GL_TEXTURE0;
	ctx->client_active_texture = GL_TEXTURE0;
	ctx->bound_texture_external = 0;
	for (int i = 0; i < MAX_TEXTURES; ++i)
		ctx->textures[i] = NULL;
}

void context_init(void)
{
	init_defaults(&g_render_context);
	thread_error = GL_NO_ERROR;
	LOG_INFO("Render context initialized");
}

RenderContext *context_get(void)
{
	return &g_render_context;
}

RenderContext *GetCurrentContext(void)
{
	return g_current_context;
}

void context_update_modelview_matrix(const mat4 *mat)
{
	mat4_copy(&g_render_context.modelview_matrix, mat);
	atomic_fetch_add_explicit(&g_render_context.version_modelview, 1,
				  memory_order_relaxed);
	log_state_change("modelview updated");
}

void context_update_projection_matrix(const mat4 *mat)
{
	mat4_copy(&g_render_context.projection_matrix, mat);
	atomic_fetch_add_explicit(&g_render_context.version_projection, 1,
				  memory_order_relaxed);
	log_state_change("projection updated");
}

void context_update_texture_matrix(const mat4 *mat)
{
	mat4_copy(&g_render_context.texture_matrix, mat);
	atomic_fetch_add_explicit(&g_render_context.version_texture, 1,
				  memory_order_relaxed);
	log_state_change("texture matrix updated");
}

void context_set_clear_color(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	g_render_context.clear_color[0] = r;
	g_render_context.clear_color[1] = g;
	g_render_context.clear_color[2] = b;
	g_render_context.clear_color[3] = a;
	log_state_change("clear color");
}

void context_set_texture_env(GLenum unit, GLenum pname, const GLfloat *params)
{
	if (unit >= 2)
		return;
	TextureState *ts = &g_render_context.texture_env[unit];
	if (pname == GL_TEXTURE_ENV_MODE) {
		ts->env_mode = (GLenum)params[0];
	} else if (pname == GL_TEXTURE_ENV_COLOR) {
		memcpy(ts->env_color, params, sizeof(GLfloat) * 4);
	} else {
		return;
	}
	atomic_fetch_add_explicit(&ts->version, 1, memory_order_relaxed);
	log_state_change("texture env updated");
}

void context_bind_texture(GLenum unit, GLenum target, GLuint texture)
{
	if (unit >= 2)
		return;
	TextureState *ts = &g_render_context.texture_env[unit];
	ts->bound_texture = texture;
	if (target == GL_TEXTURE_EXTERNAL_OES)
		g_render_context.bound_texture_external = texture;
	atomic_fetch_add_explicit(&ts->version, 1, memory_order_relaxed);
	LOG_DEBUG("bind texture unit %u target 0x%X id %u", unit, target,
		  texture);
}

void context_active_texture(GLenum unit)
{
	if (unit < GL_TEXTURE0 || unit > GL_TEXTURE1)
		return;
	g_render_context.active_texture = unit;
}

void context_set_blend_func(GLenum sfactor, GLenum dfactor)
{
	g_render_context.blend.src_factor = sfactor;
	g_render_context.blend.dst_factor = dfactor;
	atomic_fetch_add_explicit(&g_render_context.blend.version, 1,
				  memory_order_relaxed);
	log_state_change("blend func");
}

void context_set_alpha_func(GLenum func, GLfloat ref)
{
	g_render_context.alpha_test.func = func;
	g_render_context.alpha_test.ref = ref;
	atomic_fetch_add_explicit(&g_render_context.alpha_test.version, 1,
				  memory_order_relaxed);
	log_state_change("alpha func");
}

void context_set_depth_func(GLenum func)
{
	g_render_context.depth_func = func;
	atomic_fetch_add_explicit(&g_render_context.version_depth, 1,
				  memory_order_relaxed);
	log_state_change("depth func");
}

void context_set_light(GLenum light, GLenum pname, const GLfloat *params)
{
	if (light < GL_LIGHT0 || light > GL_LIGHT7)
		return;
	LightState *ls = &g_render_context.lights[light - GL_LIGHT0];
	switch (pname) {
	case GL_AMBIENT:
		memcpy(ls->ambient, params, sizeof(GLfloat) * 4);
		break;
	case GL_DIFFUSE:
		memcpy(ls->diffuse, params, sizeof(GLfloat) * 4);
		break;
	case GL_SPECULAR:
		memcpy(ls->specular, params, sizeof(GLfloat) * 4);
		break;
	case GL_POSITION:
		memcpy(ls->position, params, sizeof(GLfloat) * 4);
		break;
	case GL_SPOT_DIRECTION:
		memcpy(ls->spot_direction, params, sizeof(GLfloat) * 3);
		break;
	case GL_SPOT_EXPONENT:
		ls->spot_exponent = params[0];
		break;
	case GL_SPOT_CUTOFF:
		ls->spot_cutoff = params[0];
		break;
	case GL_CONSTANT_ATTENUATION:
		ls->constant_attenuation = params[0];
		break;
	case GL_LINEAR_ATTENUATION:
		ls->linear_attenuation = params[0];
		break;
	case GL_QUADRATIC_ATTENUATION:
		ls->quadratic_attenuation = params[0];
		break;
	case GL_LIGHT_MODEL_TWO_SIDE:
		/* ignore for now */
		break;
	default:
		return;
	}
	ls->enabled = GL_TRUE;
	atomic_fetch_add_explicit(&ls->version, 1, memory_order_relaxed);
}

void context_set_material(GLenum pname, const GLfloat *params)
{
	MaterialState *ms = &g_render_context.material;
	switch (pname) {
	case GL_AMBIENT:
		memcpy(ms->ambient, params, sizeof(GLfloat) * 4);
		break;
	case GL_DIFFUSE:
		memcpy(ms->diffuse, params, sizeof(GLfloat) * 4);
		break;
	case GL_SPECULAR:
		memcpy(ms->specular, params, sizeof(GLfloat) * 4);
		break;
	case GL_EMISSION:
		memcpy(ms->emission, params, sizeof(GLfloat) * 4);
		break;
	case GL_SHININESS:
		ms->shininess = params[0];
		break;
	default:
		return;
	}
	atomic_fetch_add_explicit(&ms->version, 1, memory_order_relaxed);
}

void context_set_fog(GLenum pname, const GLfloat *params)
{
	FogState *fs = &g_render_context.fog;
	switch (pname) {
	case GL_FOG_MODE:
		fs->mode = (GLenum)params[0];
		break;
	case GL_FOG_DENSITY:
		fs->density = params[0];
		break;
	case GL_FOG_START:
		fs->start = params[0];
		break;
	case GL_FOG_END:
		fs->end = params[0];
		break;
	case GL_FOG_COLOR:
		memcpy(fs->color, params, sizeof(GLfloat) * 4);
		break;
	default:
		return;
	}
	fs->enabled = GL_TRUE;
	atomic_fetch_add_explicit(&fs->version, 1, memory_order_relaxed);
	log_state_change("fog updated");
}

void context_set_stencil_func(GLenum func, GLint ref, GLuint mask)
{
	StencilState *s = &g_render_context.stencil;
	s->func = func;
	s->ref = ref;
	s->mask = mask;
	atomic_fetch_add_explicit(&s->version, 1, memory_order_relaxed);
	log_state_change("stencil func");
}

void context_set_stencil_op(GLenum sfail, GLenum zfail, GLenum zpass)
{
	StencilState *s = &g_render_context.stencil;
	s->sfail = sfail;
	s->zfail = zfail;
	s->zpass = zpass;
	atomic_fetch_add_explicit(&s->version, 1, memory_order_relaxed);
	log_state_change("stencil op");
}

void context_set_stencil_mask(GLuint mask)
{
	StencilState *s = &g_render_context.stencil;
	s->writemask = mask;
	atomic_fetch_add_explicit(&s->version, 1, memory_order_relaxed);
	log_state_change("stencil mask");
}

void context_set_clear_stencil(GLint s)
{
	g_render_context.clear_stencil = s;
	log_state_change("clear stencil");
}

void context_set_error(GLenum error)
{
	if (thread_error == GL_NO_ERROR) {
		thread_error = error;
		LOG_DEBUG("context_set_error: Set error to 0x%X", error);
	} else {
		LOG_DEBUG(
			"context_set_error: Error already set to 0x%X, ignoring 0x%X",
			thread_error, error);
	}
}

GLenum context_get_error(void)
{
	GLenum e = thread_error;
	thread_error = GL_NO_ERROR;
	if (e != GL_NO_ERROR)
		LOG_DEBUG("context_get_error: returning 0x%X", e);
	return e;
}

void log_state_change(const char *msg)
{
	LOG_DEBUG("State change: %s", msg);
}

/* Texture management */

static TextureOES *texture_find_internal(GLuint id)
{
	for (GLuint i = 0; i < g_render_context.texture_count; ++i) {
		if (g_render_context.textures[i] &&
		    g_render_context.textures[i]->id == id)
			return g_render_context.textures[i];
	}
	return NULL;
}

TextureOES *context_find_texture(GLuint id)
{
	return texture_find_internal(id);
}

void context_gen_textures(GLsizei n, GLuint *textures)
{
	for (GLsizei i = 0; i < n; ++i) {
		textures[i] = g_render_context.next_texture_id++;
	}
}

void context_delete_textures(GLsizei n, const GLuint *textures)
{
	for (GLsizei i = 0; i < n; ++i) {
		TextureOES *tex = texture_find_internal(textures[i]);
		if (!tex)
			continue;
		for (GLuint j = 0; j < g_render_context.texture_count; ++j) {
			if (g_render_context.textures[j] == tex) {
				for (int l = 0; l < MAX_MIPMAP_LEVELS; ++l)
					if (tex->levels[l])
						MT_FREE(tex->levels[l],
							STAGE_FRAGMENT);
				MT_FREE(tex, STAGE_FRAGMENT);
				for (GLuint k = j + 1;
				     k < g_render_context.texture_count; ++k)
					g_render_context.textures[k - 1] =
						g_render_context.textures[k];
				g_render_context.texture_count--;
				break;
			}
		}
	}
}

void context_tex_image_2d(GLenum target, GLint level, GLint internalformat,
			  GLsizei width, GLsizei height, GLenum format,
			  GLenum type, const void *pixels)
{
	(void)type;
	if (target != GL_TEXTURE_2D && target != GL_TEXTURE_EXTERNAL_OES)
		return;
	TextureOES *tex = texture_find_internal(
		g_render_context
			.texture_env[g_render_context.active_texture -
				     GL_TEXTURE0]
			.bound_texture);
	if (!tex) {
		if (g_render_context.texture_count >= MAX_TEXTURES)
			return;
		tex = MT_ALLOC(sizeof(TextureOES), STAGE_FRAGMENT);
		if (!tex)
			return;
		memset(tex, 0, sizeof(TextureOES));
		tex->id = g_render_context
				  .texture_env[g_render_context.active_texture -
					       GL_TEXTURE0]
				  .bound_texture;
		tex->target = target;
		g_render_context.textures[g_render_context.texture_count++] =
			tex;
	}
	tex->internalformat = internalformat;
	tex->format = format;
	if (level == 0)
		tex->mipmap_supported = true;
	tex->mip_width[level] = width;
	tex->mip_height[level] = height;
	if (level == 0) {
		tex->width = width;
		tex->height = height;
	}
	if (level > tex->current_level)
		tex->current_level = level;
	size_t size = (size_t)width * height * 4;
	if (tex->levels[level])
		MT_FREE(tex->levels[level], STAGE_FRAGMENT);
	tex->levels[level] = MT_ALLOC(size, STAGE_FRAGMENT);
	if (!tex->levels[level])
		return;
	if (pixels) {
		const uint8_t *src = pixels;
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				uint32_t c = 0xFF000000u;
				switch (format) {
				case GL_RGBA:
					c = (uint32_t)src[(y * width + x) * 4 +
							  0] |
					    ((uint32_t)
						     src[(y * width + x) * 4 + 1]
					     << 8) |
					    ((uint32_t)
						     src[(y * width + x) * 4 + 2]
					     << 16) |
					    ((uint32_t)
						     src[(y * width + x) * 4 + 3]
					     << 24);
					break;
				case GL_RGB:
					c = (uint32_t)src[(y * width + x) * 3 +
							  0] |
					    ((uint32_t)
						     src[(y * width + x) * 3 + 1]
					     << 8) |
					    ((uint32_t)
						     src[(y * width + x) * 3 + 2]
					     << 16) |
					    0xFF000000u;
					break;
				case GL_ALPHA:
					c = ((uint32_t)src[y * width + x]
					     << 24);
					break;
				case GL_LUMINANCE: {
					uint8_t l = src[y * width + x];
					c = (uint32_t)l | ((uint32_t)l << 8) |
					    ((uint32_t)l << 16) | 0xFF000000u;
					break;
				}
				case GL_LUMINANCE_ALPHA: {
					uint8_t l =
						src[(y * width + x) * 2 + 0];
					uint8_t a =
						src[(y * width + x) * 2 + 1];
					c = (uint32_t)l | ((uint32_t)l << 8) |
					    ((uint32_t)l << 16) |
					    ((uint32_t)a << 24);
					break;
				}
				default:
					c = 0xFFFFFFFFu;
					break;
				}
				tex->levels[level][y * width + x] = c;
			}
		}
	}
	atomic_fetch_add_explicit(&tex->version, 1, memory_order_relaxed);
}

void context_tex_sub_image_2d(GLenum target, GLint level, GLint xoffset,
			      GLint yoffset, GLsizei width, GLsizei height,
			      GLenum format, GLenum type, const void *pixels)
{
	(void)type;
	if (target != GL_TEXTURE_2D && target != GL_TEXTURE_EXTERNAL_OES)
		return;
	TextureOES *tex = texture_find_internal(
		g_render_context
			.texture_env[g_render_context.active_texture -
				     GL_TEXTURE0]
			.bound_texture);
	if (!tex || !tex->levels[level] || !pixels)
		return;
	const uint8_t *src = pixels;

	if (format == GL_RGBA && type == GL_UNSIGNED_BYTE) {
		int row_bytes = width * 4;
		int align = gl_state.unpack_alignment ?
				    gl_state.unpack_alignment :
				    4;
		int row_padded = (row_bytes + align - 1) & ~(align - 1);

		uint32_t *dst = tex->levels[level] +
				(size_t)yoffset * tex->mip_width[level] +
				xoffset;

		if (row_padded == row_bytes) {
			if (width == tex->mip_width[level] && xoffset == 0) {
				memcpy(dst, src, (size_t)row_bytes * height);
			} else {
				for (int y = 0; y < height; ++y) {
					memcpy(dst + y * tex->mip_width[level],
					       src + y * row_bytes, row_bytes);
				}
			}
			atomic_fetch_add_explicit(&tex->version, 1,
						  memory_order_relaxed);
			return;
		}
	}

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			uint32_t c = 0xFF000000u;
			switch (format) {
			case GL_RGBA:
				c = (uint32_t)src[(y * width + x) * 4 + 0] |
				    ((uint32_t)src[(y * width + x) * 4 + 1]
				     << 8) |
				    ((uint32_t)src[(y * width + x) * 4 + 2]
				     << 16) |
				    ((uint32_t)src[(y * width + x) * 4 + 3]
				     << 24);
				break;
			case GL_RGB:
				c = (uint32_t)src[(y * width + x) * 3 + 0] |
				    ((uint32_t)src[(y * width + x) * 3 + 1]
				     << 8) |
				    ((uint32_t)src[(y * width + x) * 3 + 2]
				     << 16) |
				    0xFF000000u;
				break;
			case GL_ALPHA:
				c = ((uint32_t)src[y * width + x] << 24);
				break;
			case GL_LUMINANCE: {
				uint8_t l = src[y * width + x];
				c = (uint32_t)l | ((uint32_t)l << 8) |
				    ((uint32_t)l << 16) | 0xFF000000u;
				break;
			}
			case GL_LUMINANCE_ALPHA: {
				uint8_t l = src[(y * width + x) * 2 + 0];
				uint8_t a = src[(y * width + x) * 2 + 1];
				c = (uint32_t)l | ((uint32_t)l << 8) |
				    ((uint32_t)l << 16) | ((uint32_t)a << 24);
				break;
			}
			default:
				c = 0xFFFFFFFFu;
				break;
			}
			size_t idx =
				(size_t)(yoffset + y) * tex->mip_width[level] +
				(xoffset + x);
			tex->levels[level][idx] = c;
		}
	}
	atomic_fetch_add_explicit(&tex->version, 1, memory_order_relaxed);
}

void context_tex_parameterf(GLenum target, GLenum pname, GLfloat param)
{
	if (target != GL_TEXTURE_2D && target != GL_TEXTURE_EXTERNAL_OES)
		return;
	TextureOES *tex = texture_find_internal(
		g_render_context
			.texture_env[g_render_context.active_texture -
				     GL_TEXTURE0]
			.bound_texture);
	if (!tex)
		return;
	switch (pname) {
	case GL_TEXTURE_MIN_FILTER:
		tex->min_filter = (GLint)param;
		break;
	case GL_TEXTURE_MAG_FILTER:
		tex->mag_filter = (GLint)param;
		break;
	case GL_TEXTURE_WRAP_S:
		tex->wrap_s = (GLint)param;
		break;
	case GL_TEXTURE_WRAP_T:
		tex->wrap_t = (GLint)param;
		break;
	default:
		return;
	}
	atomic_fetch_add_explicit(&tex->version, 1, memory_order_relaxed);
}
