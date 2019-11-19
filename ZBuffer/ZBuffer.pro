SOURCES += \
  lib/tiny_obj_loader.cc \
  src/MainWindow.cpp \
  src/ZBWidget.cpp \
  src/Model.cpp \
  src/main.cc

HEADERS += lib/Logger.hpp \
    lib/tiny_obj_loader.h \
        src/MainWindow.hpp \
        src/Model.hpp \
        src/ZBWidget.hpp \

FORMS += src/MainWindow.ui  \

INCLUDEPATH += lib/ \
  ../QGLViewer/ \
  ../

DEPENDPATH += lib/ \
  ../QGLViewer/ \
  ../



OBJECTS_DIR = build/
RCC_DIR     = build/
MOC_DIR     = build/
UI_DIR      = build/

QT          += opengl xml widgets gui
CONFIG      += debug

DESTDIR     = ..
TARGET      = zbuffer

DISTFILES += \
    ../../../../../../../Program Files (x86)/Microsoft Visual Studio 14.0/VC/include/Eigen/CMakeLists.txt \
    ../../../../../../../Program Files (x86)/Microsoft Visual Studio 14.0/VC/include/Eigen/CMakeLists.txt
