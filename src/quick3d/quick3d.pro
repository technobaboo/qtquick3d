TARGET = QtQuick3d
MODULE = quick3d

QT = core-private gui-private demonruntimerender-private quick-private

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
    qdemonwindow.cpp \
    qdemonrenderloop.cpp

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
    qtquick3dglobal_p.h \
    qdemonwindow_p.h \
    qdemonrenderloop_p.h \
    qdemonobject_p.h \
    qdemonobjectchangelistener_p.h

load(qt_module)
