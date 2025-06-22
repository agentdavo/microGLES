#ifndef C11_OPT_H
#define C11_OPT_H

#include <math.h>

/* Generic math wrappers choosing float or double variants */
#define GL_SQRT(x) _Generic((x), float: sqrtf, double: sqrt)((x))
#define GL_POW(x, y) _Generic((x), float: powf, double: pow)((x), (y))

#endif /* C11_OPT_H */
