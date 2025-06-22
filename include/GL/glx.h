#ifndef GLX_H
#define GLX_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GLES/gl.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GLX_VERSION_1_1 1

enum _GLX_CONFIGS {
    GLX_USE_GL = 1,
    GLX_BUFFER_SIZE,
    GLX_LEVEL,
    GLX_RGBA,
    GLX_DOUBLEBUFFER,
    GLX_STEREO,
    GLX_AUX_BUFFERS,
    GLX_RED_SIZE,
    GLX_GREEN_SIZE,
    GLX_BLUE_SIZE,
    GLX_ALPHA_SIZE,
    GLX_DEPTH_SIZE,
    GLX_STENCIL_SIZE,
    GLX_ACCUM_RED_SIZE,
    GLX_ACCUM_GREEN_SIZE,
    GLX_ACCUM_BLUE_SIZE,
    GLX_ACCUM_ALPHA_SIZE,
    GLX_X_VISUAL_TYPE_EXT = 0x22,
    GLX_TRANSPARENT_TYPE_EXT = 0x23,
    GLX_TRANSPARENT_INDEX_VALUE_EXT = 0x24,
    GLX_TRANSPARENT_RED_VALUE_EXT = 0x25,
    GLX_TRANSPARENT_GREEN_VALUE_EXT = 0x26,
    GLX_TRANSPARENT_BLUE_VALUE_EXT = 0x27,
    GLX_TRANSPARENT_ALPHA_VALUE_EXT = 0x28
};

#define GLX_BAD_SCREEN 1
#define GLX_BAD_ATTRIBUTE 2
#define GLX_NO_EXTENSION 3
#define GLX_BAD_VISUAL 4
#define GLX_BAD_CONTEXT 5
#define GLX_BAD_VALUE 6
#define GLX_BAD_ENUM 7

#define GLX_VENDOR 1
#define GLX_VERSION 2
#define GLX_EXTENSIONS 3

#define GLX_TRUE_COLOR_EXT 0x8002
#define GLX_DIRECT_COLOR_EXT 0x8003
#define GLX_PSEUDO_COLOR_EXT 0x8004
#define GLX_STATIC_COLOR_EXT 0x8005
#define GLX_GRAY_SCALE_EXT 0x8006
#define GLX_STATIC_GRAY_EXT 0x8007
#define GLX_NONE_EXT 0x8000
#define GLX_TRANSPARENT_RGB_EXT 0x8008
#define GLX_TRANSPARENT_INDEX_EXT 0x8009

typedef void *GLXContext;
typedef Pixmap GLXPixmap;
typedef Drawable GLXDrawable;
typedef XID GLXContextID;

XVisualInfo *glXChooseVisual(Display *dpy, int screen, int *attribList);
GLXContext glXCreateContext(Display *dpy, XVisualInfo *vis,
                            GLXContext shareList, Bool direct);
void glXDestroyContext(Display *dpy, GLXContext ctx);
Bool glXMakeCurrent(Display *dpy, GLXDrawable drawable, GLXContext ctx);
void glXCopyContext(Display *dpy, GLXContext src, GLXContext dst,
                    GLuint mask);
void glXSwapBuffers(Display *dpy, GLXDrawable drawable);
GLXPixmap glXCreateGLXPixmap(Display *dpy, XVisualInfo *visual, Pixmap pixmap);
void glXDestroyGLXPixmap(Display *dpy, GLXPixmap pixmap);
Bool glXQueryExtension(Display *dpy, int *errorb, int *event);
Bool glXQueryVersion(Display *dpy, int *maj, int *min);
Bool glXIsDirect(Display *dpy, GLXContext ctx);
int glXGetConfig(Display *dpy, XVisualInfo *visual, int attrib, int *value);
GLXContext glXGetCurrentContext(void);
GLXDrawable glXGetCurrentDrawable(void);
void glXWaitGL(void);
void glXWaitX(void);
void glXUseXFont(Font font, int first, int count, int list);
const char *glXQueryExtensionsString(Display *dpy, int screen);
const char *glXQueryServerString(Display *dpy, int screen, int name);
const char *glXGetClientString(Display *dpy, int name);

#ifdef __cplusplus
}
#endif

#endif /* GLX_H */
