import QtQuick 2.12
import QtDemon 1.0

DemonCustomMaterial {
    // These properties names need to match the ones in the shader code!
    property bool uEnvironmentMappingEnabled: true
    property bool uShadowMappingEnabled: false
    property real bump_amount: 0.4
    property real material_ior: 1.5
    property real roughness: 0.25
    property vector2d texture_tiling: Qt.vector2d(10.0, 7.0)

    shaderInfo: DemonCustomMaterialShaderInfo {
        version: "330"
        type: "GLSL"
        shaderKey: DemonCustomMaterialShaderInfo.Glossy | DemonCustomMaterialShaderInfo.Diffuse
        layers: 2
    }

    property DemonCustomMaterialTexture uEnvironmentTexture: DemonCustomMaterialTexture {
        type: DemonCustomMaterialTexture.Environment
        enabled: true
        image: DemonImage {
            tilingModeHorizontal: DemonImage.Repeat
            tilingModeVertical: DemonImage.Repeat
            source: "maps/spherical_checker.png"
        }
    }
    property DemonCustomMaterialTexture uBakedShadowTexture: DemonCustomMaterialTexture {
            enabled: uShadowMappingEnabled
            type: DemonCustomMaterialTexture.LightmapShadow
            image: DemonImage {
                tilingModeHorizontal: DemonImage.Repeat
                tilingModeVertical: DemonImage.Repeat
                id: shadowImage
                source: "maps/shadow.png"
            }
    }
    property DemonCustomMaterialTexture diffuse_texture: DemonCustomMaterialTexture {
        type: DemonCustomMaterialTexture.Diffuse
        enabled: true
        image: DemonImage {
            tilingModeHorizontal: DemonImage.Repeat
            tilingModeVertical: DemonImage.Repeat
            source: "maps/walnut.png"
        }
    }
    property DemonCustomMaterialTexture bump_texture: DemonCustomMaterialTexture {
        type: DemonCustomMaterialTexture.Bump
        enabled: true
        image: DemonImage {
            tilingModeHorizontal: DemonImage.Repeat
            tilingModeVertical: DemonImage.Repeat
            source: "maps/walnut_bump.png"
        }
    }
    property DemonCustomMaterialTexture reflect_texture: DemonCustomMaterialTexture {
        type: DemonCustomMaterialTexture.Specular
        enabled: true
        image: DemonImage {
            tilingModeHorizontal: DemonImage.Repeat
            tilingModeVertical: DemonImage.Repeat
            source: "maps/walnut_spec.png"
        }
    }

    DemonCustomMaterialShader {
        id: walnutMatteFragShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/walnutMatte.frag"
    }

    passes: [ DemonCustomMaterialPass {
            shaders: walnutMatteFragShader
        }
    ]
}
