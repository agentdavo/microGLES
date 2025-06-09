```c
// functions.c
#include "gl_types.h"
#include "gl_errors.h"
#include "matrix_utils.h"
#include "logger.h"

// Assume GLState is a global or accessible structure
extern GLState gl_state;

// Implementation of glAlphaFunc
GL_API void GL_APIENTRY glAlphaFunc (GLenum func, GLfloat ref) {
    glAlphaFunc(func, ref);
    gl_state.alpha_func = func;
    gl_state.alpha_ref = ref;
    LOG_DEBUG("Set AlphaFunc: func=0x%X, ref=%f.", func, ref);
}

// Implementation of glClearColor
GL_API void GL_APIENTRY glClearColor (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    glClearColor(red, green, blue, alpha);
    gl_state.clear_color[0] = red;
    gl_state.clear_color[1] = green;
    gl_state.clear_color[2] = blue;
    gl_state.clear_color[3] = alpha;
    LOG_DEBUG("Set ClearColor: (%f, %f, %f, %f).", red, green, blue, alpha);
}

// Implementation of glClearDepthf
GL_API void GL_APIENTRY glClearDepthf (GLfloat d) {
    glClearDepthf(d);
    gl_state.clear_depth = d;
    LOG_DEBUG("Set ClearDepthf: %f.", d);
}

// Implementation of glClipPlanef
GL_API void GL_APIENTRY glClipPlanef (GLenum p, const GLfloat *eqn) {
    glClipPlanef(p, eqn);
    memcpy(gl_state.clip_planes[p - GL_CLIP_PLANE0], eqn, sizeof(GLfloat) * 4);
    LOG_DEBUG("Set ClipPlanef: plane=0x%X, eqn=(%f, %f, %f, %f).", p, eqn[0], eqn[1], eqn[2], eqn[3]);
}

// Implementation of glColor4f
GL_API void GL_APIENTRY glColor4f (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    glColor4f(red, green, blue, alpha);
    gl_state.current_color[0] = red;
    gl_state.current_color[1] = green;
    gl_state.current_color[2] = blue;
    gl_state.current_color[3] = alpha;
    LOG_DEBUG("Set Color4f: (%f, %f, %f, %f).", red, green, blue, alpha);
}

// Implementation of glDepthRangef
GL_API void GL_APIENTRY glDepthRangef (GLfloat n, GLfloat f) {
    glDepthRangef(n, f);
    gl_state.depth_range_near = n;
    gl_state.depth_range_far = f;
    LOG_DEBUG("Set DepthRangef: near=%f, far=%f.", n, f);
}

// Implementation of glFogf
GL_API void GL_APIENTRY glFogf (GLenum pname, GLfloat param) {
    glFogf(pname, param);
    switch(pname) {
        case GL_FOG_DENSITY:
            gl_state.fog_density = param;
            break;
        case GL_FOG_START:
            gl_state.fog_start = param;
            break;
        case GL_FOG_END:
            gl_state.fog_end = param;
            break;
        case GL_FOG_MODE:
            gl_state.fog_mode = param;
            break;
        default:
            break;
    }
    LOG_DEBUG("Set Fogf: pname=0x%X, param=%f.", pname, param);
}

// Implementation of glFogfv
GL_API void GL_APIENTRY glFogfv (GLenum pname, const GLfloat *params) {
    glFogfv(pname, params);
    switch(pname) {
        case GL_FOG_COLOR:
            memcpy(gl_state.fog_color, params, sizeof(GLfloat) * 4);
            break;
        default:
            break;
    }
    LOG_DEBUG("Set Fogfv: pname=0x%X.", pname);
}

// Implementation of glFrustumf
GL_API void GL_APIENTRY glFrustumf (GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f) {
    glFrustumf(l, r, b, t, n, f);
    mat4 frustum_matrix;
    generateFrustumMatrix4x4(&frustum_matrix, l, r, b, t, n, f);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(frustum_matrix.data);
    LOG_DEBUG("Set Frustumf: l=%f, r=%f, b=%f, t=%f, n=%f, f=%f.", l, r, b, t, n, f);
}

// Implementation of glGetClipPlanef
GL_API void GL_APIENTRY glGetClipPlanef (GLenum plane, GLfloat *equation) {
    glGetClipPlanef(plane, equation);
    memcpy(equation, gl_state.clip_planes[plane - GL_CLIP_PLANE0], sizeof(GLfloat) * 4);
    LOG_DEBUG("Get ClipPlanef: plane=0x%X.", plane);
}

// Implementation of glGetFloatv
GL_API void GL_APIENTRY glGetFloatv (GLenum pname, GLfloat *data) {
    glGetFloatv(pname, data);
    switch(pname) {
        case GL_MODELVIEW_MATRIX:
            memcpy(data, gl_state.modelview_matrix.data, sizeof(mat4));
            break;
        case GL_PROJECTION_MATRIX:
            memcpy(data, gl_state.projection_matrix.data, sizeof(mat4));
            break;
        case GL_TEXTURE_MATRIX:
            memcpy(data, gl_state.texture_matrix.data, sizeof(mat4));
            break;
        case GL_CURRENT_COLOR:
            memcpy(data, gl_state.current_color, sizeof(GLfloat) * 4);
            break;
        default:
            break;
    }
    LOG_DEBUG("Get Floatv: pname=0x%X.", pname);
}

// Implementation of glGetLightfv
GL_API void GL_APIENTRY glGetLightfv (GLenum light, GLenum pname, GLfloat *params) {
    glGetLightfv(light, pname, params);
    // Example: Store retrieved light parameters if needed
    LOG_DEBUG("Get Lightfv: light=0x%X, pname=0x%X.", light, pname);
}

// Implementation of glGetMaterialfv
GL_API void GL_APIENTRY glGetMaterialfv (GLenum face, GLenum pname, GLfloat *params) {
    glGetMaterialfv(face, pname, params);
    // Example: Store retrieved material parameters if needed
    LOG_DEBUG("Get Materialfv: face=0x%X, pname=0x%X.", face, pname);
}

// Implementation of glGetTexEnvfv
GL_API void GL_APIENTRY glGetTexEnvfv (GLenum target, GLenum pname, GLfloat *params) {
    glGetTexEnvfv(target, pname, params);
    // Example: Store retrieved texture environment parameters if needed
    LOG_DEBUG("Get TexEnvfv: target=0x%X, pname=0x%X.", target, pname);
}

// Implementation of glGetTexParameterfv
GL_API void GL_APIENTRY glGetTexParameterfv (GLenum target, GLenum pname, GLfloat *params) {
    glGetTexParameterfv(target, pname, params);
    // Example: Store retrieved texture parameters if needed
    LOG_DEBUG("Get TexParameterfv: target=0x%X, pname=0x%X.", target, pname);
}

// Implementation of glLightModelf
GL_API void GL_APIENTRY glLightModelf (GLenum pname, GLfloat param) {
    glLightModelf(pname, param);
    switch(pname) {
        case GL_LIGHT_MODEL_AMBIENT:
            gl_state.light_model_ambient[0] = param;
            gl_state.light_model_ambient[1] = param;
            gl_state.light_model_ambient[2] = param;
            gl_state.light_model_ambient[3] = param;
            break;
        case GL_LIGHT_MODEL_TWO_SIDE:
            gl_state.light_model_two_side = (GLboolean)param;
            break;
        default:
            break;
    }
    LOG_DEBUG("Set LightModelf: pname=0x%X, param=%f.", pname, param);
}

// Implementation of glLightModelfv
GL_API void GL_APIENTRY glLightModelfv (GLenum pname, const GLfloat *params) {
    glLightModelfv(pname, params);
    switch(pname) {
        case GL_LIGHT_MODEL_AMBIENT:
            memcpy(gl_state.light_model_ambient, params, sizeof(GLfloat) * 4);
            break;
        case GL_LIGHT_MODEL_TWO_SIDE:
            gl_state.light_model_two_side = (GLboolean)params[0];
            break;
        default:
            break;
    }
    LOG_DEBUG("Set LightModelfv: pname=0x%X.", pname);
}

// Implementation of glLightf
GL_API void GL_APIENTRY glLightf (GLenum light, GLenum pname, GLfloat param) {
    glLightf(light, pname, param);
    LOG_DEBUG("Set Lightf: light=0x%X, pname=0x%X, param=%f.", light, pname, param);
}

// Implementation of glLightfv
GL_API void GL_APIENTRY glLightfv (GLenum light, GLenum pname, const GLfloat *params) {
    glLightfv(light, pname, params);
    LOG_DEBUG("Set Lightfv: light=0x%X, pname=0x%X.", light, pname);
}

// Implementation of glLineWidth
GL_API void GL_APIENTRY glLineWidth (GLfloat width) {
    glLineWidth(width);
    gl_state.line_width = width;
    LOG_DEBUG("Set LineWidth: %f.", width);
}

// Implementation of glLoadMatrixf
GL_API void GL_APIENTRY glLoadMatrixf (const GLfloat *m) {
    glLoadMatrixf(m);
    if (gl_state.matrix_mode == GL_MODELVIEW) {
        memcpy(gl_state.modelview_matrix.data, m, sizeof(mat4));
    } else if (gl_state.matrix_mode == GL_PROJECTION) {
        memcpy(gl_state.projection_matrix.data, m, sizeof(mat4));
    } else if (gl_state.matrix_mode == GL_TEXTURE) {
        memcpy(gl_state.texture_matrix.data, m, sizeof(mat4));
    }
    LOG_DEBUG("Loaded Matrixf for mode=0x%X.", gl_state.matrix_mode);
}

// Implementation of glMaterialf
GL_API void GL_APIENTRY glMaterialf (GLenum face, GLenum pname, GLfloat param) {
    glMaterialf(face, pname, param);
    LOG_DEBUG("Set Materialf: face=0x%X, pname=0x%X, param=%f.", face, pname, param);
}

// Implementation of glMaterialfv
GL_API void GL_APIENTRY glMaterialfv (GLenum face, GLenum pname, const GLfloat *params) {
    glMaterialfv(face, pname, params);
    LOG_DEBUG("Set Materialfv: face=0x%X, pname=0x%X.", face, pname);
}

// Implementation of glMultMatrixf
GL_API void GL_APIENTRY glMultMatrixf (const GLfloat *m) {
    glMultMatrixf(m);
    mat4 multiplier;
    memcpy(multiplier.data, m, sizeof(mat4));
    if (gl_state.matrix_mode == GL_MODELVIEW) {
        multiplyMatrices(&gl_state.modelview_matrix, &multiplier, &gl_state.modelview_matrix);
    } else if (gl_state.matrix_mode == GL_PROJECTION) {
        multiplyMatrices(&gl_state.projection_matrix, &multiplier, &gl_state.projection_matrix);
    } else if (gl_state.matrix_mode == GL_TEXTURE) {
        multiplyMatrices(&gl_state.texture_matrix, &multiplier, &gl_state.texture_matrix);
    }
    LOG_DEBUG("Multiplied current matrix by provided matrix for mode=0x%X.", gl_state.matrix_mode);
}

// Implementation of glMultiTexCoord4f
GL_API void GL_APIENTRY glMultiTexCoord4f (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q) {
    glMultiTexCoord4f(target, s, t, r, q);
    LOG_DEBUG("Set MultiTexCoord4f: target=0x%X, s=%f, t=%f, r=%f, q=%f.", target, s, t, r, q);
}

// Implementation of glNormal3f
GL_API void GL_APIENTRY glNormal3f (GLfloat nx, GLfloat ny, GLfloat nz) {
    glNormal3f(nx, ny, nz);
    gl_state.current_normal[0] = nx;
    gl_state.current_normal[1] = ny;
    gl_state.current_normal[2] = nz;
    LOG_DEBUG("Set Normal3f: (%f, %f, %f).", nx, ny, nz);
}

// Implementation of glOrthof
GL_API void GL_APIENTRY glOrthof (GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f) {
    glOrthof(l, r, b, t, n, f);
    mat4 ortho_matrix;
    generateOrthographicMatrix4x4(&ortho_matrix, l, r, b, t, n, f);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(ortho_matrix.data);
    LOG_DEBUG("Set Orthof: l=%f, r=%f, b=%f, t=%f, n=%f, f=%f.", l, r, b, t, n, f);
}

// Implementation of glPointParameterf
GL_API void GL_APIENTRY glPointParameterf (GLenum pname, GLfloat param) {
    glPointParameterf(pname, param);
    switch(pname) {
        case GL_POINT_SIZE_MIN:
            gl_state.point_size_min = param;
            break;
        case GL_POINT_SIZE_MAX:
            gl_state.point_size_max = param;
            break;
        case GL_POINT_FADE_THRESHOLD_SIZE:
            gl_state.point_fade_threshold = param;
            break;
        case GL_POINT_DISTANCE_ATTENUATION:
            gl_state.point_distance_attenuation = param;
            break;
        default:
            break;
    }
    LOG_DEBUG("Set PointParameterf: pname=0x%X, param=%f.", pname, param);
}

// Implementation of glPointParameterfv
GL_API void GL_APIENTRY glPointParameterfv (GLenum pname, const GLfloat *params) {
    glPointParameterfv(pname, params);
    switch(pname) {
        case GL_POINT_DISTANCE_ATTENUATION:
            memcpy(gl_state.point_distance_attenuation_params, params, sizeof(GLfloat) * 3);
            break;
        default:
            break;
    }
    LOG_DEBUG("Set PointParameterfv: pname=0x%X.", pname);
}

// Implementation of glPointSize
GL_API void GL_APIENTRY glPointSize (GLfloat size) {
    glPointSize(size);
    gl_state.point_size = size;
    LOG_DEBUG("Set PointSize: %f.", size);
}

// Implementation of glPolygonOffset
GL_API void GL_APIENTRY glPolygonOffset (GLfloat factor, GLfloat units) {
    glPolygonOffset(factor, units);
    gl_state.polygon_offset_factor = factor;
    gl_state.polygon_offset_units = units;
    LOG_DEBUG("Set PolygonOffset: factor=%f, units=%f.", factor, units);
}

// Implementation of glRotatef
GL_API void GL_APIENTRY glRotatef (GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
    glRotatef(angle, x, y, z);
    mat4 rotation;
    generateRotationMatrix(&rotation, angle, x, y, z);
    if (gl_state.matrix_mode == GL_MODELVIEW) {
        multiplyMatrices(&gl_state.modelview_matrix, &rotation, &gl_state.modelview_matrix);
        glLoadMatrixf(gl_state.modelview_matrix.data);
    } else if (gl_state.matrix_mode == GL_PROJECTION) {
        multiplyMatrices(&gl_state.projection_matrix, &rotation, &gl_state.projection_matrix);
        glLoadMatrixf(gl_state.projection_matrix.data);
    } else if (gl_state.matrix_mode == GL_TEXTURE) {
        multiplyMatrices(&gl_state.texture_matrix, &rotation, &gl_state.texture_matrix);
        glLoadMatrixf(gl_state.texture_matrix.data);
    }
    LOG_DEBUG("Applied Rotatef: angle=%f, axis=(%f, %f, %f).", angle, x, y, z);
}

// Implementation of glScalef
GL_API void GL_APIENTRY glScalef (GLfloat x, GLfloat y, GLfloat z) {
    glScalef(x, y, z);
    mat4 scale;
    generateScaleMatrix(&scale, x, y, z);
    if (gl_state.matrix_mode == GL_MODELVIEW) {
        multiplyMatrices(&gl_state.modelview_matrix, &scale, &gl_state.modelview_matrix);
        glLoadMatrixf(gl_state.modelview_matrix.data);
    } else if (gl_state.matrix_mode == GL_PROJECTION) {
        multiplyMatrices(&gl_state.projection_matrix, &scale, &gl_state.projection_matrix);
        glLoadMatrixf(gl_state.projection_matrix.data);
    } else if (gl_state.matrix_mode == GL_TEXTURE) {
        multiplyMatrices(&gl_state.texture_matrix, &scale, &gl_state.texture_matrix);
        glLoadMatrixf(gl_state.texture_matrix.data);
    }
    LOG_DEBUG("Applied Scalef: x=%f, y=%f, z=%f.", x, y, z);
}

// Implementation of glTexEnvf
GL_API void GL_APIENTRY glTexEnvf (GLenum target, GLenum pname, GLfloat param) {
    glTexEnvf(target, pname, param);
    if (target == GL_TEXTURE_ENV) {
        switch(pname) {
            case GL_TEXTURE_ENV_MODE:
                gl_state.tex_env_mode = param;
                break;
            default:
                break;
        }
    }
    LOG_DEBUG("Set TexEnvf: target=0x%X, pname=0x%X, param=%f.", target, pname, param);
}

// Implementation of glTexEnvfv
GL_API void GL_APIENTRY glTexEnvfv (GLenum target, GLenum pname, const GLfloat *params) {
    glTexEnvfv(target, pname, params);
    if (target == GL_TEXTURE_ENV) {
        switch(pname) {
            case GL_TEXTURE_ENV_COLOR:
                memcpy(gl_state.tex_env_color, params, sizeof(GLfloat) * 4);
                break;
            default:
                break;
        }
    }
    LOG_DEBUG("Set TexEnvfv: target=0x%X, pname=0x%X.", target, pname);
}

// Implementation of glTexParameterf
GL_API void GL_APIENTRY glTexParameterf (GLenum target, GLenum pname, GLfloat param) {
    glTexParameterf(target, pname, param);
    Texture *tex = getTextureByID(gl_state.current_texture);
    if (tex && tex->target == target) {
        switch(pname) {
            case GL_TEXTURE_MIN_FILTER:
                tex->min_filter = param;
                break;
            case GL_TEXTURE_MAG_FILTER:
                tex->mag_filter = param;
                break;
            case GL_TEXTURE_WRAP_S:
                tex->wrap_s = param;
                break;
            case GL_TEXTURE_WRAP_T:
                tex->wrap_t = param;
                break;
            default:
                break;
        }
    }
    LOG_DEBUG("Set TexParameterf: target=0x%X, pname=0x%X, param=%f.", target, pname, param);
}

// Implementation of glTexParameterfv
GL_API void GL_APIENTRY glTexParameterfv (GLenum target, GLenum pname, const GLfloat *params) {
    glTexParameterfv(target, pname, params);
    Texture *tex = getTextureByID(gl_state.current_texture);
    if (tex && tex->target == target) {
        switch(pname) {
            case GL_TEXTURE_ENV_COLOR:
                memcpy(tex->env_color, params, sizeof(GLfloat) * 4);
                break;
            default:
                break;
        }
    }
    LOG_DEBUG("Set TexParameterfv: target=0x%X, pname=0x%X.", target, pname);
}

// Implementation of glTranslatef
GL_API void GL_APIENTRY glTranslatef (GLfloat x, GLfloat y, GLfloat z) {
    glTranslatef(x, y, z);
    mat4 translation;
    generateTranslationMatrix(&translation, x, y, z);
    if (gl_state.matrix_mode == GL_MODELVIEW) {
        multiplyMatrices(&gl_state.modelview_matrix, &translation, &gl_state.modelview_matrix);
        glLoadMatrixf(gl_state.modelview_matrix.data);
    } else if (gl_state.matrix_mode == GL_PROJECTION) {
        multiplyMatrices(&gl_state.projection_matrix, &translation, &gl_state.projection_matrix);
        glLoadMatrixf(gl_state.projection_matrix.data);
    } else if (gl_state.matrix_mode == GL_TEXTURE) {
        multiplyMatrices(&gl_state.texture_matrix, &translation, &gl_state.texture_matrix);
        glLoadMatrixf(gl_state.texture_matrix.data);
    }
    LOG_DEBUG("Applied Translatef: x=%f, y=%f, z=%f.", x, y, z);
}

// Implementation of glActiveTexture
GL_API void GL_APIENTRY glActiveTexture (GLenum texture) {
    glActiveTexture(texture);
    gl_state.active_texture = texture - GL_TEXTURE0;
    LOG_DEBUG("Set ActiveTexture: 0x%X.", texture);
}

// Implementation of glAlphaFuncx
GL_API void GL_APIENTRY glAlphaFuncx (GLenum func, GLfixed ref) {
    glAlphaFuncx(func, ref);
    gl_state.alpha_func = func;
    gl_state.alpha_ref_fixed = ref;
    LOG_DEBUG("Set AlphaFuncx: func=0x%X, ref=%d.", func, ref);
}

// Implementation of glBindBuffer
GL_API void GL_APIENTRY glBindBuffer (GLenum target, GLuint buffer) {
    glBindBuffer(target, buffer);
    gl_state.bound_buffer[target] = buffer;
    LOG_DEBUG("Bound Buffer: target=0x%X, buffer=%u.", target, buffer);
}

// Implementation of glBindTexture
GL_API void GL_APIENTRY glBindTexture (GLenum target, GLuint texture) {
    glBindTexture(target, texture);
    gl_state.bound_texture[target] = texture;
    LOG_DEBUG("Bound Texture: target=0x%X, texture=%u.", target, texture);
}

// Implementation of glBlendFunc
GL_API void GL_APIENTRY glBlendFunc (GLenum sfactor, GLenum dfactor) {
    glBlendFunc(sfactor, dfactor);
    gl_state.blend_sfactor = sfactor;
    gl_state.blend_dfactor = dfactor;
    LOG_DEBUG("Set BlendFunc: sfactor=0x%X, dfactor=0x%X.", sfactor, dfactor);
}

// Implementation of glBufferData
GL_API void GL_APIENTRY glBufferData (GLenum target, GLsizeiptr size, const void *data, GLenum usage) {
    glBufferData(target, size, data, usage);
    Buffer *buf = getBufferByID(gl_state.bound_buffer[target]);
    if (buf) {
        if (buf->data) {
            tracked_free(buf->data, buf->size);
        }
        buf->data = tracked_malloc(size);
        if (buf->data) {
            memcpy(buf->data, data, size);
            buf->size = size;
            buf->usage = usage;
            LOG_DEBUG("Set BufferData: target=0x%X, size=%ld, usage=0x%X.", target, size, usage);
        } else {
            LOG_ERROR("Failed to allocate memory for BufferData.");
            glSetError(GL_OUT_OF_MEMORY);
        }
    } else {
        LOG_WARN("No buffer bound to target=0x%X.", target);
        glSetError(GL_INVALID_OPERATION);
    }
}

// Implementation of glBufferSubData
GL_API void GL_APIENTRY glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const void *data) {
    glBufferSubData(target, offset, size, data);
    Buffer *buf = getBufferByID(gl_state.bound_buffer[target]);
    if (buf && buf->data) {
        if (offset + size > buf->size) {
            LOG_WARN("BufferSubData exceeds buffer size.");
            glSetError(GL_INVALID_VALUE);
            return;
        }
        memcpy((char*)buf->data + offset, data, size);
        LOG_DEBUG("Set BufferSubData: target=0x%X, offset=%ld, size=%ld.", target, offset, size);
    } else {
        LOG_WARN("No buffer bound or buffer data is NULL for target=0x%X.", target);
        glSetError(GL_INVALID_OPERATION);
    }
}

// Implementation of glClear
GL_API void GL_APIENTRY glClear (GLbitfield mask) {
    glClear(mask);
    if (mask & GL_COLOR_BUFFER_BIT) {
        // Clear color buffer
        memset(gl_state.color_buffer, 0, sizeof(GLfloat) * 4); // Assuming RGBA
    }
    if (mask & GL_DEPTH_BUFFER_BIT) {
        gl_state.depth_buffer = gl_state.clear_depth;
    }
    if (mask & GL_STENCIL_BUFFER_BIT) {
        gl_state.stencil_buffer = 0;
    }
    LOG_DEBUG("Cleared buffers with mask=0x%X.", mask);
}

// Implementation of glClearColorx
GL_API void GL_APIENTRY glClearColorx (GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha) {
    glClearColorx(red, green, blue, alpha);
    gl_state.clear_color_fixed[0] = red;
    gl_state.clear_color_fixed[1] = green;
    gl_state.clear_color_fixed[2] = blue;
    gl_state.clear_color_fixed[3] = alpha;
    LOG_DEBUG("Set ClearColorx: (%d, %d, %d, %d).", red, green, blue, alpha);
}

// Implementation of glClearDepthx
GL_API void GL_APIENTRY glClearDepthx (GLfixed depth) {
    glClearDepthx(depth);
    gl_state.clear_depth_fixed = depth;
    LOG_DEBUG("Set ClearDepthx: %d.", depth);
}

// Implementation of glClearStencil
GL_API void GL_APIENTRY glClearStencil (GLint s) {
    glClearStencil(s);
    gl_state.stencil_clear_value = s;
    LOG_DEBUG("Set ClearStencil: %d.", s);
}

// Implementation of glClientActiveTexture
GL_API void GL_APIENTRY glClientActiveTexture (GLenum texture) {
    glClientActiveTexture(texture);
    gl_state.client_active_texture = texture - GL_TEXTURE0;
    LOG_DEBUG("Set ClientActiveTexture: 0x%X.", texture);
}

// Implementation of glClipPlanex
GL_API void GL_APIENTRY glClipPlanex (GLenum plane, const GLfixed *equation) {
    glClipPlanex(plane, equation);
    memcpy(gl_state.clip_planes_fixed[plane - GL_CLIP_PLANE0], equation, sizeof(GLfixed) * 4);
    LOG_DEBUG("Set ClipPlanex: plane=0x%X, equation=(%d, %d, %d, %d).", plane, equation[0], equation[1], equation[2], equation[3]);
}

// Implementation of glColor4ub
GL_API void GL_APIENTRY glColor4ub (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha) {
    glColor4ub(red, green, blue, alpha);
    gl_state.current_color_ub[0] = red;
    gl_state.current_color_ub[1] = green;
    gl_state.current_color_ub[2] = blue;
    gl_state.current_color_ub[3] = alpha;
    LOG_DEBUG("Set Color4ub: (%u, %u, %u, %u).", red, green, blue, alpha);
}

// Implementation of glColor4x
GL_API void GL_APIENTRY glColor4x (GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha) {
    glColor4x(red, green, blue, alpha);
    gl_state.current_color_fixed[0] = red;
    gl_state.current_color_fixed[1] = green;
    gl_state.current_color_fixed[2] = blue;
    gl_state.current_color_fixed[3] = alpha;
    LOG_DEBUG("Set Color4x: (%d, %d, %d, %d).", red, green, blue, alpha);
}

// Implementation of glColorMask
GL_API void GL_APIENTRY glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
    glColorMask(red, green, blue, alpha);
    gl_state.color_mask_red = red;
    gl_state.color_mask_green = green;
    gl_state.color_mask_blue = blue;
    gl_state.color_mask_alpha = alpha;
    LOG_DEBUG("Set ColorMask: red=%d, green=%d, blue=%d, alpha=%d.", red, green, blue, alpha);
}

// Implementation of glColorPointer
GL_API void GL_APIENTRY glColorPointer (GLint size, GLenum type, GLsizei stride, const void *pointer) {
    glColorPointer(size, type, stride, pointer);
    gl_state.color_array.size = size;
    gl_state.color_array.type = type;
    gl_state.color_array.stride = stride;
    gl_state.color_array.pointer = pointer;
    LOG_DEBUG("Set ColorPointer: size=%d, type=0x%X, stride=%d, pointer=%p.", size, type, stride, pointer);
}

// Implementation of glCompressedTexImage2D
GL_API void GL_APIENTRY glCompressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data) {
    glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
    Texture *tex = getTextureByID(gl_state.bound_texture[target]);
    if (tex) {
        tex->compressed = GL_TRUE;
        tex->compressed_image_size = imageSize;
        if (tex->compressed_data) {
            tracked_free(tex->compressed_data, tex->compressed_image_size);
        }
        tex->compressed_data = tracked_malloc(imageSize);
        if (tex->compressed_data) {
            memcpy(tex->compressed_data, data, imageSize);
            LOG_DEBUG("Set CompressedTexImage2D: target=0x%X, level=%d, internalformat=0x%X, width=%d, height=%d, border=%d, imageSize=%d.", target, level, internalformat, width, height, border, imageSize);
        } else {
            LOG_ERROR("Failed to allocate memory for CompressedTexImage2D.");
            glSetError(GL_OUT_OF_MEMORY);
        }
    } else {
        LOG_WARN("No texture bound to target=0x%X.", target);
        glSetError(GL_INVALID_OPERATION);
    }
}

// Implementation of glCompressedTexSubImage2D
GL_API void GL_APIENTRY glCompressedTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data) {
    glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
    Texture *tex = getTextureByID(gl_state.bound_texture[target]);
    if (tex && tex->compressed_data) {
        // Assuming simple handling without bounds checking
        memcpy((char*)tex->compressed_data + (yoffset * tex->width + xoffset) * 4, data, imageSize);
        LOG_DEBUG("Set CompressedTexSubImage2D: target=0x%X, level=%d, xoffset=%d, yoffset=%d, width=%d, height=%d, format=0x%X, imageSize=%d.", target, level, xoffset, yoffset, width, height, format, imageSize);
    } else {
        LOG_WARN("No compressed texture data bound to target=0x%X.", target);
        glSetError(GL_INVALID_OPERATION);
    }
}

// Implementation of glCopyTexImage2D
GL_API void GL_APIENTRY glCopyTexImage2D (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
    glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
    Texture *tex = getTextureByID(gl_state.bound_texture[target]);
    if (tex) {
        // Allocate memory and simulate texture copy
        if (tex->data) {
            tracked_free(tex->data, tex->width * tex->height * 4);
        }
        tex->width = width;
        tex->height = height;
        tex->data = tracked_malloc(width * height * 4);
        if (tex->data) {
            memset(tex->data, 255, width * height * 4); // Simulate white texture
            LOG_DEBUG("Set CopyTexImage2D: target=0x%X, level=%d, internalformat=0x%X, x=%d, y=%d, width=%d, height=%d, border=%d.", target, level, internalformat, x, y, width, height, border);
        } else {
            LOG_ERROR("Failed to allocate memory for CopyTexImage2D.");
            glSetError(GL_OUT_OF_MEMORY);
        }
    } else {
        LOG_WARN("No texture bound to target=0x%X.", target);
        glSetError(GL_INVALID_OPERATION);
    }
}

// Implementation of glCopyTexSubImage2D
GL_API void GL_APIENTRY glCopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
    Texture *tex = getTextureByID(gl_state.bound_texture[target]);
    if (tex && tex->data) {
        // Simulate copying by setting a region to a color
        for(int row = yoffset; row < yoffset + height; row++) {
            for(int col = xoffset; col < xoffset + width; col++) {
                int index = (row * tex->width + col) * 4;
                tex->data[index + 0] = 255; // R
                tex->data[index + 1] = 255; // G
                tex->data[index + 2] = 255; // B
                tex->data[index + 3] = 255; // A
            }
        }
        LOG_DEBUG("Set CopyTexSubImage2D: target=0x%X, level=%d, xoffset=%d, yoffset=%d, x=%d, y=%d, width=%d, height=%d.", target, level, xoffset, yoffset, x, y, width, height);
    } else {
        LOG_WARN("No texture data bound to target=0x%X.", target);
        glSetError(GL_INVALID_OPERATION);
    }
}

// Implementation of glCullFace
GL_API void GL_APIENTRY glCullFace (GLenum mode) {
    glCullFace(mode);
    gl_state.cull_face_mode = mode;
    LOG_DEBUG("Set CullFace: mode=0x%X.", mode);
}

// Implementation of glDeleteBuffers
GL_API void GL_APIENTRY glDeleteBuffers (GLsizei n, const GLuint *buffers) {
    glDeleteBuffers(n, buffers);
    for (GLsizei i = 0; i < n; ++i) {
        GLuint buf_id = buffers[i];
        Buffer *buf = getBufferByID(buf_id);
        if (buf) {
            if (buf->data) {
                tracked_free(buf->data, buf->size);
            }
            // Remove buffer from state
            for (int j = 0; j < gl_state.buffer_count; ++j) {
                if (gl_state.buffers[j].id == buf_id) {
                    for (int k = j; k < gl_state.buffer_count - 1; ++k) {
                        gl_state.buffers[k] = gl_state.buffers[k + 1];
                    }
                    gl_state.buffer_count--;
                    break;
                }
            }
            LOG_DEBUG("Deleted Buffer: ID=%u.", buf_id);
        } else {
            LOG_WARN("Attempted to delete non-existent Buffer ID=%u.", buf_id);
            glSetError(GL_INVALID_VALUE);
        }
    }
}

// Implementation of glDeleteTextures
GL_API void GL_APIENTRY glDeleteTextures (GLsizei n, const GLuint *textures) {
    glDeleteTextures(n, textures);
    for (GLsizei i = 0; i < n; ++i) {
        GLuint tex_id = textures[i];
        Texture *tex = getTextureByID(tex_id);
        if (tex) {
            if (tex->data) {
                tracked_free(tex->data, tex->width * tex->height * 4);
            }
            if (tex->compressed_data) {
                tracked_free(tex->compressed_data, tex->compressed_image_size);
            }
            // Remove texture from state
            for (int j = 0; j < gl_state.texture_count; ++j) {
                if (gl_state.textures[j].id == tex_id) {
                    for (int k = j; k < gl_state.texture_count - 1; ++k) {
                        gl_state.textures[k] = gl_state.textures[k + 1];
                    }
                    gl_state.texture_count--;
                    break;
                }
            }
            LOG_DEBUG("Deleted Texture: ID=%u.", tex_id);
        } else {
            LOG_WARN("Attempted to delete non-existent Texture ID=%u.", tex_id);
            glSetError(GL_INVALID_VALUE);
        }
    }
}

// Implementation of glDepthFunc
GL_API void GL_APIENTRY glDepthFunc (GLenum func) {
    glDepthFunc(func);
    gl_state.depth_func = func;
    LOG_DEBUG("Set DepthFunc: func=0x%X.", func);
}

// Implementation of glDepthMask
GL_API void GL_APIENTRY glDepthMask (GLboolean flag) {
    glDepthMask(flag);
    gl_state.depth_writemask = flag;
    LOG_DEBUG("Set DepthMask: flag=%d.", flag);
}

// Implementation of glDepthRangex
GL_API void GL_APIENTRY glDepthRangex (GLfixed n, GLfixed f) {
    glDepthRangex(n, f);
    gl_state.depth_range_fixed_near = n;
    gl_state.depth_range_fixed_far = f;
    LOG_DEBUG("Set DepthRangex: near=%d, far=%d.", n, f);
}

// Implementation of glDisable
GL_API void GL_APIENTRY glDisable (GLenum cap) {
    glDisable(cap);
    gl_state.enabled_caps &= ~cap;
    LOG_DEBUG("Disabled Capability: 0x%X.", cap);
}

// Implementation of glDisableClientState
GL_API void GL_APIENTRY glDisableClientState (GLenum array) {
    glDisableClientState(array);
    if (array == GL_VERTEX_ARRAY) {
        gl_state.client_enabled_vertex = GL_FALSE;
    } else if (array == GL_NORMAL_ARRAY) {
        gl_state.client_enabled_normal = GL_FALSE;
    } else if (array == GL_COLOR_ARRAY) {
        gl_state.client_enabled_color = GL_FALSE;
    } else if (array == GL_TEXTURE_COORD_ARRAY) {
        gl_state.client_enabled_texcoord = GL_FALSE;
    }
    LOG_DEBUG("Disabled ClientState: 0x%X.", array);
}

// Implementation of glDrawArrays
GL_API void GL_APIENTRY glDrawArrays (GLenum mode, GLint first, GLsizei count) {
    glDrawArrays(mode, first, count);
    LOG_DEBUG("Called DrawArrays: mode=0x%X, first=%d, count=%d.", mode, first, count);
    // Here you would add your rendering logic based on the current state and vertex arrays
}

// Implementation of glDrawElements
GL_API void GL_APIENTRY glDrawElements (GLenum mode, GLsizei count, GLenum type, const void *indices) {
    glDrawElements(mode, count, type, indices);
    LOG_DEBUG("Called DrawElements: mode=0x%X, count=%d, type=0x%X, indices=%p.", mode, count, type, indices);
    // Here you would add your indexed rendering logic based on the current state and index buffer
}

// Implementation of glEnable
GL_API void GL_APIENTRY glEnable (GLenum cap) {
    glEnable(cap);
    gl_state.enabled_caps |= cap;
    LOG_DEBUG("Enabled Capability: 0x%X.", cap);
}

// Implementation of glEnableClientState
GL_API void GL_APIENTRY glEnableClientState (GLenum array) {
    glEnableClientState(array);
    if (array == GL_VERTEX_ARRAY) {
        gl_state.client_enabled_vertex = GL_TRUE;
    } else if (array == GL_NORMAL_ARRAY) {
        gl_state.client_enabled_normal = GL_TRUE;
    } else if (array == GL_COLOR_ARRAY) {
        gl_state.client_enabled_color = GL_TRUE;
    } else if (array == GL_TEXTURE_COORD_ARRAY) {
        gl_state.client_enabled_texcoord = GL_TRUE;
    }
    LOG_DEBUG("Enabled ClientState: 0x%X.", array);
}

// Implementation of glFinish
GL_API void GL_APIENTRY glFinish (void) {
    glFinish();
    LOG_DEBUG("Called Finish.");
    // Ensure all rendering commands are completed
}

// Implementation of glFlush
GL_API void GL_APIENTRY glFlush (void) {
    glFlush();
    LOG_DEBUG("Called Flush.");
    // Flush the rendering commands
}

// Implementation of glFogx
GL_API void GL_APIENTRY glFogx (GLenum pname, GLfixed param) {
    glFogx(pname, param);
    switch(pname) {
        case GL_FOG_DENSITY:
            gl_state.fog_density_fixed = param;
            break;
        case GL_FOG_START:
            gl_state.fog_start_fixed = param;
            break;
        case GL_FOG_END:
            gl_state.fog_end_fixed = param;
            break;
        case GL_FOG_MODE:
            gl_state.fog_mode_fixed = param;
            break;
        default:
            break;
    }
    LOG_DEBUG("Set Fogx: pname=0x%X, param=%d.", pname, param);
}

// Implementation of glFogxv
GL_API void GL_APIENTRY glFogxv (GLenum pname, const GLfixed *param) {
    glFogxv(pname, param);
    switch(pname) {
        case GL_FOG_COLOR:
            memcpy(gl_state.fog_color_fixed, param, sizeof(GLfixed) * 4);
            break;
        default:
            break;
    }
    LOG_DEBUG("Set Fogxv: pname=0x%X.", pname);
}

// Implementation of glFrontFace
GL_API void GL_APIENTRY glFrontFace (GLenum mode) {
    glFrontFace(mode);
    gl_state.front_face = mode;
    LOG_DEBUG("Set FrontFace: mode=0x%X.", mode);
}

// Implementation of glFrustumx
GL_API void GL_APIENTRY glFrustumx (GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f) {
    glFrustumx(l, r, b, t, n, f);
    mat4 frustum_matrix;
    generateFrustumMatrixFixed(&frustum_matrix, l, r, b, t, n, f);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixx(frustum_matrix.data);
    LOG_DEBUG("Set Frustumx: l=%d, r=%d, b=%d, t=%d, n=%d, f=%d.", l, r, b, t, n, f);
}

// Implementation of glGetBooleanv
GL_API void GL_APIENTRY glGetBooleanv (GLenum pname, GLboolean *data) {
    glGetBooleanv(pname, data);
    switch(pname) {
        case GL_DEPTH_WRITEMASK:
            data[0] = gl_state.depth_writemask;
            break;
        case GL_COLOR_WRITEMASK:
            data[0] = gl_state.color_mask_red;
            data[1] = gl_state.color_mask_green;
            data[2] = gl_state.color_mask_blue;
            data[3] = gl_state.color_mask_alpha;
            break;
        case GL_CULL_FACE_MODE:
            data[0] = (gl_state.cull_face_mode == GL_FRONT || gl_state.cull_face_mode == GL_BACK || gl_state.cull_face_mode == GL_FRONT_AND_BACK) ? GL_TRUE : GL_FALSE;
            break;
        default:
            break;
    }
    LOG_DEBUG("Get Booleanv: pname=0x%X.", pname);
}

// Implementation of glGetBufferParameteriv
GL_API void GL_APIENTRY glGetBufferParameteriv (GLenum target, GLenum pname, GLint *params) {
    glGetBufferParameteriv(target, pname, params);
    Buffer *buf = getBufferByID(gl_state.bound_buffer[target]);
    if (buf) {
        switch(pname) {
            case GL_BUFFER_SIZE:
                params[0] = buf->size;
                break;
            case GL_BUFFER_USAGE:
                params[0] = buf->usage;
                break;
            default:
                break;
        }
    }
    LOG_DEBUG("Get BufferParameteriv: target=0x%X, pname=0x%X.", target, pname);
}

// Implementation of glGetClipPlanex
GL_API void GL_APIENTRY glGetClipPlanex (GLenum plane, GLfixed *equation) {
    glGetClipPlanex(plane, equation);
    memcpy(equation, gl_state.clip_planes_fixed[plane - GL_CLIP_PLANE0], sizeof(GLfixed) * 4);
    LOG_DEBUG("Get ClipPlanex: plane=0x%X.", plane);
}

// Implementation of glGenBuffers
GL_API void GL_APIENTRY glGenBuffers (GLsizei n, GLuint *buffers) {
    glGenBuffers(n, buffers);
    for (GLsizei i = 0; i < n; ++i) {
        if (gl_state.buffer_count < MAX_BUFFERS) {
            GLuint new_id = gl_state.buffer_count + 1;
            buffers[i] = new_id;
            gl_state.buffers[gl_state.buffer_count].id = new_id;
            gl_state.buffers[gl_state.buffer_count].target = 0;
            gl_state.buffers[gl_state.buffer_count].data = NULL;
            gl_state.buffers[gl_state.buffer_count].size = 0;
            gl_state.buffers[gl_state.buffer_count].usage = 0;
            gl_state.buffer_count++;
            LOG_DEBUG("Generated Buffer ID %u.", new_id);
        } else {
            LOG_WARN("Maximum buffer limit reached.");
            buffers[i] = 0;
            glSetError(GL_OUT_OF_MEMORY);
        }
    }
}

// Implementation of glGenTextures
GL_API void GL_APIENTRY glGenTextures (GLsizei n, GLuint *textures) {
    glGenTextures(n, textures);
    for (GLsizei i = 0; i < n; ++i) {
        if (gl_state.texture_count < MAX_TEXTURES) {
            GLuint new_id = gl_state.texture_count + 1;
            textures[i] = new_id;
            gl_state.textures[gl_state.texture_count].id = new_id;
            gl_state.textures[gl_state.texture_count].target = 0;
            gl_state.textures[gl_state.texture_count].width = 0;
            gl_state.textures[gl_state.texture_count].height = 0;
            gl_state.textures[gl_state.texture_count].data = NULL;
            gl_state.textures[gl_state.texture_count].compressed = GL_FALSE;
            gl_state.textures[gl_state.texture_count].compressed_image_size = 0;
            gl_state.textures[gl_state.texture_count].compressed_data = NULL;
            gl_state.texture_count++;
            LOG_DEBUG("Generated Texture ID %u.", new_id);
        } else {
            LOG_WARN("Maximum texture limit reached.");
            textures[i] = 0;
            glSetError(GL_OUT_OF_MEMORY);
        }
    }
}

// Implementation of glGetError
GL_API GLenum GL_APIENTRY glGetError (void) {
    GLenum error = glGetError();
    LOG_DEBUG("GetError: 0x%X.", error);
    return error;
}

// Implementation of glGetFixedv
GL_API void GL_APIENTRY glGetFixedv (GLenum pname, GLfixed *params) {
    glGetFixedv(pname, params);
    switch(pname) {
        case GL_MODELVIEW_MATRIX:
            memcpy(params, gl_state.modelview_matrix_fixed.data, sizeof(mat4) * 4);
            break;
        case GL_PROJECTION_MATRIX:
            memcpy(params, gl_state.projection_matrix_fixed.data, sizeof(mat4) * 4);
            break;
        case GL_TEXTURE_MATRIX:
            memcpy(params, gl_state.texture_matrix_fixed.data, sizeof(mat4) * 4);
            break;
        default:
            break;
    }
    LOG_DEBUG("Get Fixedv: pname=0x%X.", pname);
}

// Implementation of glGetIntegerv
GL_API void GL_APIENTRY glGetIntegerv (GLenum pname, GLint *data) {
    glGetIntegerv(pname, data);
    switch(pname) {
        case GL_MAX_LIGHTS:
            data[0] = gl_state.max_lights;
            break;
        case GL_MAX_CLIP_PLANES:
            data[0] = gl_state.max_clip_planes;
            break;
        case GL_MAX_TEXTURE_SIZE:
            data[0] = gl_state.max_texture_size;
            break;
        case GL_MAX_MODELVIEW_STACK_DEPTH:
            data[0] = gl_state.max_modelview_stack_depth;
            break;
        case GL_MAX_PROJECTION_STACK_DEPTH:
            data[0] = gl_state.max_projection_stack_depth;
            break;
        case GL_MAX_TEXTURE_STACK_DEPTH:
            data[0] = gl_state.max_texture_stack_depth;
            break;
        case GL_MAX_VIEWPORT_DIMS:
            data[0] = gl_state.max_viewport_dims[0];
            data[1] = gl_state.max_viewport_dims[1];
            break;
        case GL_MAX_TEXTURE_UNITS:
            data[0] = gl_state.max_texture_units;
            break;
        case GL_SUBPIXEL_BITS:
            data[0] = gl_state.subpixel_bits;
            break;
        case GL_RED_BITS:
            data[0] = gl_state.red_bits;
            break;
        case GL_GREEN_BITS:
            data[0] = gl_state.green_bits;
            break;
        case GL_BLUE_BITS:
            data[0] = gl_state.blue_bits;
            break;
        case GL_ALPHA_BITS:
            data[0] = gl_state.alpha_bits;
            break;
        case GL_DEPTH_BITS:
            data[0] = gl_state.depth_bits;
            break;
        case GL_STENCIL_BITS:
            data[0] = gl_state.stencil_bits;
            break;
        case GL_MATRIX_MODE:
            data[0] = gl_state.matrix_mode;
            break;
        case GL_VIEWPORT:
            data[0] = gl_state.viewport[0];
            data[1] = gl_state.viewport[1];
            data[2] = gl_state.viewport[2];
            data[3] = gl_state.viewport[3];
            break;
        default:
            break;
    }
    LOG_DEBUG("GetIntegerv: pname=0x%X.", pname);
}

// Implementation of glGetLightxv
GL_API void GL_APIENTRY glGetLightxv (GLenum light, GLenum pname, GLfixed *params) {
    glGetLightxv(light, pname, params);
    LOG_DEBUG("Get Lightxv: light=0x%X, pname=0x%X.", light, pname);
}

// Implementation of glGetMaterialxv
GL_API void GL_APIENTRY glGetMaterialxv (GLenum face, GLenum pname, GLfixed *params) {
    glGetMaterialxv(face, pname, params);
    LOG_DEBUG("Get Materialxv: face=0x%X, pname=0x%X.", face, pname);
}

// Implementation of glGetPointerv
GL_API void GL_APIENTRY glGetPointerv (GLenum pname, void **params) {
    glGetPointerv(pname, params);
    switch(pname) {
        case GL_VERTEX_ARRAY_POINTER:
            *params = gl_state.vertex_array.pointer;
            break;
        case GL_NORMAL_ARRAY_POINTER:
            *params = gl_state.normal_array.pointer;
            break;
        case GL_COLOR_ARRAY_POINTER:
            *params = gl_state.color_array.pointer;
            break;
        case GL_TEXTURE_COORD_ARRAY_POINTER:
            *params = gl_state.texcoord_array.pointer;
            break;
        default:
            break;
    }
    LOG_DEBUG("Get Pointerv: pname=0x%X.", pname);
}

// Implementation of glGetString
GL_API const GLubyte *GL_APIENTRY glGetString (GLenum name) {
    const GLubyte *str = glGetString(name);
    LOG_DEBUG("GetString: name=0x%X, string=%s.", name, str ? (const char*)str : "NULL");
    return str;
}

// Implementation of glGetTexEnviv
GL_API void GL_APIENTRY glGetTexEnviv (GLenum target, GLenum pname, GLint *params) {
    glGetTexEnviv(target, pname, params);
    LOG_DEBUG("Get TexEnviv: target=0x%X, pname=0x%X.", target, pname);
}

// Implementation of glGetTexEnvxv
GL_API void GL_APIENTRY glGetTexEnvxv (GLenum target, GLenum pname, GLfixed *params) {
    glGetTexEnvxv(target, pname, params);
    LOG_DEBUG("Get TexEnvxv: target=0x%X, pname=0x%X.", target, pname);
}

// Implementation of glGetTexParameteriv
GL_API void GL_APIENTRY glGetTexParameteriv (GLenum target, GLenum pname, GLint *params) {
    glGetTexParameteriv(target, pname, params);
    LOG_DEBUG("Get TexParameteriv: target=0x%X, pname=0x%X.", target, pname);
}

// Implementation of glGetTexParameterxv
GL_API void GL_APIENTRY glGetTexParameterxv (GLenum target, GLenum pname, GLfixed *params) {
    glGetTexParameterxv(target, pname, params);
    LOG_DEBUG("Get TexParameterxv: target=0x%X, pname=0x%X.", target, pname);
}

// Implementation of glHint
GL_API void GL_APIENTRY glHint (GLenum target, GLenum mode) {
    glHint(target, mode);
    gl_state.hints[target] = mode;
    LOG_DEBUG("Set Hint: target=0x%X, mode=0x%X.", target, mode);
}

// Implementation of glIsBuffer
GL_API GLboolean GL_APIENTRY glIsBuffer (GLuint buffer) {
    GLboolean result = GL_FALSE;
    for(int i = 0; i < gl_state.buffer_count; ++i) {
        if(gl_state.buffers[i].id == buffer) {
            result = GL_TRUE;
            break;
        }
    }
    LOG_DEBUG("IsBuffer: buffer=%u, result=%d.", buffer, result);
    return result;
}

// Implementation of glIsEnabled
GL_API GLboolean GL_APIENTRY glIsEnabled (GLenum cap) {
    GLboolean result = (gl_state.enabled_caps & cap) ? GL_TRUE : GL_FALSE;
    LOG_DEBUG("IsEnabled: cap=0x%X, result=%d.", cap, result);
    return result;
}

// Implementation of glIsTexture
GL_API GLboolean GL_APIENTRY glIsTexture (GLuint texture) {
    GLboolean result = GL_FALSE;
    for(int i = 0; i < gl_state.texture_count; ++i) {
        if(gl_state.textures[i].id == texture) {
            result = GL_TRUE;
            break;
        }
    }
    LOG_DEBUG("IsTexture: texture=%u, result=%d.", texture, result);
    return result;
}

// Implementation of glLightModelx
GL_API void GL_APIENTRY glLightModelx (GLenum pname, GLfixed param) {
    glLightModelx(pname, param);
    LOG_DEBUG("Set LightModelx: pname=0x%X, param=%d.", pname, param);
}

// Implementation of glLightModelxv
GL_API void GL_APIENTRY glLightModelxv (GLenum pname, const GLfixed *param) {
    glLightModelxv(pname, param);
    LOG_DEBUG("Set LightModelxv: pname=0x%X.", pname);
}

// Implementation of glLightx
GL_API void GL_APIENTRY glLightx (GLenum light, GLenum pname, GLfixed param) {
    glLightx(light, pname, param);
    LOG_DEBUG("Set Lightx: light=0x%X, pname=0x%X, param=%d.", light, pname, param);
}

// Implementation of glLightxv
GL_API void GL_APIENTRY glLightxv (GLenum light, GLenum pname, const GLfixed *params) {
    glLightxv(light, pname, params);
    LOG_DEBUG("Set Lightxv: light=0x%X, pname=0x%X.", light, pname);
}

// Implementation of glLineWidthx
GL_API void GL_APIENTRY glLineWidthx (GLfixed width) {
    glLineWidthx(width);
    gl_state.line_width_fixed = width;
    LOG_DEBUG("Set LineWidthx: %d.", width);
}

// Implementation of glLoadIdentity
GL_API void GL_APIENTRY glLoadIdentity (void) {
    glLoadIdentity();
    mat4 identity;
    generateIdentityMatrix(&identity);
    if (gl_state.matrix_mode == GL_MODELVIEW) {
        memcpy(gl_state.modelview_matrix.data, identity.data, sizeof(mat4));
    } else if (gl_state.matrix_mode == GL_PROJECTION) {
        memcpy(gl_state.projection_matrix.data, identity.data, sizeof(mat4));
    } else if (gl_state.matrix_mode == GL_TEXTURE) {
        memcpy(gl_state.texture_matrix.data, identity.data, sizeof(mat4));
    }
    glLoadMatrixf(identity.data);
    LOG_DEBUG("Loaded Identity Matrix for mode=0x%X.", gl_state.matrix_mode);
}

// Implementation of glLoadMatrixx
GL_API void GL_APIENTRY glLoadMatrixx (const GLfixed *m) {
    glLoadMatrixx(m);
    mat4 matrix;
    memcpy(matrix.data, m, sizeof(mat4));
    if (gl_state.matrix_mode == GL_MODELVIEW) {
        memcpy(gl_state.modelview_matrix_fixed.data, m, sizeof(mat4));
    } else if (gl_state.matrix_mode == GL_PROJECTION) {
        memcpy(gl_state.projection_matrix_fixed.data, m, sizeof(mat4));
    } else if (gl_state.matrix_mode == GL_TEXTURE) {
        memcpy(gl_state.texture_matrix_fixed.data, m, sizeof(mat4));
    }
    LOG_DEBUG("Loaded Matrixx for mode=0x%X.", gl_state.matrix_mode);
}

// Implementation of glLogicOp
GL_API void GL_APIENTRY glLogicOp (GLenum opcode) {
    glLogicOp(opcode);
    gl_state.logic_op = opcode;
    LOG_DEBUG("Set LogicOp: opcode=0x%X.", opcode);
}

// Implementation of glMaterialx
GL_API void GL_APIENTRY glMaterialx (GLenum face, GLenum pname, GLfixed param) {
    glMaterialx(face, pname, param);
    LOG_DEBUG("Set Materialx: face=0x%X, pname=0x%X, param=%d.", face, pname, param);
}

// Implementation of glMaterialxv
GL_API void GL_APIENTRY glMaterialxv (GLenum face, GLenum pname, const GLfixed *param) {
    glMaterialxv(face, pname, param);
    LOG_DEBUG("Set Materialxv: face=0x%X, pname=0x%X.", face, pname);
}

// Implementation of glMatrixMode
GL_API void GL_APIENTRY glMatrixMode (GLenum mode) {
    glMatrixMode(mode);
    gl_state.matrix_mode = mode;
    LOG_DEBUG("Set MatrixMode: mode=0x%X.", mode);
}

// Implementation of glMultMatrixx
GL_API void GL_APIENTRY glMultMatrixx (const GLfixed *m) {
    glMultMatrixx(m);
    mat4 matrix;
    memcpy(matrix.data, m, sizeof(mat4));
    if (gl_state.matrix_mode == GL_MODELVIEW) {
        multiplyMatricesFixed(&gl_state.modelview_matrix_fixed, &matrix, &gl_state.modelview_matrix_fixed);
        glLoadMatrixx(gl_state.modelview_matrix_fixed.data);
    } else if (gl_state.matrix_mode == GL_PROJECTION) {
        multiplyMatricesFixed(&gl_state.projection_matrix_fixed, &matrix, &gl_state.projection_matrix_fixed);
        glLoadMatrixx(gl_state.projection_matrix_fixed.data);
    } else if (gl_state.matrix_mode == GL_TEXTURE) {
        multiplyMatricesFixed(&gl_state.texture_matrix_fixed, &matrix, &gl_state.texture_matrix_fixed);
        glLoadMatrixx(gl_state.texture_matrix_fixed.data);
    }
    LOG_DEBUG("Multiplied current matrix by provided matrix for mode=0x%X.", gl_state.matrix_mode);
}

// Implementation of glMultiTexCoord4x
GL_API void GL_APIENTRY glMultiTexCoord4x (GLenum texture, GLfixed s, GLfixed t, GLfixed r, GLfixed q) {
    glMultiTexCoord4x(texture, s, t, r, q);
    LOG_DEBUG("Set MultiTexCoord4x: texture=0x%X, s=%d, t=%d, r=%d, q=%d.", texture, s, t, r, q);
}

// Implementation of glNormal3x
GL_API void GL_APIENTRY glNormal3x (GLfixed nx, GLfixed ny, GLfixed nz) {
    glNormal3x(nx, ny, nz);
    gl_state.current_normal_fixed[0] = nx;
    gl_state.current_normal_fixed[1] = ny;
    gl_state.current_normal_fixed[2] = nz;
    LOG_DEBUG("Set Normal3x: (%d, %d, %d).", nx, ny, nz);
}

// Implementation of glNormalPointer
GL_API void GL_APIENTRY glNormalPointer (GLenum type, GLsizei stride, const void *pointer) {
    glNormalPointer(type, stride, pointer);
    gl_state.normal_array.type = type;
    gl_state.normal_array.stride = stride;
    gl_state.normal_array.pointer = pointer;
    LOG_DEBUG("Set NormalPointer: type=0x%X, stride=%d, pointer=%p.", type, stride, pointer);
}

// Implementation of glOrthox
GL_API void GL_APIENTRY glOrthox (GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f) {
    glOrthox(l, r, b, t, n, f);
    mat4 ortho_matrix;
    generateOrthographicMatrixFixed(&ortho_matrix, l, r, b, t, n, f);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixx(ortho_matrix.data);
    LOG_DEBUG("Set Orthox: l=%d, r=%d, b=%d, t=%d, n=%d, f=%d.", l, r, b, t, n, f);
}

// Implementation of glPixelStorei
GL_API void GL_APIENTRY glPixelStorei (GLenum pname, GLint param) {
    glPixelStorei(pname, param);
    switch(pname) {
        case GL_UNPACK_ALIGNMENT:
            gl_state.unpack_alignment = param;
            break;
        case GL_PACK_ALIGNMENT:
            gl_state.pack_alignment = param;
            break;
        default:
            break;
    }
    LOG_DEBUG("Set PixelStorei: pname=0x%X, param=%d.", pname, param);
}

// Implementation of glPointParameterx
GL_API void GL_APIENTRY glPointParameterx (GLenum pname, GLfixed param) {
    glPointParameterx(pname, param);
    switch(pname) {
        case GL_POINT_SIZE_MIN:
            gl_state.point_size_min_fixed = param;
            break;
        case GL_POINT_SIZE_MAX:
            gl_state.point_size_max_fixed = param;
            break;
        case GL_POINT_FADE_THRESHOLD_SIZE:
            gl_state.point_fade_threshold_fixed = param;
            break;
        case GL_POINT_DISTANCE_ATTENUATION:
            gl_state.point_distance_attenuation_fixed[0] = param;
            gl_state.point_distance_attenuation_fixed[1] = param;
            gl_state.point_distance_attenuation_fixed[2] = param;
            break;
        default:
            break;
    }
    LOG_DEBUG("Set PointParameterx: pname=0x%X, param=%d.", pname, param);
}

// Implementation of glPointParameterxv
GL_API void GL_APIENTRY glPointParameterxv (GLenum pname, const GLfixed *params) {
    glPointParameterxv(pname, params);
    switch(pname) {
        case GL_POINT_DISTANCE_ATTENUATION:
            memcpy(gl_state.point_distance_attenuation_fixed, params, sizeof(GLfixed) * 3);
            break;
        default:
            break;
    }
    LOG_DEBUG("Set PointParameterxv: pname=0x%X.", pname);
}

// Implementation of glPointSizex
GL_API void GL_APIENTRY glPointSizex (GLfixed size) {
    glPointSizex(size);
    gl_state.point_size_fixed = size;
    LOG_DEBUG("Set PointSizex: %d.", size);
}

// Implementation of glPolygonOffsetx
GL_API void GL_APIENTRY glPolygonOffsetx (GLfixed factor, GLfixed units) {
    glPolygonOffsetx(factor, units);
    gl_state.polygon_offset_factor_fixed = factor;
    gl_state.polygon_offset_units_fixed = units;
    LOG_DEBUG("Set PolygonOffsetx: factor=%d, units=%d.", factor, units);
}

// Implementation of glPopMatrix
GL_API void GL_APIENTRY glPopMatrix (void) {
    glPopMatrix();
    if (gl_state.matrix_mode == GL_MODELVIEW && gl_state.modelview_stack_depth > 0) {
        gl_state.modelview_stack_depth--;
        memcpy(gl_state.modelview_matrix_fixed.data, gl_state.modelview_matrix_stack[gl_state.modelview_stack_depth].data, sizeof(mat4));
        glLoadMatrixx(gl_state.modelview_matrix_fixed.data);
    } else if (gl_state.matrix_mode == GL_PROJECTION && gl_state.projection_stack_depth > 0) {
        gl_state.projection_stack_depth--;
        memcpy(gl_state.projection_matrix_fixed.data, gl_state.projection_matrix_stack[gl_state.projection_stack_depth].data, sizeof(mat4));
        glLoadMatrixx(gl_state.projection_matrix_fixed.data);
    } else if (gl_state.matrix_mode == GL_TEXTURE && gl_state.texture_stack_depth > 0) {
        gl_state.texture_stack_depth--;
        memcpy(gl_state.texture_matrix_fixed.data, gl_state.texture_matrix_stack[gl_state.texture_stack_depth].data, sizeof(mat4));
        glLoadMatrixx(gl_state.texture_matrix_fixed.data);
    }
    LOG_DEBUG("Popped matrix for mode=0x%X.", gl_state.matrix_mode);
}

// Implementation of glPushMatrix
GL_API void GL_APIENTRY glPushMatrix (void) {
    glPushMatrix();
    if (gl_state.matrix_mode == GL_MODELVIEW && gl_state.modelview_stack_depth < MAX_STACK_DEPTH) {
        memcpy(gl_state.modelview_matrix_stack[gl_state.modelview_stack_depth].data, gl_state.modelview_matrix_fixed.data, sizeof(mat4));
        gl_state.modelview_stack_depth++;
    } else if (gl_state.matrix_mode == GL_PROJECTION && gl_state.projection_stack_depth < MAX_STACK_DEPTH) {
        memcpy(gl_state.projection_matrix_stack[gl_state.projection_stack_depth].data, gl_state.projection_matrix_fixed.data, sizeof(mat4));
        gl_state.projection_stack_depth++;
    } else if (gl_state.matrix_mode == GL_TEXTURE && gl_state.texture_stack_depth < MAX_STACK_DEPTH) {
        memcpy(gl_state.texture_matrix_stack[gl_state.texture_stack_depth].data, gl_state.texture_matrix_fixed.data, sizeof(mat4));
        gl_state.texture_stack_depth++;
    }
    LOG_DEBUG("Pushed matrix for mode=0x%X.", gl_state.matrix_mode);
}

// Implementation of glReadPixels
GL_API void GL_APIENTRY glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels) {
    glReadPixels(x, y, width, height, format, type, pixels);
    LOG_DEBUG("Called ReadPixels: x=%d, y=%d, width=%d, height=%d, format=0x%X, type=0x%X, pixels=%p.", x, y, width, height, format, type, pixels);
    // Implement actual pixel reading based on framebuffer data if needed
}

// Implementation of glRotatex
GL_API void GL_APIENTRY glRotatex (GLfixed angle, GLfixed x, GLfixed y, GLfixed z) {
    glRotatex(angle, x, y, z);
    mat4 rotation;
    generateRotationMatrixFixed(&rotation, angle, x, y, z);
    if (gl_state.matrix_mode == GL_MODELVIEW) {
        multiplyMatricesFixed(&gl_state.modelview_matrix_fixed, &rotation, &gl_state.modelview_matrix_fixed);
        glLoadMatrixx(gl_state.modelview_matrix_fixed.data);
    } else if (gl_state.matrix_mode == GL_PROJECTION) {
        multiplyMatricesFixed(&gl_state.projection_matrix_fixed, &rotation, &gl_state.projection_matrix_fixed);
        glLoadMatrixx(gl_state.projection_matrix_fixed.data);
    } else if (gl_state.matrix_mode == GL_TEXTURE) {
        multiplyMatricesFixed(&gl_state.texture_matrix_fixed, &rotation, &gl_state.texture_matrix_fixed);
        glLoadMatrixx(gl_state.texture_matrix_fixed.data);
    }
    LOG_DEBUG("Applied Rotatex: angle=%d, axis=(%d, %d, %d).", angle, x, y, z);
}

// Implementation of glSampleCoverage
GL_API void GL_APIENTRY glSampleCoverage (GLfloat value, GLboolean invert) {
    glSampleCoverage(value, invert);
    gl_state.sample_coverage_value = value;
    gl_state.sample_coverage_invert = invert;
    LOG_DEBUG("Set SampleCoverage: value=%f, invert=%d.", value, invert);
}

// Implementation of glSampleCoveragex
GL_API void GL_APIENTRY glSampleCoveragex (GLclampx value, GLboolean invert) {
    glSampleCoveragex(value, invert);
    gl_state.sample_coverage_fixed_value = value;
    gl_state.sample_coverage_invert = invert;
    LOG_DEBUG("Set SampleCoveragex: value=%d, invert=%d.", value, invert);
}

// Implementation of glScalex
GL_API void GL_APIENTRY glScalex (GLfixed x, GLfixed y, GLfixed z) {
    glScalex(x, y, z);
    mat4 scale;
    generateScaleMatrixFixed(&scale, x, y, z);
    if (gl_state.matrix_mode == GL_MODELVIEW) {
        multiplyMatricesFixed(&gl_state.modelview_matrix_fixed, &scale, &gl_state.modelview_matrix_fixed);
        glLoadMatrixx(gl_state.modelview_matrix_fixed.data);
    } else if (gl_state.matrix_mode == GL_PROJECTION) {
        multiplyMatricesFixed(&gl_state.projection_matrix_fixed, &scale, &gl_state.projection_matrix_fixed);
        glLoadMatrixx(gl_state.projection_matrix_fixed.data);
    } else if (gl_state.matrix_mode == GL_TEXTURE) {
        multiplyMatricesFixed(&gl_state.texture_matrix_fixed, &scale, &gl_state.texture_matrix_fixed);
        glLoadMatrixx(gl_state.texture_matrix_fixed.data);
    }
    LOG_DEBUG("Applied Scalex: x=%d, y=%d, z=%d.", x, y, z);
}

// Implementation of glScissor
GL_API void GL_APIENTRY glScissor (GLint x, GLint y, GLsizei width, GLsizei height) {
    glScissor(x, y, width, height);
    gl_state.scissor_box[0] = x;
    gl_state.scissor_box[1] = y;
    gl_state.scissor_box[2] = width;
    gl_state.scissor_box[3] = height;
    LOG_DEBUG("Set Scissor: x=%d, y=%d, width=%d, height=%d.", x, y, width, height);
}

// Implementation of glShadeModel
GL_API void GL_APIENTRY glShadeModel (GLenum mode) {
    glShadeModel(mode);
    gl_state.shade_model = mode;
    LOG_DEBUG("Set ShadeModel: mode=0x%X.", mode);
}

// renderer.c
#include "gl_types.h"
#include "gl_errors.h"
#include "matrix_utils.h"
#include "logger.h"
#include "memory_tracker.h"

// Assume GLState is a global or accessible structure
extern GLState gl_state;

// Implementation of glStencilFunc
GL_API void GL_APIENTRY glStencilFunc (GLenum func, GLint ref, GLuint mask) {
    glStencilFunc(func, ref, mask);
    gl_state.stencil_func = func;
    gl_state.stencil_ref = ref;
    gl_state.stencil_mask = mask;
    LOG_DEBUG("Set StencilFunc: func=0x%X, ref=%d, mask=0x%X.", func, ref, mask);
}

// Implementation of glStencilMask
GL_API void GL_APIENTRY glStencilMask (GLuint mask) {
    glStencilMask(mask);
    gl_state.stencil_writemask = mask;
    LOG_DEBUG("Set StencilMask: mask=0x%X.", mask);
}

// Implementation of glStencilOp
GL_API void GL_APIENTRY glStencilOp (GLenum fail, GLenum zfail, GLenum zpass) {
    glStencilOp(fail, zfail, zpass);
    gl_state.stencil_fail = fail;
    gl_state.stencil_zfail = zfail;
    gl_state.stencil_zpass = zpass;
    LOG_DEBUG("Set StencilOp: fail=0x%X, zfail=0x%X, zpass=0x%X.", fail, zfail, zpass);
}

// Implementation of glTexCoordPointer
GL_API void GL_APIENTRY glTexCoordPointer (GLint size, GLenum type, GLsizei stride, const void *pointer) {
    glTexCoordPointer(size, type, stride, pointer);
    gl_state.texcoord_array.size = size;
    gl_state.texcoord_array.type = type;
    gl_state.texcoord_array.stride = stride;
    gl_state.texcoord_array.pointer = pointer;
    LOG_DEBUG("Set TexCoordPointer: size=%d, type=0x%X, stride=%d, pointer=%p.", size, type, stride, pointer);
}

// Implementation of glTexEnvi
GL_API void GL_APIENTRY glTexEnvi (GLenum target, GLenum pname, GLint param) {
    glTexEnvi(target, pname, param);
    if (target == GL_TEXTURE_ENV) {
        switch(pname) {
            case GL_TEXTURE_ENV_MODE:
                gl_state.tex_env_mode = param;
                break;
            default:
                break;
        }
    }
    LOG_DEBUG("Set TexEnvi: target=0x%X, pname=0x%X, param=%d.", target, pname, param);
}

// Implementation of glTexEnvx
GL_API void GL_APIENTRY glTexEnvx (GLenum target, GLenum pname, GLfixed param) {
    glTexEnvx(target, pname, param);
    if (target == GL_TEXTURE_ENV) {
        switch(pname) {
            case GL_TEXTURE_ENV_MODE:
                gl_state.tex_env_mode_fixed = param;
                break;
            default:
                break;
        }
    }
    LOG_DEBUG("Set TexEnvx: target=0x%X, pname=0x%X, param=%d.", target, pname, param);
}

// Implementation of glTexEnviv
GL_API void GL_APIENTRY glTexEnviv (GLenum target, GLenum pname, const GLint *params) {
    glTexEnviv(target, pname, params);
    LOG_DEBUG("Set TexEnviv: target=0x%X, pname=0x%X.", target, pname);
}

// Implementation of glTexEnvxv
GL_API void GL_APIENTRY glTexEnvxv (GLenum target, GLenum pname, const GLfixed *params) {
    glTexEnvxv(target, pname, params);
    LOG_DEBUG("Set TexEnvxv: target=0x%X, pname=0x%X.", target, pname);
}

// Implementation of glTexImage2D
GL_API void GL_APIENTRY glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels) {
    glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
    Texture *tex = getTextureByID(gl_state.bound_texture[target]);
    if (tex) {
        tex->level = level;
        tex->internal_format = internalformat;
        tex->width = width;
        tex->height = height;
        tex->border = border;
        tex->format = format;
        tex->type = type;
        if (tex->data) {
            tracked_free(tex->data, tex->width * tex->height * 4); // Assuming RGBA
        }
        tex->data = tracked_malloc(width * height * 4);
        if (tex->data) {
            memcpy(tex->data, pixels, width * height * 4);
            LOG_DEBUG("Set TexImage2D: target=0x%X, level=%d, internalformat=0x%X, width=%d, height=%d, border=%d, format=0x%X, type=0x%X, pixels=%p.", target, level, internalformat, width, height, border, format, type, pixels);
        } else {
            LOG_ERROR("Failed to allocate memory for TexImage2D.");
            glSetError(GL_OUT_OF_MEMORY);
        }
    } else {
        LOG_WARN("No texture bound to target=0x%X.", target);
        glSetError(GL_INVALID_OPERATION);
    }
}

// Implementation of glTexParameteri
GL_API void GL_APIENTRY glTexParameteri (GLenum target, GLenum pname, GLint param) {
    glTexParameteri(target, pname, param);
    Texture *tex = getTextureByID(gl_state.bound_texture[target]);
    if (tex) {
        switch(pname) {
            case GL_TEXTURE_MIN_FILTER:
                tex->min_filter = param;
                break;
            case GL_TEXTURE_MAG_FILTER:
                tex->mag_filter = param;
                break;
            case GL_TEXTURE_WRAP_S:
                tex->wrap_s = param;
                break;
            case GL_TEXTURE_WRAP_T:
                tex->wrap_t = param;
                break;
            default:
                break;
        }
    }
    LOG_DEBUG("Set TexParameteri: target=0x%X, pname=0x%X, param=%d.", target, pname, param);
}

// Implementation of glTexParameterx
GL_API void GL_APIENTRY glTexParameterx (GLenum target, GLenum pname, GLfixed param) {
    glTexParameterx(target, pname, param);
    Texture *tex = getTextureByID(gl_state.bound_texture[target]);
    if (tex) {
        switch(pname) {
            case GL_TEXTURE_MIN_FILTER:
                tex->min_filter_fixed = param;
                break;
            case GL_TEXTURE_MAG_FILTER:
                tex->mag_filter_fixed = param;
                break;
            case GL_TEXTURE_WRAP_S:
                tex->wrap_s_fixed = param;
                break;
            case GL_TEXTURE_WRAP_T:
                tex->wrap_t_fixed = param;
                break;
            default:
                break;
        }
    }
    LOG_DEBUG("Set TexParameterx: target=0x%X, pname=0x%X, param=%d.", target, pname, param);
}

// Implementation of glTexParameteriv
GL_API void GL_APIENTRY glTexParameteriv (GLenum target, GLenum pname, const GLint *params) {
    glTexParameteriv(target, pname, params);
    LOG_DEBUG("Set TexParameteriv: target=0x%X, pname=0x%X.", target, pname);
}

// Implementation of glTexParameterxv
GL_API void GL_APIENTRY glTexParameterxv (GLenum target, GLenum pname, const GLfixed *params) {
    glTexParameterxv(target, pname, params);
    LOG_DEBUG("Set TexParameterxv: target=0x%X, pname=0x%X.", target, pname);
}

// Implementation of glTexSubImage2D
GL_API void GL_APIENTRY glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels) {
    glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
    Texture *tex = getTextureByID(gl_state.bound_texture[target]);
    if (tex && tex->data) {
        // Simple simulation: copy pixels into texture data
        for(int row = 0; row < height; row++) {
            memcpy((char*)tex->data + ((yoffset + row) * tex->width + xoffset) * 4, 
                   (char*)pixels + (row * width * 4), 
                   width * 4);
        }
        LOG_DEBUG("Set TexSubImage2D: target=0x%X, level=%d, xoffset=%d, yoffset=%d, width=%d, height=%d, format=0x%X, type=0x%X, pixels=%p.", target, level, xoffset, yoffset, width, height, format, type, pixels);
    } else {
        LOG_WARN("No texture data bound to target=0x%X.", target);
        glSetError(GL_INVALID_OPERATION);
    }
}

// Implementation of glTranslatex
GL_API void GL_APIENTRY glTranslatex (GLfixed x, GLfixed y, GLfixed z) {
    glTranslatex(x, y, z);
    mat4 translation;
    generateTranslationMatrixFixed(&translation, x, y, z);
    if (gl_state.matrix_mode == GL_MODELVIEW) {
        multiplyMatricesFixed(&gl_state.modelview_matrix_fixed, &translation, &gl_state.modelview_matrix_fixed);
        glLoadMatrixx(gl_state.modelview_matrix_fixed.data);
    } else if (gl_state.matrix_mode == GL_PROJECTION) {
        multiplyMatricesFixed(&gl_state.projection_matrix_fixed, &translation, &gl_state.projection_matrix_fixed);
        glLoadMatrixx(gl_state.projection_matrix_fixed.data);
    } else if (gl_state.matrix_mode == GL_TEXTURE) {
        multiplyMatricesFixed(&gl_state.texture_matrix_fixed, &translation, &gl_state.texture_matrix_fixed);
        glLoadMatrixx(gl_state.texture_matrix_fixed.data);
    }
    LOG_DEBUG("Applied Translatex: x=%d, y=%d, z=%d.", x, y, z);
}

// Implementation of glVertexPointer
GL_API void GL_APIENTRY glVertexPointer (GLint size, GLenum type, GLsizei stride, const void *pointer) {
    glVertexPointer(size, type, stride, pointer);
    gl_state.vertex_array.size = size;
    gl_state.vertex_array.type = type;
    gl_state.vertex_array.stride = stride;
    gl_state.vertex_array.pointer = pointer;
    LOG_DEBUG("Set VertexPointer: size=%d, type=0x%X, stride=%d, pointer=%p.", size, type, stride, pointer);
}

// Implementation of glViewport
GL_API void GL_APIENTRY glViewport (GLint x, GLint y, GLsizei width, GLsizei height) {
    glViewport(x, y, width, height);
    gl_state.viewport[0] = x;
    gl_state.viewport[1] = y;
    gl_state.viewport[2] = width;
    gl_state.viewport[3] = height;
    LOG_DEBUG("Set Viewport: x=%d, y=%d, width=%d, height=%d.", x, y, width, height);
}
