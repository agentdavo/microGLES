/* matrix_utils.c */

#include "matrix_utils.h"
#include "gl_logger.h" // Ensure logger is initialized before using
#include "c11_opt.h"

/* Define Pi for angle conversions */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Helper Macros for Degrees to Radians Conversion */
#define DEG2RAD(angle) ((angle) * (M_PI / 180.0f))

/* ---------------------- */
/* Vector Utility Functions */
/* ---------------------- */

void vec3_normalize(GLfloat *restrict x, GLfloat *restrict y,
		    GLfloat *restrict z)
{
	GLfloat length = GL_SQRT((*x) * (*x) + (*y) * (*y) + (*z) * (*z));
	if (length > 0.0f) {
		*x /= length;
		*y /= length;
		*z /= length;
	} else {
		/* Avoid spamming the logs when normalizing a zero vector. */
		*x = 0.0f;
		*y = 0.0f;
		*z = 0.0f;
	}
}

void vec3_cross(GLfloat aX, GLfloat aY, GLfloat aZ, GLfloat bX, GLfloat bY,
		GLfloat bZ, GLfloat *restrict outX, GLfloat *restrict outY,
		GLfloat *restrict outZ)
{
	*outX = aY * bZ - aZ * bY;
	*outY = aZ * bX - aX * bZ;
	*outZ = aX * bY - aY * bX;
}

GLfloat vec3_dot(GLfloat aX, GLfloat aY, GLfloat aZ, GLfloat bX, GLfloat bY,
		 GLfloat bZ)
{
	return aX * bX + aY * bY + aZ * bZ;
}

GLfloat vec3_magnitude(GLfloat x, GLfloat y, GLfloat z)
{
	return GL_SQRT(x * x + y * y + z * z);
}

void vec3_add(GLfloat aX, GLfloat aY, GLfloat aZ, GLfloat bX, GLfloat bY,
	      GLfloat bZ, GLfloat *restrict outX, GLfloat *restrict outY,
	      GLfloat *restrict outZ)
{
	*outX = aX + bX;
	*outY = aY + bY;
	*outZ = aZ + bZ;
}

void vec3_subtract(GLfloat aX, GLfloat aY, GLfloat aZ, GLfloat bX, GLfloat bY,
		   GLfloat bZ, GLfloat *restrict outX, GLfloat *restrict outY,
		   GLfloat *restrict outZ)
{
	*outX = aX - bX;
	*outY = aY - bY;
	*outZ = aZ - bZ;
}

/* ---------------------- */
/* Matrix Utility Functions */
/* ---------------------- */

/**
 * @brief Sets the matrix to the identity matrix.
 */
void mat4_identity(mat4 *restrict mat)
{
	/* Unrolled loop for performance */
	mat->data[0] = 1.0f;
	mat->data[4] = 0.0f;
	mat->data[8] = 0.0f;
	mat->data[12] = 0.0f;
	mat->data[1] = 0.0f;
	mat->data[5] = 1.0f;
	mat->data[9] = 0.0f;
	mat->data[13] = 0.0f;
	mat->data[2] = 0.0f;
	mat->data[6] = 0.0f;
	mat->data[10] = 1.0f;
	mat->data[14] = 0.0f;
	mat->data[3] = 0.0f;
	mat->data[7] = 0.0f;
	mat->data[11] = 0.0f;
	mat->data[15] = 1.0f;
}

/**
 * @brief Copies the source matrix to the destination matrix.
 */
void mat4_copy(mat4 *restrict dest, const mat4 *restrict src)
{
	/* Unrolled copy for performance */
	dest->data[0] = src->data[0];
	dest->data[1] = src->data[1];
	dest->data[2] = src->data[2];
	dest->data[3] = src->data[3];
	dest->data[4] = src->data[4];
	dest->data[5] = src->data[5];
	dest->data[6] = src->data[6];
	dest->data[7] = src->data[7];
	dest->data[8] = src->data[8];
	dest->data[9] = src->data[9];
	dest->data[10] = src->data[10];
	dest->data[11] = src->data[11];
	dest->data[12] = src->data[12];
	dest->data[13] = src->data[13];
	dest->data[14] = src->data[14];
	dest->data[15] = src->data[15];
}

/**
 * @brief Multiplies two matrices: result = a * b.
 */
void mat4_multiply(mat4 *restrict result, const mat4 *restrict a,
		   const mat4 *restrict b)
{
	const GLfloat *ap = a->data;
	const GLfloat *bp = b->data;

	/* Unrolled matrix multiplication for better compiler vectorization */
	const GLfloat a0 = ap[0], a4 = ap[4], a8 = ap[8], a12 = ap[12];
	const GLfloat a1 = ap[1], a5 = ap[5], a9 = ap[9], a13 = ap[13];
	const GLfloat a2 = ap[2], a6 = ap[6], a10 = ap[10], a14 = ap[14];
	const GLfloat a3 = ap[3], a7 = ap[7], a11 = ap[11], a15 = ap[15];

	const GLfloat b0 = bp[0], b4 = bp[4], b8 = bp[8], b12 = bp[12];
	const GLfloat b1 = bp[1], b5 = bp[5], b9 = bp[9], b13 = bp[13];
	const GLfloat b2 = bp[2], b6 = bp[6], b10 = bp[10], b14 = bp[14];
	const GLfloat b3 = bp[3], b7 = bp[7], b11 = bp[11], b15 = bp[15];

	result->data[0] = a0 * b0 + a4 * b1 + a8 * b2 + a12 * b3;
	result->data[1] = a1 * b0 + a5 * b1 + a9 * b2 + a13 * b3;
	result->data[2] = a2 * b0 + a6 * b1 + a10 * b2 + a14 * b3;
	result->data[3] = a3 * b0 + a7 * b1 + a11 * b2 + a15 * b3;

	result->data[4] = a0 * b4 + a4 * b5 + a8 * b6 + a12 * b7;
	result->data[5] = a1 * b4 + a5 * b5 + a9 * b6 + a13 * b7;
	result->data[6] = a2 * b4 + a6 * b5 + a10 * b6 + a14 * b7;
	result->data[7] = a3 * b4 + a7 * b5 + a11 * b6 + a15 * b7;

	result->data[8] = a0 * b8 + a4 * b9 + a8 * b10 + a12 * b11;
	result->data[9] = a1 * b8 + a5 * b9 + a9 * b10 + a13 * b11;
	result->data[10] = a2 * b8 + a6 * b9 + a10 * b10 + a14 * b11;
	result->data[11] = a3 * b8 + a7 * b9 + a11 * b10 + a15 * b11;

	result->data[12] = a0 * b12 + a4 * b13 + a8 * b14 + a12 * b15;
	result->data[13] = a1 * b12 + a5 * b13 + a9 * b14 + a13 * b15;
	result->data[14] = a2 * b12 + a6 * b13 + a10 * b14 + a14 * b15;
	result->data[15] = a3 * b12 + a7 * b13 + a11 * b14 + a15 * b15;
}

/**
 * @brief Inverts a 4x4 matrix.
 * @param mat The matrix to invert. The result overwrites this matrix.
 * @return 1 if the matrix was successfully inverted, 0 otherwise.
 */
int mat4_inverse(mat4 *restrict mat)
{
	GLfloat inv[16], det;
	int i;

	inv[0] = mat->data[5] * mat->data[10] * mat->data[15] -
		 mat->data[5] * mat->data[11] * mat->data[14] -
		 mat->data[9] * mat->data[6] * mat->data[15] +
		 mat->data[9] * mat->data[7] * mat->data[14] +
		 mat->data[13] * mat->data[6] * mat->data[11] -
		 mat->data[13] * mat->data[7] * mat->data[10];

	inv[4] = -mat->data[4] * mat->data[10] * mat->data[15] +
		 mat->data[4] * mat->data[11] * mat->data[14] +
		 mat->data[8] * mat->data[6] * mat->data[15] -
		 mat->data[8] * mat->data[7] * mat->data[14] -
		 mat->data[12] * mat->data[6] * mat->data[11] +
		 mat->data[12] * mat->data[7] * mat->data[10];

	inv[8] = mat->data[4] * mat->data[9] * mat->data[15] -
		 mat->data[4] * mat->data[11] * mat->data[13] -
		 mat->data[8] * mat->data[5] * mat->data[15] +
		 mat->data[8] * mat->data[7] * mat->data[13] +
		 mat->data[12] * mat->data[5] * mat->data[11] -
		 mat->data[12] * mat->data[7] * mat->data[9];

	inv[12] = -mat->data[4] * mat->data[9] * mat->data[14] +
		  mat->data[4] * mat->data[10] * mat->data[13] +
		  mat->data[8] * mat->data[5] * mat->data[14] -
		  mat->data[8] * mat->data[6] * mat->data[13] -
		  mat->data[12] * mat->data[5] * mat->data[10] +
		  mat->data[12] * mat->data[6] * mat->data[9];

	inv[1] = -mat->data[1] * mat->data[10] * mat->data[15] +
		 mat->data[1] * mat->data[11] * mat->data[14] +
		 mat->data[9] * mat->data[2] * mat->data[15] -
		 mat->data[9] * mat->data[3] * mat->data[14] -
		 mat->data[13] * mat->data[2] * mat->data[11] +
		 mat->data[13] * mat->data[3] * mat->data[10];

	inv[5] = mat->data[0] * mat->data[10] * mat->data[15] -
		 mat->data[0] * mat->data[11] * mat->data[14] -
		 mat->data[8] * mat->data[2] * mat->data[15] +
		 mat->data[8] * mat->data[3] * mat->data[14] +
		 mat->data[12] * mat->data[2] * mat->data[11] -
		 mat->data[12] * mat->data[3] * mat->data[10];

	inv[9] = -mat->data[0] * mat->data[9] * mat->data[15] +
		 mat->data[0] * mat->data[11] * mat->data[13] +
		 mat->data[8] * mat->data[1] * mat->data[15] -
		 mat->data[8] * mat->data[3] * mat->data[13] -
		 mat->data[12] * mat->data[1] * mat->data[11] +
		 mat->data[12] * mat->data[3] * mat->data[9];

	inv[13] = mat->data[0] * mat->data[9] * mat->data[14] -
		  mat->data[0] * mat->data[10] * mat->data[13] -
		  mat->data[4] * mat->data[1] * mat->data[14] +
		  mat->data[4] * mat->data[2] * mat->data[13] +
		  mat->data[12] * mat->data[1] * mat->data[10] -
		  mat->data[12] * mat->data[2] * mat->data[9];

	inv[2] = mat->data[1] * mat->data[6] * mat->data[15] -
		 mat->data[1] * mat->data[7] * mat->data[14] -
		 mat->data[5] * mat->data[2] * mat->data[15] +
		 mat->data[5] * mat->data[3] * mat->data[14] +
		 mat->data[13] * mat->data[2] * mat->data[7] -
		 mat->data[13] * mat->data[3] * mat->data[6];

	inv[6] = -mat->data[0] * mat->data[6] * mat->data[15] +
		 mat->data[0] * mat->data[7] * mat->data[14] +
		 mat->data[4] * mat->data[2] * mat->data[15] -
		 mat->data[4] * mat->data[3] * mat->data[14] -
		 mat->data[12] * mat->data[2] * mat->data[7] +
		 mat->data[12] * mat->data[3] * mat->data[6];

	inv[10] = mat->data[0] * mat->data[5] * mat->data[15] -
		  mat->data[0] * mat->data[7] * mat->data[13] -
		  mat->data[4] * mat->data[1] * mat->data[15] +
		  mat->data[4] * mat->data[3] * mat->data[13] +
		  mat->data[8] * mat->data[1] * mat->data[7] -
		  mat->data[8] * mat->data[3] * mat->data[5];

	inv[14] = -mat->data[0] * mat->data[5] * mat->data[14] +
		  mat->data[0] * mat->data[6] * mat->data[13] +
		  mat->data[4] * mat->data[1] * mat->data[14] -
		  mat->data[4] * mat->data[2] * mat->data[13] -
		  mat->data[8] * mat->data[1] * mat->data[6] +
		  mat->data[8] * mat->data[2] * mat->data[5];

	inv[3] = -mat->data[1] * mat->data[6] * mat->data[11] +
		 mat->data[1] * mat->data[7] * mat->data[10] +
		 mat->data[5] * mat->data[2] * mat->data[11] -
		 mat->data[5] * mat->data[3] * mat->data[10] -
		 mat->data[9] * mat->data[2] * mat->data[7] +
		 mat->data[9] * mat->data[3] * mat->data[6];

	inv[7] = mat->data[0] * mat->data[6] * mat->data[11] -
		 mat->data[0] * mat->data[7] * mat->data[10] -
		 mat->data[4] * mat->data[2] * mat->data[11] +
		 mat->data[4] * mat->data[3] * mat->data[10] +
		 mat->data[8] * mat->data[2] * mat->data[7] -
		 mat->data[8] * mat->data[3] * mat->data[6];

	inv[11] = -mat->data[0] * mat->data[5] * mat->data[11] +
		  mat->data[0] * mat->data[7] * mat->data[9] +
		  mat->data[4] * mat->data[1] * mat->data[11] -
		  mat->data[4] * mat->data[3] * mat->data[9] -
		  mat->data[8] * mat->data[1] * mat->data[7] +
		  mat->data[8] * mat->data[3] * mat->data[5];

	inv[15] = mat->data[0] * inv[0] + mat->data[1] * inv[4] +
		  mat->data[2] * inv[8] + mat->data[3] * inv[12];

	/* Compute determinant */
	det = mat->data[0] * inv[0] + mat->data[1] * inv[4] +
	      mat->data[2] * inv[8] + mat->data[3] * inv[12];

	if (det == 0.0f) {
		LOG_WARN(
			"mat4_inverse: Matrix is singular and cannot be inverted.");
		return 0;
	}

	det = 1.0f / det;

	/* Multiply the inverse matrix by the reciprocal of the determinant */
	for (i = 0; i < 16; i++) {
		mat->data[i] = inv[i] * det;
	}

	return 1;
}

/**
 * @brief Transposes a 4x4 matrix.
 */
void mat4_transpose(mat4 *restrict mat)
{
	/* Unrolled transpose for performance */
	GLfloat temp;

	temp = mat->data[1];
	mat->data[1] = mat->data[4];
	mat->data[4] = temp;

	temp = mat->data[2];
	mat->data[2] = mat->data[8];
	mat->data[8] = temp;

	temp = mat->data[3];
	mat->data[3] = mat->data[12];
	mat->data[12] = temp;

	temp = mat->data[6];
	mat->data[6] = mat->data[9];
	mat->data[9] = temp;

	temp = mat->data[7];
	mat->data[7] = mat->data[13];
	mat->data[13] = temp;

	temp = mat->data[11];
	mat->data[11] = mat->data[14];
	mat->data[14] = temp;
}

/**
 * @brief Applies a translation to the matrix.
 */
void mat4_translate(mat4 *restrict mat, GLfloat x, GLfloat y, GLfloat z)
{
	/* Unrolled translation application */
	mat->data[12] += mat->data[0] * x + mat->data[4] * y + mat->data[8] * z;
	mat->data[13] += mat->data[1] * x + mat->data[5] * y + mat->data[9] * z;
	mat->data[14] +=
		mat->data[2] * x + mat->data[6] * y + mat->data[10] * z;
	mat->data[15] +=
		mat->data[3] * x + mat->data[7] * y + mat->data[11] * z;
}

/**
 * @brief Applies a scaling transformation to the matrix.
 */
void mat4_scale(mat4 *restrict mat, GLfloat x, GLfloat y, GLfloat z)
{
	/* Unrolled scaling application */
	mat->data[0] *= x;
	mat->data[1] *= x;
	mat->data[2] *= x;
	mat->data[3] *= x;

	mat->data[4] *= y;
	mat->data[5] *= y;
	mat->data[6] *= y;
	mat->data[7] *= y;

	mat->data[8] *= z;
	mat->data[9] *= z;
	mat->data[10] *= z;
	mat->data[11] *= z;
}

/**
 * @brief Applies uniform scaling to the matrix.
 */
void mat4_scale_uniform(mat4 *restrict mat, GLfloat scale)
{
	mat4_scale(mat, scale, scale, scale);
}

/**
 * @brief Applies a rotation around the X-axis to the matrix.
 */
void mat4_rotate_x(mat4 *restrict mat, GLfloat angle)
{
	GLfloat rad = DEG2RAD(angle);
	GLfloat c = cosf(rad);
	GLfloat s = sinf(rad);

	/* Create rotation matrix around X-axis */
	mat4 rot;
	mat4_identity(&rot);
	rot.data[5] = c;
	rot.data[6] = s;
	rot.data[9] = -s;
	rot.data[10] = c;

	/* Multiply mat by rot */
	mat4 temp;
	mat4_multiply(&temp, mat, &rot);
	mat4_copy(mat, &temp);
}

/**
 * @brief Applies a rotation around the Y-axis to the matrix.
 */
void mat4_rotate_y(mat4 *restrict mat, GLfloat angle)
{
	GLfloat rad = DEG2RAD(angle);
	GLfloat c = cosf(rad);
	GLfloat s = sinf(rad);

	/* Create rotation matrix around Y-axis */
	mat4 rot;
	mat4_identity(&rot);
	rot.data[0] = c;
	rot.data[2] = -s;
	rot.data[8] = s;
	rot.data[10] = c;

	/* Multiply mat by rot */
	mat4 temp;
	mat4_multiply(&temp, mat, &rot);
	mat4_copy(mat, &temp);
}

/**
 * @brief Applies a rotation around the Z-axis to the matrix.
 */
void mat4_rotate_z(mat4 *restrict mat, GLfloat angle)
{
	GLfloat rad = DEG2RAD(angle);
	GLfloat c = cosf(rad);
	GLfloat s = sinf(rad);

	/* Create rotation matrix around Z-axis */
	mat4 rot;
	mat4_identity(&rot);
	rot.data[0] = c;
	rot.data[1] = s;
	rot.data[4] = -s;
	rot.data[5] = c;

	/* Multiply mat by rot */
	mat4 temp;
	mat4_multiply(&temp, mat, &rot);
	mat4_copy(mat, &temp);
}

/**
 * @brief Applies a rotation around an arbitrary axis to the matrix.
 */
void mat4_rotate_axis(mat4 *restrict mat, GLfloat angle, GLfloat x, GLfloat y,
		      GLfloat z)
{
	GLfloat rad = DEG2RAD(angle);
	GLfloat c = cosf(rad);
	GLfloat s = sinf(rad);
	GLfloat one_minus_c = 1.0f - c;

	/* Normalize the axis vector */
	GLfloat length = GL_SQRT(x * x + y * y + z * z);
	if (length == 0.0f) {
		LOG_WARN("mat4_rotate_axis: Zero-length axis vector.");
		return;
	}
	x /= length;
	y /= length;
	z /= length;

	/* Create rotation matrix around arbitrary axis */
	mat4 rot;
	mat4_identity(&rot);
	rot.data[0] = c + x * x * one_minus_c;
	rot.data[1] = y * x * one_minus_c + z * s;
	rot.data[2] = z * x * one_minus_c - y * s;
	rot.data[4] = x * y * one_minus_c - z * s;
	rot.data[5] = c + y * y * one_minus_c;
	rot.data[6] = z * y * one_minus_c + x * s;
	rot.data[8] = x * z * one_minus_c + y * s;
	rot.data[9] = y * z * one_minus_c - x * s;
	rot.data[10] = c + z * z * one_minus_c;

	/* Multiply mat by rot */
	mat4 temp;
	mat4_multiply(&temp, mat, &rot);
	mat4_copy(mat, &temp);
}

/**
 * @brief Creates a lookAt view matrix.
 */
void mat4_look_at(mat4 *restrict mat, GLfloat eyeX, GLfloat eyeY, GLfloat eyeZ,
		  GLfloat centerX, GLfloat centerY, GLfloat centerZ,
		  GLfloat upX, GLfloat upY, GLfloat upZ)
{
	GLfloat fx = centerX - eyeX;
	GLfloat fy = centerY - eyeY;
	GLfloat fz = centerZ - eyeZ;

	/* Normalize f */
	vec3_normalize(&fx, &fy, &fz);

	/* Normalize up */
	vec3_normalize(&upX, &upY, &upZ);

	/* s = f x up */
	GLfloat sx, sy, sz;
	vec3_cross(fx, fy, fz, upX, upY, upZ, &sx, &sy, &sz);
	vec3_normalize(&sx, &sy, &sz);

	/* u = s x f */
	GLfloat ux, uy, uz;
	vec3_cross(sx, sy, sz, fx, fy, fz, &ux, &uy, &uz);

	/* Create lookAt matrix */
	mat4_identity(mat);
	mat->data[0] = sx;
	mat->data[1] = ux;
	mat->data[2] = -fx;
	mat->data[4] = sy;
	mat->data[5] = uy;
	mat->data[6] = -fy;
	mat->data[8] = sz;
	mat->data[9] = uz;
	mat->data[10] = -fz;

	/* Translation */
	mat->data[12] = -sx * eyeX - sy * eyeY - sz * eyeZ;
	mat->data[13] = -ux * eyeX - uy * eyeY - uz * eyeZ;
	mat->data[14] = fx * eyeX + fy * eyeY + fz * eyeZ;
}

/**
 * @brief Creates a perspective projection matrix.
 */
void mat4_perspective(mat4 *restrict mat, GLfloat fovy, GLfloat aspect,
		      GLfloat zNear, GLfloat zFar)
{
	GLfloat f = 1.0f / tanf(DEG2RAD(fovy) / 2.0f);

	/* Create perspective matrix */
	mat4_identity(mat);
	mat->data[0] = f / aspect;
	mat->data[5] = f;
	mat->data[10] = (zFar + zNear) / (zNear - zFar);
	mat->data[11] = -1.0f;
	mat->data[14] = (2.0f * zFar * zNear) / (zNear - zFar);
	mat->data[15] = 0.0f;
}

/**
 * @brief Creates a frustum projection matrix.
 */
void mat4_frustum(mat4 *restrict mat, GLfloat left, GLfloat right,
		  GLfloat bottom, GLfloat top, GLfloat nearVal, GLfloat farVal)
{
	GLfloat rl = right - left;
	GLfloat tb = top - bottom;
	GLfloat fn = farVal - nearVal;

	mat4_identity(mat);
	mat->data[0] = (2.0f * nearVal) / rl;
	mat->data[5] = (2.0f * nearVal) / tb;
	mat->data[8] = (right + left) / rl;
	mat->data[9] = (top + bottom) / tb;
	mat->data[10] = -(farVal + nearVal) / fn;
	mat->data[11] = -1.0f;
	mat->data[14] = -(2.0f * farVal * nearVal) / fn;
	mat->data[15] = 0.0f;
}

/**
 * @brief Creates an orthographic projection matrix.
 */
void mat4_orthographic(mat4 *restrict mat, GLfloat left, GLfloat right,
		       GLfloat bottom, GLfloat top, GLfloat nearVal,
		       GLfloat farVal)
{
	GLfloat rl = right - left;
	GLfloat tb = top - bottom;
	GLfloat fn = farVal - nearVal;

	mat4_identity(mat);
	mat->data[0] = 2.0f / rl;
	mat->data[5] = 2.0f / tb;
	mat->data[10] = -2.0f / fn;
	mat->data[12] = -(right + left) / rl;
	mat->data[13] = -(top + bottom) / tb;
	mat->data[14] = -(farVal + nearVal) / fn;
}

/**
 * @brief Performs a perspective divide on the matrix, converting from clip
 * space to normalized device coordinates.
 */
void mat4_perspective_divide(mat4 *restrict mat)
{
	if (mat->data[15] == 0.0f) {
		LOG_WARN(
			"mat4_perspective_divide: Division by zero encountered.");
		return;
	}

	/* Unrolled perspective divide for performance */
	mat->data[0] /= mat->data[15];
	mat->data[1] /= mat->data[15];
	mat->data[2] /= mat->data[15];
	mat->data[3] /= mat->data[15];
	mat->data[4] /= mat->data[15];
	mat->data[5] /= mat->data[15];
	mat->data[6] /= mat->data[15];
	mat->data[7] /= mat->data[15];
	mat->data[8] /= mat->data[15];
	mat->data[9] /= mat->data[15];
	mat->data[10] /= mat->data[15];
	mat->data[11] /= mat->data[15];
	mat->data[12] /= mat->data[15];
	mat->data[13] /= mat->data[15];
	mat->data[14] /= mat->data[15];
	mat->data[15] /= mat->data[15];
}

/* ---------------------- */
/* Quaternion Utility Functions */
/* ---------------------- */

/**
 * @brief Normalizes a quaternion.
 */
void quat_normalize(Quaternion *restrict q)
{
	GLfloat mag =
		GL_SQRT(q->w * q->w + q->x * q->x + q->y * q->y + q->z * q->z);
	if (mag > 0.0f) {
		q->w /= mag;
		q->x /= mag;
		q->y /= mag;
		q->z /= mag;
	} else {
		LOG_WARN("quat_normalize: Zero-length quaternion encountered.");
	}
}

/**
 * @brief Creates a quaternion from an axis and angle.
 */
void quat_from_axis_angle(Quaternion *restrict q, GLfloat angle, GLfloat x,
			  GLfloat y, GLfloat z)
{
	GLfloat rad = DEG2RAD(angle);
	GLfloat half_rad = rad / 2.0f;
	GLfloat s = sinf(half_rad);

	q->w = cosf(half_rad);
	q->x = x * s;
	q->y = y * s;
	q->z = z * s;
	quat_normalize(q);
}

/**
 * @brief Converts a quaternion to a rotation matrix.
 */
void quat_to_mat4(const Quaternion *restrict q, mat4 *restrict mat)
{
	mat4_identity(mat);

	mat->data[0] = 1.0f - 2.0f * (q->y * q->y + q->z * q->z);
	mat->data[1] = 2.0f * (q->x * q->y + q->z * q->w);
	mat->data[2] = 2.0f * (q->x * q->z - q->y * q->w);

	mat->data[4] = 2.0f * (q->x * q->y - q->z * q->w);
	mat->data[5] = 1.0f - 2.0f * (q->x * q->x + q->z * q->z);
	mat->data[6] = 2.0f * (q->y * q->z + q->x * q->w);

	mat->data[8] = 2.0f * (q->x * q->z + q->y * q->w);
	mat->data[9] = 2.0f * (q->y * q->z - q->x * q->w);
	mat->data[10] = 1.0f - 2.0f * (q->x * q->x + q->y * q->y);
}

void mat4_transform_vec4(const mat4 *restrict mat, const GLfloat in[restrict 4],
			 GLfloat out[restrict 4])
{
	out[0] = mat->data[0] * in[0] + mat->data[4] * in[1] +
		 mat->data[8] * in[2] + mat->data[12] * in[3];
	out[1] = mat->data[1] * in[0] + mat->data[5] * in[1] +
		 mat->data[9] * in[2] + mat->data[13] * in[3];
	out[2] = mat->data[2] * in[0] + mat->data[6] * in[1] +
		 mat->data[10] * in[2] + mat->data[14] * in[3];
	out[3] = mat->data[3] * in[0] + mat->data[7] * in[1] +
		 mat->data[11] * in[2] + mat->data[15] * in[3];
}

/* ---------------------- */
/* Unit Testing (Optional) */
/* ---------------------- */

/* Implement unit tests here or in a separate test file */

/* ---------------------- */
/* End of Matrix Utility Functions */
/* ---------------------- */
