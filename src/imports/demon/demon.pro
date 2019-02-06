CXX_MODULE = qml
TARGET = qdemonplugin
TARGETPATH = QtDemon
IMPORT_VERSION = 5.12

QT += qml quick demonruntimerender-private

OTHER_FILES += \
    qmldir

load(qml_plugin)

SOURCES += \
    plugin.cpp \
    qdemonobject.cpp \
    qdemonscene.cpp \
    qdemonnode.cpp \
    qdemonimage.cpp \
    qdemoncamera.cpp \
    qdemonlight.cpp \
    qdemonmodel.cpp \
    qdemonlayer.cpp \
    qdemoneffect.cpp

HEADERS += \
    qdemonobject.h \
    qdemonscene.h \
    qdemonnode.h \
    qdemonimage.h \
    qdemoncamera.h \
    qdemonlight.h \
    qdemonmodel.h \
    qdemonlayer.h \
    qdemoneffect.h
