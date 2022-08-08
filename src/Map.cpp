#include "tp_maps_headless/Map.h"

#include "tp_maps/MouseEvent.h"
#include "tp_maps/KeyEvent.h"

#include "tp_utils/TimeUtils.h"
#include "tp_utils/DebugUtils.h"

#ifdef TP_LINUX
#include <EGL/egl.h>
#include <EGL/eglext.h>
#endif

#ifdef TP_WIN32
// You may need to build the MESA SDK on Windows.
// You can download it here:
//   - https://archive.mesa3d.org/
//
// Build instructions are here:
//   - https://docs.mesa3d.org/install.html
//   - https://docs.mesa3d.org/egl.html#build-egl
//
// Prerequisites:
// choco install meson
// choco install winflexbison
// /c/Python310/python3.exe -m pip install mako
//
// Build commands:
// meson ../build/ -D egl=enabled -D gles1=enabled -D gles2=enabled -D shared-glapi=enabled -Dprefix="C:\\Users\\PC\\projects\\external\\mesa\\install\\"
// ninja -C ../build/ install

#include <EGL/egl.h>
#include <EGL/eglext.h>
#endif


namespace tp_maps_headless
{
//##################################################################################################
struct Map::Private
{
  Map* q;

  bool ready{false};
#if defined(TP_LINUX) || defined(TP_WIN32)
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
#if defined(TP_LINUX) || defined(TP_WIN32)

  tpWarning() << "Query available EGL devices.";

  PFNEGLQUERYDEVICESEXTPROC eglQueryDevicesEXT = reinterpret_cast<PFNEGLQUERYDEVICESEXTPROC>(eglGetProcAddress("eglQueryDevicesEXT"));
  PFNEGLQUERYDEVICESTRINGEXTPROC eglQueryDeviceStringEXT = reinterpret_cast<PFNEGLQUERYDEVICESTRINGEXTPROC>(eglGetProcAddress("eglQueryDeviceStringEXT"));
  PFNEGLQUERYDISPLAYATTRIBEXTPROC eglQueryDisplayAttribEXT = reinterpret_cast<PFNEGLQUERYDISPLAYATTRIBEXTPROC>(eglGetProcAddress("eglQueryDisplayAttribEXT"));
  PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT"));

  EGLDeviceEXT devices[10];
  EGLint num_devices{0};
  eglQueryDevicesEXT(10, devices, &num_devices);
  if(num_devices<1)
  {
    tpWarning() << "Failed to find any EGL devices.";
    return;
  }

  for(EGLint n=0; n<num_devices; n++)
    tpWarning() << "Found device: " << n << " name: " << eglQueryDeviceStringEXT(devices[n], EGL_EXTENSIONS);

  tpDebug() << "Map::Map a";
  //-- Display -------------------------------------------------------------------------------------
  //d->display = eglGetDisplay(nullptr);

#ifdef TP_LINUX
  d->display = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, devices[0], nullptr);
#elif defined(TP_WIN32)
  d->display = eglGetPlatformDisplayEXT(EGL_PLATFORM_SURFACELESS_MESA, devices[0], nullptr);
#endif

  if(!d->display)
  {
    tpWarning() << "Failed to create EGL display.";
    return;
  }
  // //           0x31DD

//  switch (disp->Platform) {
//  case _EGL_PLATFORM_SURFACELESS:
//     ret = wgl_initialize_impl(disp, NULL);
//     break;
//  case _EGL_PLATFORM_WINDOWS:
//     ret = wgl_initialize_impl(disp, disp->PlatformDisplay);
//     break;
//  default:
//     unreachable("Callers ensure we cannot get here.");
//     return EGL_FALSE;


  tpDebug() << "Map::Map b";
  EGLint major, minor;
  if(eglInitialize(d->display, &major, &minor) != EGL_TRUE)
  {
    tpDebug() << "Map::Map b1";
    tpWarning() << "Failed to initialize EGL display.";
    return;
  }
  tpWarning() << "EGL version: " << major << '.' << minor;

  tpDebug() << "Map::Map c";
  EGLDeviceEXT device;
  if(eglQueryDisplayAttribEXT(d->display, EGL_DEVICE_EXT, reinterpret_cast<EGLAttrib*>(&device)) != EGL_TRUE)
  {
    tpWarning() << "Failed to query device!";
    return;
  }
  else
  {
    tpWarning() << "Using device: " << eglQueryDeviceStringEXT(device, EGL_EXTENSIONS);
  }

  tpDebug() << "Map::Map d";

  //-- Config --------------------------------------------------------------------------------------
  EGLConfig config;
  {
    const EGLint attributeList[] =
    {
      EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
      EGL_RED_SIZE, 1,
      EGL_GREEN_SIZE, 1,
      EGL_BLUE_SIZE, 1,
      EGL_DEPTH_SIZE, 24,
      EGL_CONFORMANT, EGL_OPENGL_BIT,
      //EGL_CONFORMANT, EGL_OPENGL_ES3_BIT,

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

  #ifdef TP_MAPS_DEBUG
      EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE,
  #endif

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
#ifdef TP_GLES2
    setOpenGLProfile(tp_maps::OpenGLProfile::VERSION_100_ES);
#else
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
#endif
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

#if defined(TP_LINUX) || defined(TP_WIN32)
  if(eglMakeCurrent(d->display, d->surface, d->surface, d->context) != EGL_TRUE)
    tpWarning() << "Failed to make current.";
#endif
}

}
