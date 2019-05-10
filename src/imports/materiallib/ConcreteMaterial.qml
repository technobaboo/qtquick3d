import QtQuick 2.12
import QtDemon 1.0

DemonCustomMaterial {
    property bool uEnvironmentMappingEnabled: true
    property bool uShadowMappingEnabled: false
    property real material_ior: 1.5
    property real roughness: 0.35
    property vector2d texture_tiling: Qt.vector2d(5, 2)
    property real bump_amount: 6

    shaderInfo: DemonCustomMaterialShaderInfo {
        version: "330"
        type: "GLSL"
        shaderKey: DemonCustomMaterialShaderInfo.Glossy | DemonCustomMaterialShaderInfo.Diffuse
        layers: 2
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
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/shadow.jpg"
            }
    }

    property DemonCustomMaterialTexture bump_texture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Bump
            enabled: true
            name: "bump_texture"
            image: DemonImage {
                id: bumpTexture
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/concrete_plain_bump.jpg"
            }
    }

    property DemonCustomMaterialTexture diffuse_texture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Diffuse
            enabled: true
            name: "diffuse_texture"
            image: DemonImage {
                id: diffuseTexture
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/concrete_plain.jpg"
            }
    }

    DemonCustomMaterialShader {
        id: concreteFragShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/concrete.frag"
    }

    passes: [
        DemonCustomMaterialPass {
            shader: concreteFragShader
        }
    ]
}
