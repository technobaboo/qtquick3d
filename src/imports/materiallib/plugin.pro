CXX_MODULE = qml
TARGET = qtdemonmaterialplugin
TARGETPATH = QtDemonMaterialLibrary
QT += quick qml
IMPORT_VERSION = 1.0
QML_FILES = \
            AluminumAnisotropicMaterial.qml \
            AluminumBrushedMaterial.qml \
            AluminumMaterial.qml \
            AluminumAnodizedEmissiveMaterial.qml \
            AluminumAnodizedMaterial.qml \
            CopperMaterial.qml \
            MeshFenceMaterial.qml \
            MetalFenceFineMaterial.qml \
            PorcelainMaterial.qml \
            GlassMaterial.qml \
            FrostedGlassMaterial.qml

# !static: CONFIG += qmlcache

OTHER_FILES += $$QML_FILES

RESOURCES += \
    qtmateriallibrary.qrc

SOURCES += \
    plugin.cpp

DISTFILES += \
    qmldir \
    MetalFenceFineMaterial.qml

load(qml_plugin)
