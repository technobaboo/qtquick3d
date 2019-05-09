CXX_MODULE = qml
TARGET = qtdemonmaterialplugin
TARGETPATH = QtDemonMaterialLibrary
QT += quick qml
IMPORT_VERSION = 1.0
QML_FILES = \
            AluminumMaterial.qml \
            CopperMaterial.qml \
            MeshFenceMaterial.qml \
            PorcelainMaterial.qml \
            GlassMaterial.qml \
            FrostedGlassMaterial.qml

# !static: CONFIG += qmlcache

RESOURCES += \
    qtmateriallibrary.qrc

SOURCES += \
    plugin.cpp

DISTFILES += \
    qmldir \
    MeshFenceMaterial.qml

load(qml_plugin)
