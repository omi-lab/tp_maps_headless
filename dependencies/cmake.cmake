
if(WIN32)
else() 
  list(APPEND TP_LIBRARIES "OpenGL")
  list(APPEND TP_LIBRARIES "EGL")
endif()

