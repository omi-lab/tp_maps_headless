
if(WIN32)
  list(APPEND TP_LIBRARIES "glfw3")
elseif(NOT APPLE)
  list(APPEND TP_LIBRARIES "OpenGL")
  list(APPEND TP_LIBRARIES "EGL")
endif()

