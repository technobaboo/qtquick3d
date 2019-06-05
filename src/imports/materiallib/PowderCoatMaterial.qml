import QtQuick 2.12
import QtDemon 1.0

DemonCustomMaterial {    
    // These properties names need to match the ones in the shader code!
    property bool uEnvironmentMappingEnabled: true
    property bool uShadowMappingEnabled: false
    property real roughness: 0.1
    property real material_ior: 5.0
    property real bump_factor: 2.0
    property real glossy_weight: 0.5
    property real reflectivity: 0.1
    property real diffuse_weight: 1.0
    property vector3d texture_scaling: Qt.vector3d(3.0, 3.0, 3.0)
    property vector3d powdercoat_diffuse_color: Qt.vector3d(0.046, 0.046, 0.054)

    shaderInfo: DemonCustomMaterialShaderInfo {
        version: "330"
        type: "GLSL"
        shaderKey: DemonCustomMaterialShaderInfo.Glossy | DemonCustomMaterialShaderInfo.Diffuse
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
    property DemonCustomMaterialTexture powdercoat_bump_texture: DemonCustomMaterialTexture {
        type: DemonCustomMaterialTexture.Bump
        enabled: true
        image: DemonImage {
            tilingModeHorizontal: DemonImage.Repeat
            tilingModeVertical: DemonImage.Repeat
            source: "maps/powdercoat_bump_01.png"
        }
    }

    DemonCustomMaterialShader {
        id: powderCoatFragShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/powderCoat.frag"
    }

    passes: [ DemonCustomMaterialPass {
            shader: powderCoatFragShader
        }
    ]
}
