TARGET = QtQuick3d
MODULE = quick3d

QT = core gui demonruntimerender-private

DEFINES += QT_BUILD_QUICK3D_LIB

SOURCES += \
    qdemonobject.cpp \
    qdemonscene.cpp \
    qdemonnode.cpp \
    qdemonimage.cpp \
    qdemoncamera.cpp \
    qdemonlight.cpp \
    qdemonmodel.cpp \
    qdemonlayer.cpp \
    qdemoneffect.cpp \
    qdemonmaterial.cpp \
    qdemondefaultmaterial.cpp \
    qdemoncustommaterial.cpp \
    qdemonwindow.cpp

HEADERS += \
    qdemonobject.h \
    qdemonscene.h \
    qdemonnode.h \
    qdemonimage.h \
    qdemoncamera.h \
    qdemonlight.h \
    qdemonmodel.h \
    qdemonlayer.h \
    qdemoneffect.h \
    qdemonmaterial.h \
    qdemondefaultmaterial.h \
    qdemoncustommaterial.h \
    qdemonwindow.h \
    qtquick3dglobal.h \
    qtquick3dglobal_p.h

load(qt_module)
