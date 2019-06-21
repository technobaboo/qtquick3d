import QtQuick 2.12
import QtDemon 1.0

DemonCustomMaterial {
    // These properties names need to match the ones in the shader code!
    property bool uEnvironmentMappingEnabled: true
    property bool uShadowMappingEnabled: false
    property real material_ior: 1.5
    property real glossy_weight: 1.0
    property real roughness: 0.15
    property vector3d porcelain_color: Qt.vector3d(0.92, 0.92, 0.92)

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
                source: "maps/shadow.png"
            }
    }

    DemonCustomMaterialShader {
        id: porcelainFragShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/porcelain.frag"
    }

    passes: [ DemonCustomMaterialPass {
            shaders: porcelainFragShader
        }
    ]
}
