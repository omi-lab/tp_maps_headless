DEFINES += TP_FILESYSTEM

macx {
  LIBS += -lOpenGL -lEGL
}

else:iphoneos {

}

else:android {

}

else:win32 {
  LIBS += libEGL.dll.a
  LIBS += libGLESv2.dll.a
}

else {
  LIBS += -lOpenGL -lEGL
}
