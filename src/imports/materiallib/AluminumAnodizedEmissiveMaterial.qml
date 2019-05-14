import QtQuick 2.12
import QtDemon 1.0

DemonCustomMaterial {

    property bool uEnvironmentMappingEnabled: true
    property bool uShadowMappingEnabled: false
    property real roughness: 0.3
    property vector3d base_color: Qt.vector3d(0.7, 0.7, 0.7)
    property real intensity: 1.0
    property vector3d emission_color: Qt.vector3d(0, 0, 0)


    shaderInfo: DemonCustomMaterialShaderInfo {
        version: "330"
        type: "GLSL"
        shaderKey: DemonCustomMaterialShaderInfo.Glossy
        layers: 1
    }

    property DemonCustomMaterialTexture emissive_texture: DemonCustomMaterialTexture {
            id: emissiveTexture
            type: DemonCustomMaterialTexture.Emissive
            enabled: true
            image: DemonImage {
                id: emissiveImage
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/emissive.jpg"
            }
    }

    property DemonCustomMaterialTexture emissive_mask_texture: DemonCustomMaterialTexture {
            id: emissiveMaskTexture
            type: DemonCustomMaterialTexture.Unknown
            enabled: true
            image: DemonImage {
                id: emissiveMaskImage
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/emissive_mask.jpg"
            }
    }

    property DemonCustomMaterialTexture uEnvironmentTexture: DemonCustomMaterialTexture {
            id: uEnvironmentTexture
            type: DemonCustomMaterialTexture.Environment
            enabled: uEnvironmentMappingEnabled
            image: DemonImage {
                id: envImage
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/spherical_checker.jpg"
            }
    }
    property DemonCustomMaterialTexture uBakedShadowTexture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.LightmapShadow
            enabled: uShadowMappingEnabled
            image: DemonImage {
                id: shadowImage
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/shadow.jpg"
            }
    }

    DemonCustomMaterialShader {
        id: aluminumAnodizedEmissiveShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/aluminumAnodizedEmissive.frag"
    }

    passes: [
        DemonCustomMaterialPass {
            shader: aluminumAnodizedEmissiveShader
        }
    ]

}
