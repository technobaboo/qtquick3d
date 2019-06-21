import QtQuick 2.12
import QtDemon 1.0

DemonCustomMaterial {
    property bool uEnvironmentMappingEnabled: true
    property bool uShadowMappingEnabled: false
    property real material_ior: 20.0
    property real anisotropy: 0.8
    property vector2d texture_tiling: Qt.vector2d(8, 5)

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
    property DemonCustomMaterialTexture diffuse_texture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Diffuse
            enabled: true
            image: DemonImage {
                id: diffuseTexture
                tilingModeHorizontal: DemonImage.Repeat
                tilingModeVertical: DemonImage.Repeat
                source: "maps/concentric_milled_steel.png"
            }
    }
    property DemonCustomMaterialTexture anisotropy_rot_texture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Anisotropy
            enabled: true
            image: DemonImage {
                id: anisoTexture
                tilingModeHorizontal: DemonImage.Repeat
                tilingModeVertical: DemonImage.Repeat
                source: "maps/concentric_milled_steel_aniso.png"
            }
    }

    DemonCustomMaterialShader {
        id: steelMilledConcentricFragShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/steelMilledConcentric.frag"
    }

    passes: [
        DemonCustomMaterialPass {
            shaders: steelMilledConcentricFragShader
        }
    ]
}
