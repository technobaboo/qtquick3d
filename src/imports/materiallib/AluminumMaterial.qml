import QtQuick 2.12
import QtDemon 1.0

DemonCustomMaterial {
    property real reflection_map_offset: 0.5
    property real reflection_map_scale: 0.3
    property real roughness_map_offset: 0.16
    property real roughness_map_scale: 0.4
    property real bump_amount: 0.5
    property bool uEnvironmentMappingEnabled: true
    property bool uShadowMappingEnabled: false
    property vector3d tiling: Qt.vector3d(1, 1, 1)
    property vector3d metal_color: Qt.vector3d(0.95, 0.95, 0.95)

    shaderInfo: DemonCustomMaterialShaderInfo {
        version: "330"
        type: "GLSL"
        shaderKey: DemonCustomMaterialShaderInfo.Glossy
        layers: 1
    }

    property DemonCustomMaterialTexture uEnvironmentTexture: DemonCustomMaterialTexture {
            id: uEnvironmentTexture
            type: DemonCustomMaterialTexture.Environment
            name: "uEnvironmentTexture"
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
            name: "uBakedShadowTexture"
            image: DemonImage {
                id: shadowImage
                source: "maps/shadow.jpg"
            }
    }
    property DemonCustomMaterialTexture reflection_texture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Specular
            enabled: true
            name: "reflection_texture"
            image: DemonImage {
                id: reflectionTexture
                source: "maps/grunge_b.jpg"
            }
    }
    property DemonCustomMaterialTexture roughness_texture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Unknown
            enabled: true
            name: "roughness_texture"
            image: DemonImage {
                id: roughnessTexture
                source: "maps/grunge_d.jpg"
            }
    }
    property DemonCustomMaterialTexture bump_texture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Bump
            enabled: true
            name: "bump_texture"
            image: DemonImage {
                id: bumpTexture
                source: "maps/grunge_d.jpg"
            }
    }

    DemonCustomMaterialShader {
        id: aluminumFragShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/aluminum.frag"
    }

    passes: [
        DemonCustomMaterialPass {
            shader: aluminumFragShader
        }
    ]
}
