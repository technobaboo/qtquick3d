import QtQuick 2.12
import QtDemon 1.0

DemonCustomMaterial {
    property bool uEnvironmentMappingEnabled: true
    property bool uShadowMappingEnabled: false
    property real roughness: 0.3
    property vector3d base_color: Qt.vector3d(0.7, 0.7, 0.7)

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

    DemonCustomMaterialShader {
        id: aluminumAnodizedShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/aluminumAnodized.frag"
    }

    passes: [
        DemonCustomMaterialPass {
            shader: aluminumAnodizedShader
        }
    ]
}
