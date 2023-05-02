
if(WIN32)
elseif(UNIX)
  list(APPEND TP_LIBRARIES "GL" "EGL")
else() 
  list(APPEND TP_LIBRARIES "OpenGL" "EGL")
endif()

