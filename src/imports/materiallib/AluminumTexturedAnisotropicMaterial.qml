import QtQuick 2.12
import QtDemon 1.0

DemonCustomMaterial {

    property bool uEnvironmentMappingEnabled: true
    property bool uShadowMappingEnabled: false
    property real material_ior: 8.0
    property real reflection_map_offset: 1.0
    property real reflection_map_scale: 1.0
    property vector2d tiling: Qt.vector2d(1, 1)
    property real roughness_map_offset: 0.0
    property real roughness_map_scale: 1.0
    property real anisotropy: 0.8
    property real aniso_tex_color_offset: 0.0
    property real aniso_tex_color_scale: 1.0
    property vector2d aniso_tiling: Qt.vector2d(1, 1)
    property real base_weight: 1.0
    property real bump_amount: 0.0

    shaderInfo: DemonCustomMaterialShaderInfo {
        version: "330"
        type: "GLSL"
        shaderKey: DemonCustomMaterialShaderInfo.Glossy | DemonCustomMaterialShaderInfo.Diffuse
        layers: 2
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

    property DemonCustomMaterialTexture reflection_texture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Specular
            enabled: true
            image: DemonImage {
                id: reflectionTexture
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/concentric_milled_steel.jpg"
            }
    }

    property DemonCustomMaterialTexture aniso_rot_texture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Anisotropy
            enabled: true
            image: DemonImage {
                id: anisoTexture
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/concentric_milled_steel_aniso.jpg"
            }
    }

    property DemonCustomMaterialTexture bump_texture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Bump
            enabled: true
            image: DemonImage {
                id: bumpTexture
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/concentric_milled_steel.jpg"
            }
    }
    DemonCustomMaterialShader {
        id: aluminumTexturedAnisoFragShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/aluminumTexturedAnisotropic.frag"
    }

    passes: [
        DemonCustomMaterialPass {
            shader: aluminumTexturedAnisoFragShader
        }
    ]
}
