/* matrix_utils.h */

#ifndef MATRIX_UTILS_H
#define MATRIX_UTILS_H
/**
 * @file matrix_utils.h
 * @brief Helper math routines for matrix operations.
 */

#include <GLES/gl.h>
#include <GLES/glext.h>
#include <stdalign.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Define a 4x4 matrix structure with 16-byte alignment */
typedef struct {
	alignas(16) GLfloat data[16];
} mat4;

/* Define a 3D vector structure with 16-byte alignment */
typedef struct {
	alignas(16) GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat w; /* Padding for alignment if needed */
} vec3;

/* Define a quaternion structure with 16-byte alignment */
typedef struct {
	alignas(16) GLfloat w;
	GLfloat x;
	GLfloat y;
	GLfloat z;
} Quaternion;

/* Compile-time checks */
static_assert(sizeof(mat4) == sizeof(GLfloat) * 16, "mat4 must be 16 floats");
static_assert(alignof(mat4) >= 16, "mat4 must be 16-byte aligned");
static_assert(alignof(vec3) >= 16, "vec3 must be 16-byte aligned");
static_assert(alignof(Quaternion) >= 16, "Quaternion must be 16-byte aligned");

/* Vector Utility Functions */
void vec3_normalize(GLfloat *restrict x, GLfloat *restrict y,
		    GLfloat *restrict z);
void vec3_cross(GLfloat aX, GLfloat aY, GLfloat aZ, GLfloat bX, GLfloat bY,
		GLfloat bZ, GLfloat *restrict outX, GLfloat *restrict outY,
		GLfloat *restrict outZ);
GLfloat vec3_dot(GLfloat aX, GLfloat aY, GLfloat aZ, GLfloat bX, GLfloat bY,
		 GLfloat bZ);
GLfloat vec3_magnitude(GLfloat x, GLfloat y, GLfloat z);
void vec3_add(GLfloat aX, GLfloat aY, GLfloat aZ, GLfloat bX, GLfloat bY,
	      GLfloat bZ, GLfloat *restrict outX, GLfloat *restrict outY,
	      GLfloat *restrict outZ);
void vec3_subtract(GLfloat aX, GLfloat aY, GLfloat aZ, GLfloat bX, GLfloat bY,
		   GLfloat bZ, GLfloat *restrict outX, GLfloat *restrict outY,
		   GLfloat *restrict outZ);

/* Matrix Utility Functions */
void mat4_identity(mat4 *restrict mat);
void mat4_copy(mat4 *restrict dest, const mat4 *restrict src);
void mat4_multiply(mat4 *restrict result, const mat4 *restrict a,
		   const mat4 *restrict b);
int mat4_inverse(mat4 *restrict mat);
void mat4_transpose(mat4 *restrict mat);
void mat4_translate(mat4 *restrict mat, GLfloat x, GLfloat y, GLfloat z);
void mat4_scale(mat4 *restrict mat, GLfloat x, GLfloat y, GLfloat z);
void mat4_scale_uniform(mat4 *restrict mat, GLfloat scale);
void mat4_rotate_x(mat4 *restrict mat, GLfloat angle);
void mat4_rotate_y(mat4 *restrict mat, GLfloat angle);
void mat4_rotate_z(mat4 *restrict mat, GLfloat angle);
void mat4_rotate_axis(mat4 *restrict mat, GLfloat angle, GLfloat x, GLfloat y,
		      GLfloat z);
void mat4_look_at(mat4 *restrict mat, GLfloat eyeX, GLfloat eyeY, GLfloat eyeZ,
		  GLfloat centerX, GLfloat centerY, GLfloat centerZ,
		  GLfloat upX, GLfloat upY, GLfloat upZ);
void mat4_perspective(mat4 *restrict mat, GLfloat fovy, GLfloat aspect,
		      GLfloat zNear, GLfloat zFar);
void mat4_frustum(mat4 *restrict mat, GLfloat left, GLfloat right,
		  GLfloat bottom, GLfloat top, GLfloat nearVal, GLfloat farVal);
void mat4_orthographic(mat4 *restrict mat, GLfloat left, GLfloat right,
		       GLfloat bottom, GLfloat top, GLfloat nearVal,
		       GLfloat farVal);
void mat4_perspective_divide(mat4 *restrict mat);
void mat4_transform_vec4(const mat4 *restrict mat, const GLfloat in[restrict 4],
			 GLfloat out[restrict 4]);

/* Quaternion Utility Functions */
void quat_normalize(Quaternion *restrict q);
void quat_from_axis_angle(Quaternion *restrict q, GLfloat angle, GLfloat x,
			  GLfloat y, GLfloat z);
void quat_to_mat4(const Quaternion *restrict q, mat4 *restrict mat);

#ifdef __cplusplus
}
#endif

#endif /* MATRIX_UTILS_H */
