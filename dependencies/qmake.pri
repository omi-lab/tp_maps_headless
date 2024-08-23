custom_opengl {
}

else:macx {
# Fails to link on macOS Ventura but seems to work without this.
# LIBS += -lOpenGL -lEGL
}

else:iphoneos {

}

else:android {

}

else:win32 {
  LIBS += glfw3.lib
  LIBS += gdi32.lib
  LIBS += opengl32.lib
  LIBS += kernel32.lib
  LIBS += user32.lib
  LIBS += shell32.lib
}

else {
  LIBS += -lOpenGL -lEGL
}
