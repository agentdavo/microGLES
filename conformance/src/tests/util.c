#include "util.h"
#include "gl_logger.h"
#include "gl_utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int write_rgba(const char *filename, const unsigned char *data, int width,
	       int height)
{
	FILE *f = fopen(filename, "w");
	if (!f) {
		LOG_ERROR("Failed to open %s", filename);
		return 0;
	}
	fprintf(f, "%d %d\n", width, height);
	for (int i = 0; i < width * height; ++i) {
		const unsigned char *p = data + i * 4;
		fprintf(f, "%u %u %u %u\n", p[0], p[1], p[2], p[3]);
	}
	fclose(f);
	return 1;
}

int read_rgba(const char *filename, unsigned char **data, int *width,
	      int *height)
{
	FILE *f = fopen(filename, "r");
	if (!f)
		return 0;
	int w = 0, h = 0;
	if (fscanf(f, "%d %d", &w, &h) != 2) {
		fclose(f);
		return 0;
	}
	unsigned char *buf = (unsigned char *)tracked_malloc((size_t)w * h * 4);
	for (int i = 0; i < w * h; ++i) {
		unsigned r, g, b, a;
		if (fscanf(f, "%u %u %u %u", &r, &g, &b, &a) != 4) {
			tracked_free(buf, (size_t)w * h * 4);
			fclose(f);
			return 0;
		}
		buf[i * 4 + 0] = (unsigned char)r;
		buf[i * 4 + 1] = (unsigned char)g;
		buf[i * 4 + 2] = (unsigned char)b;
		buf[i * 4 + 3] = (unsigned char)a;
	}
	fclose(f);
	*data = buf;
	if (width)
		*width = w;
	if (height)
		*height = h;
	return 1;
}

int compare_rgba(const char *expected, const char *actual)
{
	unsigned char *edata = NULL, *adata = NULL;
	int ew = 0, eh = 0, aw = 0, ah = 0;
	if (!read_rgba(expected, &edata, &ew, &eh))
		return 0;
	if (!read_rgba(actual, &adata, &aw, &ah)) {
		tracked_free(edata, (size_t)ew * eh * 4);
		return 0;
	}
	int pass = (ew == aw && eh == ah);
	if (pass) {
		for (int i = 0; i < ew * eh * 4; ++i) {
			if (edata[i] != adata[i]) {
				pass = 0;
				break;
			}
		}
	}
	tracked_free(edata, (size_t)ew * eh * 4);
	tracked_free(adata, (size_t)aw * ah * 4);
	return pass;
}
