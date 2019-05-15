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
            CarPaintYellowStandardMaterial.qml \
            ConcreteMaterial.qml \
            CopperMaterial.qml \
            LeatherSmoothedBlackMaterial.qml \
            MeshFenceMaterial.qml \
            MetalFenceFineMaterial.qml \
            PaperArtisticMaterial.qml \
            PaperOfficeMaterial.qml \
            PlasticStructuredRedMaterial.qml \
            PlasticStructuredRedEmissiveMaterial.qml \
            PorcelainMaterial.qml \
            PowderCoatMaterial.qml \
            PowderCoatEmissiveMaterial.qml \
            RubberStuddedMaterial.qml \
            RubberStuddedEmissiveMaterial.qml \
            SteelMilledConcentricMaterial.qml \
            GlassMaterial.qml \
            GlassRefractiveMaterial.qml \
            FrostedGlassMaterial.qml \
            FrostedGlassSinglePassMaterial.qml \
            WalnutMatteMaterial.qml

MATERIAL_IMAGE_FILES += \
    maps/randomGradient1D.png \
    maps/randomGradient2D.png \
    maps/randomGradient3D.png \
    maps/randomGradient4D.png \
    maps/art_paper_normal.jpg \
    maps/art_paper_trans.jpg \
    maps/asphalt.jpg \
    maps/asphalt_bump.jpg \
    maps/bamboo_natural.jpg \
    maps/bamboo_natural_bump.jpg \
    maps/bamboo_natural_spec.jpg \
    maps/brushed_a.jpg \
    maps/brushed_full_contrast.jpg \
    maps/carbon_fiber.jpg \
    maps/carbon_fiber_aniso.jpg \
    maps/carbon_fiber_bump.jpg \
    maps/carbon_fiber_spec.jpg \
    maps/concentric_milled_steel.jpg \
    maps/concentric_milled_steel_aniso.jpg \
    maps/concrete_plain.jpg \
    maps/concrete_plain_bump.jpg \
    maps/cyclone_mesh_fencing.jpg \
    maps/cyclone_mesh_fencing_normal.jpg \
    maps/emissive.jpg \
    maps/emissive_mask.jpg \
    maps/grunge_b.jpg \
    maps/grunge_d.jpg \
    maps/metal_mesh.jpg \
    maps/metal_mesh_bump.jpg \
    maps/metal_mesh_spec.jpg \
    maps/paper_diffuse.jpg \
    maps/paper_trans.jpg \
    maps/powdercoat_bump_01.jpg \
    maps/shadow.jpg \
    maps/smooth_black_leather.jpg \
    maps/smooth_black_leather_bump.jpg \
    maps/smooth_black_leather_spec.jpg \
    maps/spherical_checker.jpg \
    maps/studded_rubber_bump.jpg \
    maps/walnut.jpg \
    maps/walnut_bump.jpg \
    maps/walnut_spec.jpg

QML_FILES += $$MATERIAL_IMAGE_FILES

OTHER_FILES += $$QML_FILES

RESOURCES += \
    qtmateriallibrary.qrc

SOURCES += \
    plugin.cpp

DISTFILES += \
    qmldir

load(qml_plugin)
