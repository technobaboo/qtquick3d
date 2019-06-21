import QtQuick 2.12
import QtDemon 1.0

DemonCustomMaterial {
    property bool uEnvironmentMappingEnabled: true
    property bool uShadowMappingEnabled: false
    property real reflection_map_offset: 0.5
    property real reflection_map_scale: 0.3
    property vector3d tiling: Qt.vector3d(1, 1, 1)
    property real roughness_map_offset: 0.16
    property real roughness_map_scale: 0.4
    property vector3d metal_color: Qt.vector3d(0.95, 0.95, 0.95)
    property real intensity: 1.0
    property vector3d emission_color: Qt.vector3d(0, 0, 0)
    property vector3d emissive_mask_offset: Qt.vector3d(0, 0, 0)
    property real bump_amount: 0.5

    shaderInfo: DemonCustomMaterialShaderInfo {
        version: "330"
        type: "GLSL"
        shaderKey: DemonCustomMaterialShaderInfo.Glossy
        layers: 1
    }

    property DemonCustomMaterialTexture uEnvironmentTexture: DemonCustomMaterialTexture {
            id: uEnvironmentTexture
            type: DemonCustomMaterialTexture.Environment
            enabled: uEnvironmentMappingEnabled
            image: DemonImage {
                id: envImage
                tilingModeHorizontal: DemonImage.Repeat
                tilingModeVertical: DemonImage.Repeat
                source: "maps/spherical_checker.png"
            }
    }
    property DemonCustomMaterialTexture uBakedShadowTexture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.LightmapShadow
            enabled: uShadowMappingEnabled
            image: DemonImage {
                id: shadowImage
                tilingModeHorizontal: DemonImage.Repeat
                tilingModeVertical: DemonImage.Repeat
                source: "maps/shadow.png"
            }
    }

    property DemonCustomMaterialTexture reflection_texture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Specular
            enabled: true
            image: DemonImage {
                id: reflectionTexture
                tilingModeHorizontal: DemonImage.Repeat
                tilingModeVertical: DemonImage.Repeat
                source: "maps/grunge_b.png"
            }
    }

    property DemonCustomMaterialTexture roughness_texture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Unknown
            enabled: true
            image: DemonImage {
                id: roughnessTexture
                tilingModeHorizontal: DemonImage.Repeat
                tilingModeVertical: DemonImage.Repeat
                source: "maps/grunge_d.png"
            }
    }

    property DemonCustomMaterialTexture emissive_texture: DemonCustomMaterialTexture {
            id: emissiveTexture
            type: DemonCustomMaterialTexture.Emissive
            enabled: true
            image: DemonImage {
                id: emissiveImage
                tilingModeHorizontal: DemonImage.Repeat
                tilingModeVertical: DemonImage.Repeat
                source: "maps/emissive.png"
            }
    }

    property DemonCustomMaterialTexture emissive_mask_texture: DemonCustomMaterialTexture {
            id: emissiveMaskTexture
            type: DemonCustomMaterialTexture.Unknown
            enabled: true
            image: DemonImage {
                id: emissiveMaskImage
                tilingModeHorizontal: DemonImage.Repeat
                tilingModeVertical: DemonImage.Repeat
                source: "maps/emissive_mask.png"
            }
    }

    property DemonCustomMaterialTexture bump_texture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Bump
            enabled: true
            image: DemonImage {
                id: bumpTexture
                tilingModeHorizontal: DemonImage.Repeat
                tilingModeVertical: DemonImage.Repeat
                source: "maps/grunge_d.png"
            }
    }

    DemonCustomMaterialShader {
        id: aluminumEmissiveShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/aluminumEmissive.frag"
    }

    passes: [
        DemonCustomMaterialPass {
            shaders: aluminumEmissiveShader
        }
    ]
}
