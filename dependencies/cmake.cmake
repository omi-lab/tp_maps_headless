
if(WIN32)
  list(APPEND TP_LIBRARIES "glfw3")
else() 
  list(APPEND TP_LIBRARIES "OpenGL")
  list(APPEND TP_LIBRARIES "EGL")
endif()

