#ifndef gre_escontext_h
#define gre_escontext_h

#include "bcm_host.h"
#include "GLES2/gl2.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"


typedef struct
{
    uint32_t screenWidth;
    uint32_t screenHeight;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
} ESContext;

int escontext_init(ESContext* esContext, const EGLint configAttrs[]);
int escontext_release(ESContext* esContext);

#endif
