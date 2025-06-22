#include "gl_state.h"
#include "gl_context.h"
#include "gl_errors.h"
#include <GLES/gl.h>
#include <string.h>
#include "gl_utils.h"

static GLboolean valid_light_enum(GLenum light)
{
	return light >= GL_LIGHT0 && light <= GL_LIGHT7;
}

GL_API void GL_APIENTRY glLightf(GLenum light, GLenum pname, GLfloat param)
{
	if (!valid_light_enum(light)) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	Light *lt = &gl_state.lights[light - GL_LIGHT0];
	switch (pname) {
	case GL_SPOT_EXPONENT:
		if (param < 0.0f || param > 128.0f) {
			glSetError(GL_INVALID_VALUE);
			return;
		}
		lt->spot_exponent = param;
		break;
	case GL_SPOT_CUTOFF:
		if ((param < 0.0f || param > 90.0f) && param != 180.0f) {
			glSetError(GL_INVALID_VALUE);
			return;
		}
		lt->spot_cutoff = param;
		break;
	case GL_CONSTANT_ATTENUATION:
		if (param < 0.0f) {
			glSetError(GL_INVALID_VALUE);
			return;
		}
		lt->constant_attenuation = param;
		break;
	case GL_LINEAR_ATTENUATION:
		if (param < 0.0f) {
			glSetError(GL_INVALID_VALUE);
			return;
		}
		lt->linear_attenuation = param;
		break;
	case GL_QUADRATIC_ATTENUATION:
		if (param < 0.0f) {
			glSetError(GL_INVALID_VALUE);
			return;
		}
		lt->quadratic_attenuation = param;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		return;
	}
	context_set_light(light, pname, &param);
}

GL_API void GL_APIENTRY glLightx(GLenum light, GLenum pname, GLfixed param)
{
	glLightf(light, pname, fixed_to_float(param));
}

GL_API void GL_APIENTRY glLightfv(GLenum light, GLenum pname,
				  const GLfloat *params)
{
	if (!valid_light_enum(light)) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	Light *lt = &gl_state.lights[light - GL_LIGHT0];
	switch (pname) {
	case GL_AMBIENT:
		memcpy(lt->ambient, params, sizeof(GLfloat) * 4);
		break;
	case GL_DIFFUSE:
		memcpy(lt->diffuse, params, sizeof(GLfloat) * 4);
		break;
	case GL_SPECULAR:
		memcpy(lt->specular, params, sizeof(GLfloat) * 4);
		break;
	case GL_POSITION: {
		GLfloat tmp[4];
		mat4_transform_vec4(&gl_state.modelview_matrix, params, tmp);
		memcpy(lt->position, tmp, sizeof(GLfloat) * 4);
		break;
	}
	case GL_SPOT_DIRECTION: {
		GLfloat in[4] = { params[0], params[1], params[2], 0.0f };
		GLfloat tmp[4];
		mat4_transform_vec4(&gl_state.modelview_matrix, in, tmp);
		lt->spot_direction[0] = tmp[0];
		lt->spot_direction[1] = tmp[1];
		lt->spot_direction[2] = tmp[2];
		break;
	}
	case GL_SPOT_EXPONENT:
	case GL_SPOT_CUTOFF:
	case GL_CONSTANT_ATTENUATION:
	case GL_LINEAR_ATTENUATION:
	case GL_QUADRATIC_ATTENUATION:
		glLightf(light, pname, params[0]);
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		return;
	}
	context_set_light(light, pname, params);
}

GL_API void GL_APIENTRY glLightxv(GLenum light, GLenum pname,
				  const GLfixed *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat vals[4] = { fixed_to_float(params[0]),
			    fixed_to_float(params[1]),
			    fixed_to_float(params[2]),
			    fixed_to_float(params[3]) };
	glLightfv(light, pname, vals);
}

GL_API void GL_APIENTRY glLightModelf(GLenum pname, GLfloat param)
{
	if (pname == GL_LIGHT_MODEL_TWO_SIDE) {
		gl_state.light_model_two_side = param ? GL_TRUE : GL_FALSE;
	} else {
		glSetError(GL_INVALID_ENUM);
	}
}

GL_API void GL_APIENTRY glLightModelx(GLenum pname, GLfixed param)
{
	glLightModelf(pname, fixed_to_float(param));
}

GL_API void GL_APIENTRY glLightModelfv(GLenum pname, const GLfloat *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	switch (pname) {
	case GL_LIGHT_MODEL_AMBIENT:
		memcpy(gl_state.light_model_ambient, params,
		       sizeof(GLfloat) * 4);
		break;
	case GL_LIGHT_MODEL_TWO_SIDE:
		gl_state.light_model_two_side = params[0] ? GL_TRUE : GL_FALSE;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}

GL_API void GL_APIENTRY glLightModelxv(GLenum pname, const GLfixed *param)
{
	if (!param) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat vals[4] = { fixed_to_float(param[0]), fixed_to_float(param[1]),
			    fixed_to_float(param[2]),
			    fixed_to_float(param[3]) };
	glLightModelfv(pname, vals);
}

static Material *select_material(GLenum face, int *count)
{
	switch (face) {
	case GL_FRONT:
		*count = 1;
		return &gl_state.material[0];
	case GL_BACK:
		*count = 1;
		return &gl_state.material[1];
	case GL_FRONT_AND_BACK:
		*count = 2;
		return gl_state.material;
	default:
		glSetError(GL_INVALID_ENUM);
		return NULL;
	}
}

GL_API void GL_APIENTRY glMaterialf(GLenum face, GLenum pname, GLfloat param)
{
	glMaterialfv(face, pname, &param);
}

GL_API void GL_APIENTRY glMaterialx(GLenum face, GLenum pname, GLfixed param)
{
	glMaterialf(face, pname, fixed_to_float(param));
}

GL_API void GL_APIENTRY glMaterialfv(GLenum face, GLenum pname,
				     const GLfloat *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	int count;
	Material *mat = select_material(face, &count);
	if (!mat)
		return;
	for (int i = 0; i < count; ++i) {
		switch (pname) {
		case GL_AMBIENT:
			memcpy(mat[i].ambient, params, sizeof(GLfloat) * 4);
			if (i == 0)
				context_set_material(GL_AMBIENT, params);
			break;
		case GL_DIFFUSE:
			memcpy(mat[i].diffuse, params, sizeof(GLfloat) * 4);
			if (i == 0)
				context_set_material(GL_DIFFUSE, params);
			break;
		case GL_SPECULAR:
			memcpy(mat[i].specular, params, sizeof(GLfloat) * 4);
			if (i == 0)
				context_set_material(GL_SPECULAR, params);
			break;
		case GL_EMISSION:
			memcpy(mat[i].emission, params, sizeof(GLfloat) * 4);
			if (i == 0)
				context_set_material(GL_EMISSION, params);
			break;
		case GL_SHININESS:
			if (params[0] < 0.0f || params[0] > 128.0f) {
				glSetError(GL_INVALID_VALUE);
				return;
			}
			mat[i].shininess = params[0];
			if (i == 0)
				context_set_material(GL_SHININESS, params);
			break;
		case GL_AMBIENT_AND_DIFFUSE:
			memcpy(mat[i].ambient, params, sizeof(GLfloat) * 4);
			memcpy(mat[i].diffuse, params, sizeof(GLfloat) * 4);
			if (i == 0) {
				context_set_material(GL_AMBIENT, params);
				context_set_material(GL_DIFFUSE, params);
			}
			break;
		default:
			glSetError(GL_INVALID_ENUM);
			return;
		}
	}
}

GL_API void GL_APIENTRY glMaterialxv(GLenum face, GLenum pname,
				     const GLfixed *param)
{
	if (!param) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat vals[4] = { fixed_to_float(param[0]), fixed_to_float(param[1]),
			    fixed_to_float(param[2]),
			    fixed_to_float(param[3]) };
	glMaterialfv(face, pname, vals);
}
