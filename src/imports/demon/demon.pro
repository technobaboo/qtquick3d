CXX_MODULE = qml
TARGET = qdemonplugin
TARGETPATH = QtDemon
IMPORT_VERSION = 5.12

QT += qml quick quick3d

OTHER_FILES += \
    qmldir

load(qml_plugin)

SOURCES += \
    plugin.cpp
