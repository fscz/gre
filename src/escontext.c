#include "bcm_host.h"
#include "GLES2/gl2.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"

#include "escontext.h"

static const EGLint configAttrsDefault[] = {
  EGL_RED_SIZE, 8,
  EGL_GREEN_SIZE, 8,
  EGL_BLUE_SIZE, 8,
  EGL_ALPHA_SIZE, 8,
  EGL_DEPTH_SIZE, 8,
  EGL_STENCIL_SIZE, 8,
  EGL_SAMPLE_BUFFERS, 1,
  EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
  EGL_NONE
};

static const EGLint contextAttrs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};

static void get_native_window(ESContext* esContext, EGL_DISPMANX_WINDOW_T* nativeWindow) {
  DISPMANX_ELEMENT_HANDLE_T dispman_element;
  DISPMANX_DISPLAY_HANDLE_T dispman_display;
  DISPMANX_UPDATE_HANDLE_T dispman_update;
  VC_RECT_T dst_rect;
  VC_RECT_T src_rect;  

  dst_rect.x = 0;
  dst_rect.y = 0;
  dst_rect.width = esContext->screenWidth;
  dst_rect.height = esContext->screenHeight;
      
  src_rect.x = 0;
  src_rect.y = 0;
  src_rect.width = esContext->screenWidth << 16;
  src_rect.height = esContext->screenHeight << 16;        

  dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
  dispman_update = vc_dispmanx_update_start( 0 );
         
  dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
					      0/*layer*/, &dst_rect, 0/*src*/,
					      &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, 0/*transform*/);
      
  nativeWindow->element = dispman_element;
  nativeWindow->width = esContext->screenWidth;
  nativeWindow->height = esContext->screenHeight;
  vc_dispmanx_update_submit_sync( dispman_update );
}

int escontext_init(ESContext* esContext, const EGLint configAttrs[]) {
  EGLint numConfigs;
  EGLConfig config;
  bcm_host_init();

  if ( esContext != NULL ){
    memset( esContext, 0, sizeof( ESContext) );
  }

  if (EGL_TRUE != eglBindAPI(EGL_OPENGL_ES_API)) return -1;

  // Get Display
  esContext->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if ( esContext->display == EGL_NO_DISPLAY ) return -1;

  // Initialize EGL
  if ( EGL_TRUE != eglInitialize(esContext->display, NULL, NULL) ) return -1;

  // Get configs
  if ( EGL_TRUE != eglGetConfigs(esContext->display, NULL, 0, &numConfigs) ) return -1;

  // Choose config
  if ( EGL_TRUE != eglChooseConfig(esContext->display, configAttrs ? configAttrs : configAttrsDefault, &config, 1, &numConfigs) ) return -1;


  //////////////////////////////
  // NATIVE WINDOW STUFF  
  if ( 0 != graphics_get_display_size(0 /* LCD */, &esContext->screenWidth, &esContext->screenHeight) ) return -1;

  //EGL_DISPMANX_WINDOW_T nativeWindow;
  //get_native_window(esContext, &nativeWindow);  
  //////////////////////////////
  

  // Create a surface
  // for some weird reason you can pass NULL as the native window
  // this is probably a bug - nontheless i will take advantage of it for now
  esContext->surface = eglCreateWindowSurface(esContext->display, config, NULL, NULL); 
  if ( esContext->surface == EGL_NO_SURFACE ) return -1;

  // Create a GL context
  esContext->context = eglCreateContext(esContext->display, config, EGL_NO_CONTEXT, contextAttrs );
  if ( esContext->context == EGL_NO_CONTEXT ) return -1;  
   
  return 0;
}

int escontext_release(ESContext* esContext) {
  // bind context to the releasing thread
  eglMakeCurrent( esContext->display, esContext->surface, esContext->surface, esContext->context );
  // Release OpenGL resources

  glClear( GL_COLOR_BUFFER_BIT );
  eglSwapBuffers(esContext->display, esContext->surface);
  eglMakeCurrent( esContext->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );

  eglDestroySurface( esContext->display, esContext->surface );
  eglDestroyContext( esContext->display, esContext->context );
  eglTerminate( esContext->display );
  return 0;
}
