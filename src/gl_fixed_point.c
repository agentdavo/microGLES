/* gl_fixed_point.c */

#include "gl_fixed_point.h"
#include "gl_errors.h"
#include "matrix_utils.h"
#include "logger.h"
#include "memory_tracker.h"

// Assume GLState is a global or accessible structure
extern GLState gl_state;

// Implementation of glAlphaFuncxOES
GL_API void GL_APIENTRY glAlphaFuncxOES (GLenum func, GLfixed ref) {
    glAlphaFuncxOES(func, ref);
    gl_state.alpha_func = func;
    gl_state.alpha_ref_fixed = ref;
    LOG_DEBUG("Set AlphaFuncxOES: func=0x%X, ref=%d.", func, ref);
}

// Implementation of glClearColorxOES
GL_API void GL_APIENTRY glClearColorxOES (GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha) {
    glClearColorxOES(red, green, blue, alpha);
    gl_state.clear_color_fixed[0] = red;
    gl_state.clear_color_fixed[1] = green;
    gl_state.clear_color_fixed[2] = blue;
    gl_state.clear_color_fixed[3] = alpha;
    LOG_DEBUG("Set ClearColorxOES: (%d, %d, %d, %d).", red, green, blue, alpha);
}

// Implementation of glClearDepthxOES
GL_API void GL_APIENTRY glClearDepthxOES (GLfixed depth) {
    glClearDepthxOES(depth);
    gl_state.clear_depth_fixed = depth;
    LOG_DEBUG("Set ClearDepthxOES: %d.", depth);
}

// Implementation of glClipPlanexOES
GL_API void GL_APIENTRY glClipPlanexOES (GLenum plane, const GLfixed *equation) {
    glClipPlanexOES(plane, equation);
    memcpy(gl_state.clip_planes_fixed[plane - GL_CLIP_PLANE0], equation, sizeof(GLfixed) * 4);
    LOG_DEBUG("Set ClipPlanexOES: plane=0x%X, equation=(%d, %d, %d, %d).", plane, equation[0], equation[1], equation[2], equation[3]);
}

// Implementation of glColor4xOES
GL_API void GL_APIENTRY glColor4xOES (GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha) {
    glColor4xOES(red, green, blue, alpha);
    gl_state.current_color_fixed[0] = red;
    gl_state.current_color_fixed[1] = green;
    gl_state.current_color_fixed[2] = blue;
    gl_state.current_color_fixed[3] = alpha;
    LOG_DEBUG("Set Color4xOES: (%d, %d, %d, %d).", red, green, blue, alpha);
}

// Implementation of glDepthRangexOES
GL_API void GL_APIENTRY glDepthRangexOES (GLfixed n, GLfixed f) {
    glDepthRangexOES(n, f);
    gl_state.depth_range_fixed_near = n;
    gl_state.depth_range_fixed_far = f;
    LOG_DEBUG("Set DepthRangexOES: near=%d, far=%d.", n, f);
}

// Implementation of glFogxOES
GL_API void GL_APIENTRY glFogxOES (GLenum pname, GLfixed param) {
    glFogxOES(pname, param);
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
    LOG_DEBUG("Set FogxOES: pname=0x%X, param=%d.", pname, param);
}

// Implementation of glFogxvOES
GL_API void GL_APIENTRY glFogxvOES (GLenum pname, const GLfixed *param) {
    glFogxvOES(pname, param);
    switch(pname) {
        case GL_FOG_COLOR:
            memcpy(gl_state.fog_color_fixed, param, sizeof(GLfixed) * 4);
            break;
        default:
            break;
    }
    LOG_DEBUG("Set FogxvOES: pname=0x%X.", pname);
}

// Implementation of glFrustumxOES
GL_API void GL_APIENTRY glFrustumxOES (GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f) {
    glFrustumxOES(l, r, b, t, n, f);
    mat4 frustum_matrix_fixed;
    generateFrustumMatrixFixed(&frustum_matrix_fixed, l, r, b, t, n, f);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixxOES(frustum_matrix_fixed.data);
    LOG_DEBUG("Set FrustumxOES: l=%d, r=%d, b=%d, t=%d, n=%d, f=%d.", l, r, b, t, n, f);
}

// Implementation of glGetClipPlanexOES
GL_API void GL_APIENTRY glGetClipPlanexOES (GLenum plane, GLfixed *equation) {
    glGetClipPlanexOES(plane, equation);
    memcpy(equation, gl_state.clip_planes_fixed[plane - GL_CLIP_PLANE0], sizeof(GLfixed) * 4);
    LOG_DEBUG("Get ClipPlanexOES: plane=0x%X.", plane);
}

// Implementation of glGetFixedvOES
GL_API void GL_APIENTRY glGetFixedvOES (GLenum pname, GLfixed *params) {
    glGetFixedvOES(pname, params);
    switch(pname) {
        case GL_MODELVIEW_MATRIX:
            memcpy(params, gl_state.modelview_matrix_fixed.data, sizeof(mat4));
            break;
        case GL_PROJECTION_MATRIX:
            memcpy(params, gl_state.projection_matrix_fixed.data, sizeof(mat4));
            break;
        case GL_TEXTURE_MATRIX:
            memcpy(params, gl_state.texture_matrix_fixed.data, sizeof(mat4));
            break;
        default:
            break;
    }
    LOG_DEBUG("Get FixedvOES: pname=0x%X.", pname);
}

// Implementation of glGetTexEnvxvOES
GL_API void GL_APIENTRY glGetTexEnvxvOES (GLenum target, GLenum pname, GLfixed *params) {
    glGetTexEnvxvOES(target, pname, params);
    LOG_DEBUG("Get TexEnvxvOES: target=0x%X, pname=0x%X.", target, pname);
}

// Implementation of glGetTexParameterxvOES
GL_API void GL_APIENTRY glGetTexParameterxvOES (GLenum target, GLenum pname, GLfixed *params) {
    glGetTexParameterxvOES(target, pname, params);
    LOG_DEBUG("Get TexParameterxvOES: target=0x%X, pname=0x%X.", target, pname);
}

// Implementation of glLightModelxOES
GL_API void GL_APIENTRY glLightModelxOES (GLenum pname, GLfixed param) {
    glLightModelxOES(pname, param);
    LOG_DEBUG("Set LightModelxOES: pname=0x%X, param=%d.", pname, param);
}

// Implementation of glLightModelxvOES
GL_API void GL_APIENTRY glLightModelxvOES (GLenum pname, const GLfixed *param) {
    glLightModelxvOES(pname, param);
    LOG_DEBUG("Set LightModelxvOES: pname=0x%X.", pname);
}

// Implementation of glLightxOES
GL_API void GL_APIENTRY glLightxOES (GLenum light, GLenum pname, GLfixed param) {
    glLightxOES(light, pname, param);
    LOG_DEBUG("Set LightxOES: light=0x%X, pname=0x%X, param=%d.", light, pname, param);
}

// Implementation of glLightxvOES
GL_API void GL_APIENTRY glLightxvOES (GLenum light, GLenum pname, const GLfixed *params) {
    glLightxvOES(light, pname, params);
    LOG_DEBUG("Set LightxvOES: light=0x%X, pname=0x%X.", light, pname);
}

// Implementation of glLineWidthxOES
GL_API void GL_APIENTRY glLineWidthxOES (GLfixed width) {
    glLineWidthxOES(width);
    gl_state.line_width_fixed = width;
    LOG_DEBUG("Set LineWidthxOES: %d.", width);
}

// Implementation of glLoadMatrixxOES
GL_API void GL_APIENTRY glLoadMatrixxOES (const GLfixed *m) {
    glLoadMatrixxOES(m);
    mat4 matrix_fixed;
    memcpy(matrix_fixed.data, m, sizeof(mat4));
    if (gl_state.matrix_mode == GL_MODELVIEW) {
        memcpy(gl_state.modelview_matrix_fixed.data, m, sizeof(mat4));
    } else if (gl_state.matrix_mode == GL_PROJECTION) {
        memcpy(gl_state.projection_matrix_fixed.data, m, sizeof(mat4));
    } else if (gl_state.matrix_mode == GL_TEXTURE) {
        memcpy(gl_state.texture_matrix_fixed.data, m, sizeof(mat4));
    }
    LOG_DEBUG("Loaded MatrixxOES for mode=0x%X.", gl_state.matrix_mode);
}

// Implementation of glMaterialxOES
GL_API void GL_APIENTRY glMaterialxOES (GLenum face, GLenum pname, GLfixed param) {
    glMaterialxOES(face, pname, param);
    LOG_DEBUG("Set MaterialxOES: face=0x%X, pname=0x%X, param=%d.", face, pname, param);
}

// Implementation of glMaterialxvOES
GL_API void GL_APIENTRY glMaterialxvOES (GLenum face, GLenum pname, const GLfixed *param) {
    glMaterialxvOES(face, pname, param);
    LOG_DEBUG("Set MaterialxvOES: face=0x%X, pname=0x%X.", face, pname);
}

// Implementation of glMultMatrixxOES
GL_API void GL_APIENTRY glMultMatrixxOES (const GLfixed *m) {
    glMultMatrixxOES(m);
    mat4 matrix_fixed;
    memcpy(matrix_fixed.data, m, sizeof(mat4));
    if (gl_state.matrix_mode == GL_MODELVIEW) {
        multiplyMatricesFixed(&gl_state.modelview_matrix_fixed, &matrix_fixed, &gl_state.modelview_matrix_fixed);
        glLoadMatrixxOES(gl_state.modelview_matrix_fixed.data);
    } else if (gl_state.matrix_mode == GL_PROJECTION) {
        multiplyMatricesFixed(&gl_state.projection_matrix_fixed, &matrix_fixed, &gl_state.projection_matrix_fixed);
        glLoadMatrixxOES(gl_state.projection_matrix_fixed.data);
    } else if (gl_state.matrix_mode == GL_TEXTURE) {
        multiplyMatricesFixed(&gl_state.texture_matrix_fixed, &matrix_fixed, &gl_state.texture_matrix_fixed);
        glLoadMatrixxOES(gl_state.texture_matrix_fixed.data);
    }
    LOG_DEBUG("Multiplied current matrix by provided matrix for mode=0x%X.", gl_state.matrix_mode);
}

// Implementation of glMultiTexCoord4xOES
GL_API void GL_APIENTRY glMultiTexCoord4xOES (GLenum texture, GLfixed s, GLfixed t, GLfixed r, GLfixed q) {
    glMultiTexCoord4xOES(texture, s, t, r, q);
    LOG_DEBUG("Set MultiTexCoord4xOES: texture=0x%X, s=%d, t=%d, r=%d, q=%d.", texture, s, t, r, q);
}

// Implementation of glNormal3xOES
GL_API void GL_APIENTRY glNormal3xOES (GLfixed nx, GLfixed ny, GLfixed nz) {
    glNormal3xOES(nx, ny, nz);
    gl_state.current_normal_fixed[0] = nx;
    gl_state.current_normal_fixed[1] = ny;
    gl_state.current_normal_fixed[2] = nz;
    LOG_DEBUG("Set Normal3xOES: (%d, %d, %d).", nx, ny, nz);
}

// Implementation of glOrthoxOES
GL_API void GL_APIENTRY glOrthoxOES (GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f) {
    glOrthoxOES(l, r, b, t, n, f);
    mat4 ortho_matrix_fixed;
    generateOrthographicMatrixFixed(&ortho_matrix_fixed, l, r, b, t, n, f);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixxOES(ortho_matrix_fixed.data);
    LOG_DEBUG("Set OrthoxOES: l=%d, r=%d, b=%d, t=%d, n=%d, f=%d.", l, r, b, t, n, f);
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

// Implementation of glPointParameterxOES
GL_API void GL_APIENTRY glPointParameterxOES (GLenum pname, GLfixed param) {
    glPointParameterxOES(pname, param);
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
    LOG_DEBUG("Set PointParameterxOES: pname=0x%X, param=%d.", pname, param);
}

// Implementation of glPointParameterxvOES
GL_API void GL_APIENTRY glPointParameterxvOES (GLenum pname, const GLfixed *params) {
    glPointParameterxvOES(pname, params);
    switch(pname) {
        case GL_POINT_DISTANCE_ATTENUATION:
            memcpy(gl_state.point_distance_attenuation_fixed, params, sizeof(GLfixed) * 3);
            break;
        default:
            break;
    }
    LOG_DEBUG("Set PointParameterxvOES: pname=0x%X.", pname);
}

// Implementation of glPointSizexOES
GL_API void GL_APIENTRY glPointSizexOES (GLfixed size) {
    glPointSizexOES(size);
    gl_state.point_size_fixed = size;
    LOG_DEBUG("Set PointSizexOES: %d.", size);
}

// Implementation of glPolygonOffsetxOES
GL_API void GL_APIENTRY glPolygonOffsetxOES (GLfixed factor, GLfixed units) {
    glPolygonOffsetxOES(factor, units);
    gl_state.polygon_offset_factor_fixed = factor;
    gl_state.polygon_offset_units_fixed = units;
    LOG_DEBUG("Set PolygonOffsetxOES: factor=%d, units=%d.", factor, units);
}

// Implementation of glRotatexOES
GL_API void GL_APIENTRY glRotatexOES (GLfixed angle, GLfixed x, GLfixed y, GLfixed z) {
    glRotatexOES(angle, x, y, z);
    mat4 rotation_fixed;
    generateRotationMatrixFixed(&rotation_fixed, angle, x, y, z);
    if (gl_state.matrix_mode == GL_MODELVIEW) {
        multiplyMatricesFixed(&gl_state.modelview_matrix_fixed, &rotation_fixed, &gl_state.modelview_matrix_fixed);
        glLoadMatrixxOES(gl_state.modelview_matrix_fixed.data);
    } else if (gl_state.matrix_mode == GL_PROJECTION) {
        multiplyMatricesFixed(&gl_state.projection_matrix_fixed, &rotation_fixed, &gl_state.projection_matrix_fixed);
        glLoadMatrixxOES(gl_state.projection_matrix_fixed.data);
    } else if (gl_state.matrix_mode == GL_TEXTURE) {
        multiplyMatricesFixed(&gl_state.texture_matrix_fixed, &rotation_fixed, &gl_state.texture_matrix_fixed);
        glLoadMatrixxOES(gl_state.texture_matrix_fixed.data);
    }
    LOG_DEBUG("Applied RotatexOES: angle=%d, axis=(%d, %d, %d).", angle, x, y, z);
}

// Implementation of glSampleCoveragexOES
GL_API void GL_APIENTRY glSampleCoveragexOES (GLclampx value, GLboolean invert) {
    glSampleCoveragexOES(value, invert);
    gl_state.sample_coverage_fixed_value = value;
    gl_state.sample_coverage_invert = invert;
    LOG_DEBUG("Set SampleCoveragexOES: value=%d, invert=%d.", value, invert);
}

// Implementation of glScalexOES
GL_API void GL_APIENTRY glScalexOES (GLfixed x, GLfixed y, GLfixed z) {
    glScalexOES(x, y, z);
    mat4 scale_fixed;
    generateScaleMatrixFixed(&scale_fixed, x, y, z);
    if (gl_state.matrix_mode == GL_MODELVIEW) {
        multiplyMatricesFixed(&gl_state.modelview_matrix_fixed, &scale_fixed, &gl_state.modelview_matrix_fixed);
        glLoadMatrixxOES(gl_state.modelview_matrix_fixed.data);
    } else if (gl_state.matrix_mode == GL_PROJECTION) {
        multiplyMatricesFixed(&gl_state.projection_matrix_fixed, &scale_fixed, &gl_state.projection_matrix_fixed);
        glLoadMatrixxOES(gl_state.projection_matrix_fixed.data);
    } else if (gl_state.matrix_mode == GL_TEXTURE) {
        multiplyMatricesFixed(&gl_state.texture_matrix_fixed, &scale_fixed, &gl_state.texture_matrix_fixed);
        glLoadMatrixxOES(gl_state.texture_matrix_fixed.data);
    }
    LOG_DEBUG("Applied ScalexOES: x=%d, y=%d, z=%d.", x, y, z);
}

// Implementation of glTexEnvxOES
GL_API void GL_APIENTRY glTexEnvxOES (GLenum target, GLenum pname, GLfixed param) {
    glTexEnvxOES(target, pname, param);
    if (target == GL_TEXTURE_ENV) {
        switch(pname) {
            case GL_TEXTURE_ENV_MODE:
                gl_state.tex_env_mode_fixed = param;
                break;
            default:
                break;
        }
    }
    LOG_DEBUG("Set TexEnvxOES: target=0x%X, pname=0x%X, param=%d.", target, pname, param);
}

// Implementation of glTexEnvxvOES
GL_API void GL_APIENTRY glTexEnvxvOES (GLenum target, GLenum pname, const GLfixed *params) {
    glTexEnvxvOES(target, pname, params);
    LOG_DEBUG("Set TexEnvxvOES: target=0x%X, pname=0x%X.", target, pname);
}

// Implementation of glTexParameterxOES
GL_API void GL_APIENTRY glTexParameterxOES (GLenum target, GLenum pname, GLfixed param) {
    glTexParameterxOES(target, pname, param);
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
    LOG_DEBUG("Set TexParameterxOES: target=0x%X, pname=0x%X, param=%d.", target, pname, param);
}

// Implementation of glTexParameterxvOES
GL_API void GL_APIENTRY glTexParameterxvOES (GLenum target, GLenum pname, const GLfixed *params) {
    glTexParameterxvOES(target, pname, params);
    LOG_DEBUG("Set TexParameterxvOES: target=0x%X, pname=0x%X.", target, pname);
}

// Implementation of glTranslatexOES
GL_API void GL_APIENTRY glTranslatexOES (GLfixed x, GLfixed y, GLfixed z) {
    glTranslatexOES(x, y, z);
    mat4 translation_fixed;
    generateTranslationMatrixFixed(&translation_fixed, x, y, z);
    if (gl_state.matrix_mode == GL_MODELVIEW) {
        multiplyMatricesFixed(&gl_state.modelview_matrix_fixed, &translation_fixed, &gl_state.modelview_matrix_fixed);
        glLoadMatrixxOES(gl_state.modelview_matrix_fixed.data);
    } else if (gl_state.matrix_mode == GL_PROJECTION) {
        multiplyMatricesFixed(&gl_state.projection_matrix_fixed, &translation_fixed, &gl_state.projection_matrix_fixed);
        glLoadMatrixxOES(gl_state.projection_matrix_fixed.data);
    } else if (gl_state.matrix_mode == GL_TEXTURE) {
        multiplyMatricesFixed(&gl_state.texture_matrix_fixed, &translation_fixed, &gl_state.texture_matrix_fixed);
        glLoadMatrixxOES(gl_state.texture_matrix_fixed.data);
    }
    LOG_DEBUG("Applied TranslatexOES: x=%d, y=%d, z=%d.", x, y, z);
}

// Implementation of glGetLightxvOES
GL_API void GL_APIENTRY glGetLightxvOES (GLenum light, GLenum pname, GLfixed *params) {
    glGetLightxvOES(light, pname, params);
    LOG_DEBUG("Get LightxvOES: light=0x%X, pname=0x%X.", light, pname);
}

// Implementation of glGetMaterialxvOES
GL_API void GL_APIENTRY glGetMaterialxvOES (GLenum face, GLenum pname, GLfixed *params) {
    glGetMaterialxvOES(face, pname, params);
    LOG_DEBUG("Get MaterialxvOES: face=0x%X, pname=0x%X.", face, pname);
}
