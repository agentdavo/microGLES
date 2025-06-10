/* matrix_utils.c */

#include "matrix_utils.h"
#include "logger.h" // Ensure logger is initialized before using
#include <math.h>

/* Define Pi for angle conversions */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Helper Macros for Degrees to Radians Conversion */
#define DEG2RAD(angle) ((angle) * (M_PI / 180.0f))

/* ---------------------- */
/* Vector Utility Functions */
/* ---------------------- */

void vec3_normalize(GLfloat *x, GLfloat *y, GLfloat *z) {
  GLfloat length = sqrtf((*x) * (*x) + (*y) * (*y) + (*z) * (*z));
  if (length > 0.0f) {
    *x /= length;
    *y /= length;
    *z /= length;
  } else {
    LOG_WARN("vec3_normalize: Zero-length vector encountered.");
  }
}

void vec3_cross(GLfloat aX, GLfloat aY, GLfloat aZ, GLfloat bX, GLfloat bY,
                GLfloat bZ, GLfloat *outX, GLfloat *outY, GLfloat *outZ) {
  *outX = aY * bZ - aZ * bY;
  *outY = aZ * bX - aX * bZ;
  *outZ = aX * bY - aY * bX;
}

GLfloat vec3_dot(GLfloat aX, GLfloat aY, GLfloat aZ, GLfloat bX, GLfloat bY,
                 GLfloat bZ) {
  return aX * bX + aY * bY + aZ * bZ;
}

GLfloat vec3_magnitude(GLfloat x, GLfloat y, GLfloat z) {
  return sqrtf(x * x + y * y + z * z);
}

void vec3_add(GLfloat aX, GLfloat aY, GLfloat aZ, GLfloat bX, GLfloat bY,
              GLfloat bZ, GLfloat *outX, GLfloat *outY, GLfloat *outZ) {
  *outX = aX + bX;
  *outY = aY + bY;
  *outZ = aZ + bZ;
}

void vec3_subtract(GLfloat aX, GLfloat aY, GLfloat aZ, GLfloat bX, GLfloat bY,
                   GLfloat bZ, GLfloat *outX, GLfloat *outY, GLfloat *outZ) {
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
void mat4_identity(mat4 *mat) {
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
void mat4_copy(mat4 *dest, const mat4 *src) {
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
void mat4_multiply(mat4 *result, const mat4 *a, const mat4 *b) {
  const GLfloat *ap = a->data;
  const GLfloat *bp = b->data;
  for (int i = 0; i < 4; ++i) {
    const GLfloat ai0 = ap[i];
    const GLfloat ai1 = ap[i + 4];
    const GLfloat ai2 = ap[i + 8];
    const GLfloat ai3 = ap[i + 12];

    result->data[i] = ai0 * bp[0] + ai1 * bp[1] + ai2 * bp[2] + ai3 * bp[3];
    result->data[i + 4] = ai0 * bp[4] + ai1 * bp[5] + ai2 * bp[6] + ai3 * bp[7];
    result->data[i + 8] =
        ai0 * bp[8] + ai1 * bp[9] + ai2 * bp[10] + ai3 * bp[11];
    result->data[i + 12] =
        ai0 * bp[12] + ai1 * bp[13] + ai2 * bp[14] + ai3 * bp[15];
  }
}

/**
 * @brief Inverts a 4x4 matrix.
 * @param mat The matrix to invert. The result overwrites this matrix.
 * @return 1 if the matrix was successfully inverted, 0 otherwise.
 */
int mat4_inverse(mat4 *mat) {
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
  det = mat->data[0] * inv[0] + mat->data[1] * inv[4] + mat->data[2] * inv[8] +
        mat->data[3] * inv[12];

  if (det == 0.0f) {
    LOG_WARN("mat4_inverse: Matrix is singular and cannot be inverted.");
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
void mat4_transpose(mat4 *mat) {
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
void mat4_translate(mat4 *mat, GLfloat x, GLfloat y, GLfloat z) {
  /* Unrolled translation application */
  mat->data[12] += mat->data[0] * x + mat->data[4] * y + mat->data[8] * z;
  mat->data[13] += mat->data[1] * x + mat->data[5] * y + mat->data[9] * z;
  mat->data[14] += mat->data[2] * x + mat->data[6] * y + mat->data[10] * z;
  mat->data[15] += mat->data[3] * x + mat->data[7] * y + mat->data[11] * z;
}

/**
 * @brief Applies a scaling transformation to the matrix.
 */
void mat4_scale(mat4 *mat, GLfloat x, GLfloat y, GLfloat z) {
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
void mat4_scale_uniform(mat4 *mat, GLfloat scale) {
  mat4_scale(mat, scale, scale, scale);
}

/**
 * @brief Applies a rotation around the X-axis to the matrix.
 */
void mat4_rotate_x(mat4 *mat, GLfloat angle) {
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
void mat4_rotate_y(mat4 *mat, GLfloat angle) {
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
void mat4_rotate_z(mat4 *mat, GLfloat angle) {
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
void mat4_rotate_axis(mat4 *mat, GLfloat angle, GLfloat x, GLfloat y,
                      GLfloat z) {
  GLfloat rad = DEG2RAD(angle);
  GLfloat c = cosf(rad);
  GLfloat s = sinf(rad);
  GLfloat one_minus_c = 1.0f - c;

  /* Normalize the axis vector */
  GLfloat length = sqrtf(x * x + y * y + z * z);
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
void mat4_look_at(mat4 *mat, GLfloat eyeX, GLfloat eyeY, GLfloat eyeZ,
                  GLfloat centerX, GLfloat centerY, GLfloat centerZ,
                  GLfloat upX, GLfloat upY, GLfloat upZ) {
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
void mat4_perspective(mat4 *mat, GLfloat fovy, GLfloat aspect, GLfloat zNear,
                      GLfloat zFar) {
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
void mat4_frustum(mat4 *mat, GLfloat left, GLfloat right, GLfloat bottom,
                  GLfloat top, GLfloat nearVal, GLfloat farVal) {
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
void mat4_orthographic(mat4 *mat, GLfloat left, GLfloat right, GLfloat bottom,
                       GLfloat top, GLfloat nearVal, GLfloat farVal) {
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
void mat4_perspective_divide(mat4 *mat) {
  if (mat->data[15] == 0.0f) {
    LOG_WARN("mat4_perspective_divide: Division by zero encountered.");
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
void quat_normalize(Quaternion *q) {
  GLfloat mag = sqrtf(q->w * q->w + q->x * q->x + q->y * q->y + q->z * q->z);
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
void quat_from_axis_angle(Quaternion *q, GLfloat angle, GLfloat x, GLfloat y,
                          GLfloat z) {
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
void quat_to_mat4(const Quaternion *q, mat4 *mat) {
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

void mat4_transform_vec4(const mat4 *mat, const GLfloat in[4], GLfloat out[4]) {
  out[0] = mat->data[0] * in[0] + mat->data[4] * in[1] + mat->data[8] * in[2] +
           mat->data[12] * in[3];
  out[1] = mat->data[1] * in[0] + mat->data[5] * in[1] + mat->data[9] * in[2] +
           mat->data[13] * in[3];
  out[2] = mat->data[2] * in[0] + mat->data[6] * in[1] + mat->data[10] * in[2] +
           mat->data[14] * in[3];
  out[3] = mat->data[3] * in[0] + mat->data[7] * in[1] + mat->data[11] * in[2] +
           mat->data[15] * in[3];
}

/* ---------------------- */
/* Unit Testing (Optional) */
/* ---------------------- */

/* Implement unit tests here or in a separate test file */

/* ---------------------- */
/* End of Matrix Utility Functions */
/* ---------------------- */
