#include "gl_ext_common.h"

/* dummy references to force linking of extension objects */
extern int ext_link_dummy_OES_draw_texture;
extern int ext_link_dummy_OES_matrix_get;
extern int ext_link_dummy_OES_point_size_array;
extern int ext_link_dummy_OES_point_sprite;
extern int ext_link_dummy_OES_framebuffer_object;
extern int ext_link_dummy_OES_egl_image;
extern int ext_link_dummy_OES_egl_image_external;
extern int ext_link_dummy_OES_required_internalformat;
extern int ext_link_dummy_OES_blend_eq_sep;
extern int ext_link_dummy_OES_fixed_point;
__attribute__((used)) static void *force_link[] = {
	&ext_link_dummy_OES_draw_texture,
	&ext_link_dummy_OES_matrix_get,
	&ext_link_dummy_OES_point_size_array,
	&ext_link_dummy_OES_point_sprite,
	&ext_link_dummy_OES_framebuffer_object,
	&ext_link_dummy_OES_egl_image,
	&ext_link_dummy_OES_egl_image_external,
	&ext_link_dummy_OES_required_internalformat,
	&ext_link_dummy_OES_blend_eq_sep,
	&ext_link_dummy_OES_fixed_point
};
static char ext_string[512] = "";
static size_t ext_len = 0;
void ext_register(const char *flag)
{
	size_t n = strlen(flag);
	if (ext_len) {
		ext_string[ext_len++] = ' ';
	}
	memcpy(ext_string + ext_len, flag, n);
	ext_len += n;
	ext_string[ext_len] = '\0';
}
const GLubyte *renderer_get_extensions(void)
{
	return (const GLubyte *)ext_string;
}
