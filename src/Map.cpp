#include "tp_maps_headless/Map.h"

#include "tp_maps/MouseEvent.h"
#include "tp_maps/KeyEvent.h"

#include "tp_utils/TimeUtils.h"
#include "tp_utils/DebugUtils.h"

#ifdef TP_LINUX
#include <EGL/egl.h>
#endif

namespace tp_maps_headless
{
//##################################################################################################
struct Map::Private
{
  Map* q;

  bool ready{false};
#ifdef TP_LINUX
  EGLDisplay display{nullptr};
  EGLContext context{nullptr};
  EGLSurface surface{nullptr};
#endif

  //################################################################################################
  Private(Map* q_):
    q(q_)
  {

  }
};

//##################################################################################################
Map::Map(bool enableDepthBuffer):
  tp_maps::Map(enableDepthBuffer),
  d(new Private(this))
{
#ifdef TP_LINUX
  //-- Display -------------------------------------------------------------------------------------
  d->display = eglGetDisplay(nullptr);
  if(!d->display)
  {
    tpWarning() << "Failed to create EGL display.";
    return;
  }

  if(eglInitialize(d->display, nullptr, nullptr) != EGL_TRUE)
  {
    tpWarning() << "Failed to initialize EGL display.";
    return;
  }


  //-- Config --------------------------------------------------------------------------------------
  EGLConfig config;
  {
    const EGLint attributeList[] =
    {
      EGL_RED_SIZE, 1,
      EGL_GREEN_SIZE, 1,
      EGL_BLUE_SIZE, 1,
      EGL_DEPTH_SIZE, 24,
      EGL_CONFORMANT, EGL_OPENGL_BIT,
      //EGL_CONFORMANT, EGL_OPENGL_ES2_BIT,
      EGL_NONE
    };
    EGLint numConfig;
    eglChooseConfig(d->display, attributeList, &config, 1, &numConfig);
    if(!config)
    {
      tpWarning() << "Failed to initialize EGL config.";
      return;
    }
  }


  //-- Context -------------------------------------------------------------------------------------
  {
    const EGLint attributeList[] =
    {
      EGL_CONTEXT_CLIENT_VERSION, 3,
      EGL_NONE
    };
    d->context = eglCreateContext(d->display, config, EGL_NO_CONTEXT, attributeList);
    if(!d->context)
    {
      tpWarning() << "Failed to initialize EGL context.";
      return;
    }
  }


  //-- Surface -------------------------------------------------------------------------------------
  {
    const EGLint attributeList[] =
    {
      EGL_WIDTH, 100,
      EGL_HEIGHT, 100,
      EGL_NONE
    };
    d->surface = eglCreatePbufferSurface(d->display, config, attributeList);
    if(d->surface == EGL_NO_SURFACE)
    {
      tpWarning() << "Failed to create EGL surface.";
      return;
    }
  }

  d->ready = true;
  makeCurrent();

  //-- Use the OpenGL ES version that we were given ------------------------------------------------
  {
    // This could maybe go tp_maps::Map::initializeGL()
    GLint major{0};
    GLint minor{0};
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    switch(major*10+minor)
    {
    case 10: setOpenGLProfile(tp_maps::OpenGLProfile::VERSION_100_ES); break;
    case 30: setOpenGLProfile(tp_maps::OpenGLProfile::VERSION_300_ES); break;
    case 31: setOpenGLProfile(tp_maps::OpenGLProfile::VERSION_310_ES); break;
    case 32: setOpenGLProfile(tp_maps::OpenGLProfile::VERSION_320_ES); break;
    }
  }

  initializeGL();
#endif
}

//##################################################################################################
Map::~Map()
{
  preDelete();
  delete d;
}

//##################################################################################################
void Map::makeCurrent()
{
  if(!d->ready)
    return;

#ifdef TP_LINUX
  if(eglMakeCurrent(d->display, d->surface, d->surface, d->context) != EGL_TRUE)
    tpWarning() << "Failed to make current.";
#endif
}

//##################################################################################################
void Map::update()
{

}

}
