#ifndef C11_OPT_H
#define C11_OPT_H

#include <math.h>

/* Generic math wrappers choosing float or double variants */
#define GL_SQRT(x) _Generic((x), float: sqrtf, double: sqrt)((x))
#define GL_POW(x, y) _Generic((x), float: powf, double: pow)((x), (y))

/* Additional helpers */
#define GL_ABS(x) _Generic((x), float: fabsf, double: fabs)((x))
#define GL_MIN(x, y) _Generic((x), float: fminf, double: fmin)((x), (y))
#define GL_MAX(x, y) _Generic((x), float: fmaxf, double: fmax)((x), (y))
#define GL_CLAMP(x, lo, hi) (GL_MIN(GL_MAX((x), (lo)), (hi)))
#define GL_LERP(a, b, t) ((a) + ((b) - (a)) * (t))

#endif /* C11_OPT_H */
