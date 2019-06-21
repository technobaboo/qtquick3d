import QtQuick 2.12
import QtDemon 1.0

DemonCustomMaterial {
    // These properties names need to match the ones in the shader code!
    property bool uEnvironmentMappingEnabled: false
    property bool uShadowMappingEnabled: false
    property real material_ior: 15.0
    property real glossy_weight: 0.5
    property real roughness: 0.25
    property real bump_amount: 2.0
    property vector2d texture_tiling: Qt.vector2d(3.0, 3.0)

    shaderInfo: DemonCustomMaterialShaderInfo {
        version: "330"
        type: "GLSL"
        shaderKey: DemonCustomMaterialShaderInfo.Cutout | DemonCustomMaterialShaderInfo.Glossy | DemonCustomMaterialShaderInfo.Diffuse
        layers: 2
    }

    property DemonCustomMaterialTexture uEnvironmentTexture: DemonCustomMaterialTexture {
            enabled: uEnvironmentMappingEnabled
            type: DemonCustomMaterialTexture.Environment
            image: DemonImage {
                id: envImage
                source: "maps/spherical_checker.png"
            }
    }
    property DemonCustomMaterialTexture uBakedShadowTexture: DemonCustomMaterialTexture {
            enabled: uShadowMappingEnabled
            type: DemonCustomMaterialTexture.LightmapShadow
            image: DemonImage {
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
            source: "maps/cyclone_mesh_fencing.png"
        }
    }

    property DemonCustomMaterialTexture bump_texture: DemonCustomMaterialTexture {
        type: DemonCustomMaterialTexture.Bump
        enabled: true
        image: DemonImage {
            tilingModeHorizontal: DemonImage.Repeat
            tilingModeVertical: DemonImage.Repeat
            source: "maps/cyclone_mesh_fencing_normal.png"
        }
    }

    DemonCustomMaterialShader {
        id: meshFenceFragShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/meshFence.frag"
    }

    passes: [ DemonCustomMaterialPass {
            shaders: meshFenceFragShader
        }
    ]
}
