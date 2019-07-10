TARGET = QtQuick3D
MODULE = quick3d

QT = core-private gui-private demonruntimerender-private quick-private

DEFINES += QT_BUILD_QUICK3D_LIB

QMAKE_DOCS = $$PWD/doc/quick3d.qdocconf

SOURCES += \
    qquick3dcamera.cpp \
    qquick3dcustommaterial.cpp \
    qquick3ddefaultmaterial.cpp \
    qquick3deffect.cpp \
    qquick3dlight.cpp \
    qquick3dmaterial.cpp \
    qquick3dmodel.cpp \
    qquick3dnode.cpp \
    qquick3dobject.cpp \
    qquick3dsceneenvironment.cpp \
    qquick3dscenemanager.cpp \
    qquick3dscenerenderer.cpp \
    qquick3dtexture.cpp \
    qquick3dview3d.cpp

HEADERS += \
    qquick3dcamera.h \
    qquick3dcustommaterial.h \
    qquick3ddefaultmaterial.h \
    qquick3deffect.h \
    qquick3dlight.h \
    qquick3dmaterial.h \
    qquick3dmodel.h \
    qquick3dnode.h \
    qquick3dobject.h \
    qquick3dobject_p.h \
    qquick3dobjectchangelistener_p.h \
    qquick3dsceneenvironment.h \
    qquick3dscenemanager_p.h \
    qquick3dscenerenderer.h \
    qquick3dtexture.h \
    qquick3dview3d.h \
    qtquick3dglobal.h \
    qtquick3dglobal_p.h

load(qt_module)
