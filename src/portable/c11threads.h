#ifndef C11THREADS_H
#define C11THREADS_H

#if defined(__STDC_NO_THREADS__)
#include "tinycthread.h"
#elif defined(__has_include)
#if __has_include(<threads.h>)
#include <threads.h>
#else
#include "tinycthread.h"
#endif
#else
#include <threads.h>
#endif

#endif /* C11THREADS_H */
