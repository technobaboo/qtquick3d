CXX_MODULE = qml
TARGET = qdemonplugin
TARGETPATH = QtDemon
IMPORT_VERSION = 1.$$QT_MINOR_VERSION

QT += qml quick quick3d

OTHER_FILES += \
    qmldir

load(qml_plugin)

SOURCES += \
    plugin.cpp
