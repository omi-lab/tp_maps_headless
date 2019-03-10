#include "tp_maps_headless/Map.h"

#include "tp_maps/MouseEvent.h"
#include "tp_maps/KeyEvent.h"

#include "tp_utils/TimeUtils.h"
#include "tp_utils/DebugUtils.h"

#include <EGL/egl.h>

namespace tp_maps_headless
{
//##################################################################################################
struct Map::Private
{
  Map* q;

  bool ready{false};
  EGLDisplay display{nullptr};
  EGLContext context{nullptr};
  EGLSurface surface{nullptr};

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
      EGL_CONFORMANT, EGL_OPENGL_ES2_BIT,
      EGL_NONE
    };
    EGLint numConfig;
    eglChooseConfig(d->display, attributeList, &config, 1, &numConfig);
  }


  //-- Context -------------------------------------------------------------------------------------
  {

    const EGLint attributeList[] =
    {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
    };
    d->context = eglCreateContext(d->display, config, EGL_NO_CONTEXT, attributeList);
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

  initializeGL();
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

  if(eglMakeCurrent(d->display, d->surface, d->surface, d->context) != EGL_TRUE)
    tpWarning() << "Failed to make current.";
}

//##################################################################################################
void Map::update()
{

}

}
