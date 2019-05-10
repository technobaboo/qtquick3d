CXX_MODULE = qml
TARGET = qtdemonmaterialplugin
TARGETPATH = QtDemonMaterialLibrary
QT += quick qml
IMPORT_VERSION = 1.0
QML_FILES = \
            AluminumAnisotropicMaterial.qml \
            AluminumBrushedMaterial.qml \
            AluminumEmissiveMaterial.qml \
            AluminumMaterial.qml \
            AluminumAnodizedEmissiveMaterial.qml \
            AluminumAnodizedMaterial.qml \
            AluminumTexturedAnisotropicMaterial.qml \
            AsphaltMaterial.qml \
            BambooNaturalMatteEmissiveMaterial.qml \
            BambooNaturalMatteMaterial.qml \
            CarbonFiberEmissiveMaterial.qml \
            CarbonFiberMaterial.qml \
            CarPaintBlueStandardMaterial.qml \
            CarPaintColorPeel2LayerMaterial.qml \
            CopperMaterial.qml \
            MeshFenceMaterial.qml \
            MetalFenceFineMaterial.qml \
            PaperArtisticMaterial.qml \
            PaperOfficeMaterial.qml \
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
    qmldir

load(qml_plugin)
