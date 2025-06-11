#ifndef CONFORMANCE_UTIL_H
#define CONFORMANCE_UTIL_H

#include <stddef.h>
#include <GLES/gl.h>

int write_rgba(const char *filename, const unsigned char *data, int width,
	       int height);
int read_rgba(const char *filename, unsigned char **data, int *width,
	      int *height);
int compare_rgba(const char *expected, const char *actual);

#endif /* CONFORMANCE_UTIL_H */
