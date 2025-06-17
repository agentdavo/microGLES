#define _GNU_SOURCE
#include "ktx_loader.h"
#include "gl_context.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

static const uint8_t KTX_IDENTIFIER[12] = {
	0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A
};

typedef struct {
	uint8_t identifier[12];
	uint32_t endianness;
	uint32_t glType;
	uint32_t glTypeSize;
	uint32_t glFormat;
	uint32_t glInternalFormat;
	uint32_t glBaseInternalFormat;
	uint32_t pixelWidth;
	uint32_t pixelHeight;
	uint32_t pixelDepth;
	uint32_t numberOfArrayElements;
	uint32_t numberOfFaces;
	uint32_t numberOfMipmapLevels;
	uint32_t bytesOfKeyValueData;
} KTXHeader;

#define KTX_ENDIAN_REF 0x04030201
#define KTX_ENDIAN_REVERSE 0x01020304
#define ETC1_BLOCK_SIZE 8
#define ETC1_BLOCK_DIM 4

static const GLenum CUBE_MAP_FACES[] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X_OES, GL_TEXTURE_CUBE_MAP_NEGATIVE_X_OES,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y_OES, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_OES,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z_OES, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_OES
};

static inline uint32_t swap_uint32(uint32_t val)
{
	return ((val & 0xFF) << 24) | ((val & 0xFF00) << 8) |
	       ((val & 0xFF0000) >> 8) | ((val & 0xFF000000) >> 24);
}

static bool read_header(FILE *file, KTXHeader *header, bool *needs_swap)
{
	if (fread(header, sizeof(KTXHeader), 1, file) != 1)
		return false;
	if (memcmp(header->identifier, KTX_IDENTIFIER, 12) != 0)
		return false;
	*needs_swap = (header->endianness == KTX_ENDIAN_REVERSE);
	if (*needs_swap) {
		header->endianness = swap_uint32(header->endianness);
		header->glType = swap_uint32(header->glType);
		header->glTypeSize = swap_uint32(header->glTypeSize);
		header->glFormat = swap_uint32(header->glFormat);
		header->glInternalFormat =
			swap_uint32(header->glInternalFormat);
		header->glBaseInternalFormat =
			swap_uint32(header->glBaseInternalFormat);
		header->pixelWidth = swap_uint32(header->pixelWidth);
		header->pixelHeight = swap_uint32(header->pixelHeight);
		header->pixelDepth = swap_uint32(header->pixelDepth);
		header->numberOfArrayElements =
			swap_uint32(header->numberOfArrayElements);
		header->numberOfFaces = swap_uint32(header->numberOfFaces);
		header->numberOfMipmapLevels =
			swap_uint32(header->numberOfMipmapLevels);
		header->bytesOfKeyValueData =
			swap_uint32(header->bytesOfKeyValueData);
	}
	return header->endianness == KTX_ENDIAN_REF;
}

static bool parse_ascii_file(const char *filename, uint8_t **data, size_t *size)
{
	FILE *f = fopen(filename, "r");
	if (!f)
		return false;
	fseek(f, 0, SEEK_END);
	long len = ftell(f);
	if (len <= 0) {
		fclose(f);
		return false;
	}
	rewind(f);
	char *text = malloc(len + 1);
	if (!text) {
		fclose(f);
		return false;
	}
	if (fread(text, 1, len, f) != (size_t)len) {
		free(text);
		fclose(f);
		return false;
	}
	text[len] = '\0';
	fclose(f);
	size_t cap = len;
	uint8_t *buf = malloc(cap);
	size_t count = 0;
	char *p = text;
	while (*p) {
		while (*p && (isspace((unsigned char)*p) || *p == ',' ||
			      *p == '{' || *p == '}'))
			p++;
		if (!*p)
			break;
		long val = strtol(p, &p, 0);
		if (val < 0 || val > 255) {
			free(buf);
			free(text);
			return false;
		}
		if (count >= cap) {
			cap *= 2;
			uint8_t *tmp = realloc(buf, cap);
			if (!tmp) {
				free(buf);
				free(text);
				return false;
			}
			buf = tmp;
		}
		buf[count++] = (uint8_t)val;
	}
	free(text);
	*data = buf;
	*size = count;
	return count >= sizeof(KTXHeader);
}

static bool validate_header(const KTXHeader *h)
{
	if (h->pixelHeight == 0 && h->pixelDepth == 0)
		return false;
	if (h->pixelDepth != 0)
		return false;
	if (h->numberOfArrayElements != 0)
		return false;
	if (h->numberOfFaces != 1 && h->numberOfFaces != 6)
		return false;
	if (h->numberOfFaces == 6 && h->pixelWidth != h->pixelHeight)
		return false;
	if (h->numberOfMipmapLevels == 0 && h->glType == 0)
		return false;
	if (h->glInternalFormat == GL_ETC1_RGB8_OES) {
		if (h->glType != 0 || h->glFormat != 0 || h->glTypeSize != 1 ||
		    h->glBaseInternalFormat != GL_RGB)
			return false;
		uint32_t bw =
			(h->pixelWidth + ETC1_BLOCK_DIM - 1) / ETC1_BLOCK_DIM;
		uint32_t bh =
			(h->pixelHeight + ETC1_BLOCK_DIM - 1) / ETC1_BLOCK_DIM;
		if (bw == 0 || bh == 0)
			return false;
	}
	return true;
}

static bool skip_key_value_data(FILE *file, uint32_t bytesOfKeyValueData,
				bool needs_swap)
{
	uint32_t remaining = bytesOfKeyValueData;
	while (remaining > 0) {
		uint32_t keyAndValueByteSize;
		if (fread(&keyAndValueByteSize, sizeof(uint32_t), 1, file) != 1)
			return false;
		if (needs_swap)
			keyAndValueByteSize = swap_uint32(keyAndValueByteSize);
		uint32_t padding = 3 - ((keyAndValueByteSize + 3) % 4);
		if (keyAndValueByteSize + padding > remaining)
			return false;
		if (fseek(file, keyAndValueByteSize + padding, SEEK_CUR) != 0)
			return false;
		remaining -= (4 + keyAndValueByteSize + padding);
	}
	return remaining == 0;
}

static void convert_data_endianness(void *data, size_t size,
				    uint32_t glTypeSize)
{
	if (glTypeSize <= 1)
		return;
	uint8_t *bytes = data;
	if (glTypeSize == 2) {
		for (size_t i = 0; i < size; i += 2) {
			uint8_t tmp = bytes[i];
			bytes[i] = bytes[i + 1];
			bytes[i + 1] = tmp;
		}
	} else if (glTypeSize == 4) {
		for (size_t i = 0; i < size; i += 4) {
			uint8_t tmp = bytes[i];
			bytes[i] = bytes[i + 3];
			bytes[i + 3] = tmp;
			tmp = bytes[i + 1];
			bytes[i + 1] = bytes[i + 2];
			bytes[i + 2] = tmp;
		}
	}
}

static KTXError load_from_stream(FILE *file, GLuint *texture_id)
{
	KTXHeader h;
	bool swap;
	if (!read_header(file, &h, &swap))
		return KTX_INVALID_HEADER;
	if (!validate_header(&h))
		return KTX_UNSUPPORTED_TEXTURE_TYPE;
	if (h.glType == 0)
		return KTX_UNSUPPORTED_TEXTURE_TYPE;
	if (!skip_key_value_data(file, h.bytesOfKeyValueData, swap))
		return KTX_INVALID_METADATA;
	glGenTextures(1, texture_id);
	if (*texture_id == 0)
		return KTX_OPENGL_ERROR;
	GLenum target = (h.numberOfFaces == 6) ? GL_TEXTURE_CUBE_MAP_OES :
						 GL_TEXTURE_2D;
	glBindTexture(target, *texture_id);
	uint32_t levels = h.numberOfMipmapLevels ? h.numberOfMipmapLevels : 1;

	for (uint32_t level = 0; level < levels; ++level) {
		uint32_t imageSize;
		if (fread(&imageSize, sizeof(uint32_t), 1, file) != 1) {
			glDeleteTextures(1, texture_id);
			fclose(file);
			return KTX_INVALID_DATA;
		}
		if (swap)
			imageSize = swap_uint32(imageSize);
		uint32_t width = h.pixelWidth >> level;
		uint32_t height = h.pixelHeight >> level;
		width = width ? width : 1;
		height = height ? height : 1;
		uint32_t face_count = h.numberOfFaces;
		uint32_t face_image_size =
			(face_count == 6 && h.numberOfArrayElements == 0) ?
				imageSize :
				imageSize / face_count;
		for (uint32_t face = 0; face < face_count; ++face) {
			uint8_t *data = malloc(face_image_size);
			if (!data || fread(data, 1, face_image_size, file) !=
					     face_image_size) {
				free(data);
				glDeleteTextures(1, texture_id);
				return KTX_INVALID_DATA;
			}
			if (swap)
				convert_data_endianness(data, face_image_size,
							h.glTypeSize);
			GLenum face_target = (face_count == 6) ?
						     CUBE_MAP_FACES[face] :
						     GL_TEXTURE_2D;
			glTexImage2D(face_target, level, h.glBaseInternalFormat,
				     width, height, 0, h.glFormat, h.glType,
				     data);
			free(data);
			GLenum err = glGetError();
			if (err != GL_NO_ERROR) {
				glDeleteTextures(1, texture_id);
				return KTX_OPENGL_ERROR;
			}
			if (face_count == 6 && h.numberOfArrayElements == 0) {
				uint32_t cube_padding =
					3 - ((face_image_size + 3) % 4);
				if (cube_padding > 0 &&
				    fseek(file, cube_padding, SEEK_CUR) != 0) {
					glDeleteTextures(1, texture_id);
					return KTX_INVALID_DATA;
				}
			}
		}
		uint32_t mip_padding = 3 - ((imageSize + 3) % 4);
		if (mip_padding > 0 &&
		    fseek(file, mip_padding, SEEK_CUR) != 0) {
			glDeleteTextures(1, texture_id);
			return KTX_INVALID_DATA;
		}
	}
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	return KTX_SUCCESS;
}

KTXError load_ktx_texture(const char *filename, GLuint *texture_id)
{
	FILE *file = fopen(filename, "rb");
	if (!file)
		return KTX_FILE_NOT_FOUND;
	uint8_t sig[12];
	bool binary = fread(sig, 1, 12, file) == 12 &&
		      memcmp(sig, KTX_IDENTIFIER, 12) == 0;
	fseek(file, 0, SEEK_SET);
	KTXError res;
	if (binary) {
		res = load_from_stream(file, texture_id);
		fclose(file);
		return res;
	}
	fclose(file);
	uint8_t *buf;
	size_t size;
	if (!parse_ascii_file(filename, &buf, &size))
		return KTX_INVALID_HEADER;
#ifdef NO_FMEMOPEN
	FILE *mem = tmpfile();
	if (mem) {
		fwrite(buf, 1, size, mem);
		rewind(mem);
	}
#else
	FILE *mem = fmemopen(buf, size, "rb");
#endif
	if (!mem) {
		free(buf);
		return KTX_INVALID_HEADER;
	}
	res = load_from_stream(mem, texture_id);
	fclose(mem);
	free(buf);
	return res;
}
