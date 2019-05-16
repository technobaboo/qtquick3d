import QtQuick 2.12
import QtDemon 1.0

DemonCustomMaterial {

    property bool uEnvironmentMappingEnabled: true
    property bool uShadowMappingEnabled: false
    property real coat_ior: 1.24
    property real coat_weight: 1
    property real coat_roughness: 0
    property real base_ior: 1.65
    property real base_weight: 0.5
    property real base_roughness: 0.1
    property real anisotropy: 0.8
    property vector2d texture_tiling: Qt.vector2d(12, 12)
    property real bump_amount: 1.0

    shaderInfo: DemonCustomMaterialShaderInfo {
        version: "330"
        type: "GLSL"
        shaderKey: DemonCustomMaterialShaderInfo.Glossy | DemonCustomMaterialShaderInfo.Diffuse
        layers: 3
    }

    property DemonCustomMaterialTexture uEnvironmentTexture: DemonCustomMaterialTexture {
            id: uEnvironmentTexture
            type: DemonCustomMaterialTexture.Environment
            enabled: uEnvironmentMappingEnabled
            image: DemonImage {
                id: envImage
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/spherical_checker.png"
            }
    }
    property DemonCustomMaterialTexture uBakedShadowTexture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.LightmapShadow
            enabled: uShadowMappingEnabled
            image: DemonImage {
                id: shadowImage
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/shadow.png"
            }
    }

    property DemonCustomMaterialTexture anisotropy_rotation_texture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Anisotropy
            enabled: true
            image: DemonImage {
                id: anisoTexture
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/carbon_fiber_aniso.png"
            }
    }

    property DemonCustomMaterialTexture reflect_texture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Specular
            enabled: true
            image: DemonImage {
                id: reflectionTexture
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/carbon_fiber_spec.png"
            }
    }

    property DemonCustomMaterialTexture diffuse_texture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Diffuse
            enabled: true
            image: DemonImage {
                id: diffuseTexture
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/carbon_fiber.png"
            }
    }

    property DemonCustomMaterialTexture bump_texture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Bump
            enabled: true
            image: DemonImage {
                id: bumpTexture
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/carbon_fiber_bump.png"
            }
    }

    DemonCustomMaterialShader {
        id: carbonFiberEmissiveFragShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/carbonFiber.frag"
    }

    passes: [
        DemonCustomMaterialPass {
            shader: carbonFiberEmissiveFragShader
        }
    ]
}
