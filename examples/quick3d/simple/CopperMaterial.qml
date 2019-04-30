import QtQuick 2.12
import QtDemon 1.0

DemonCustomMaterial {
    // These properties names need to match the ones in the shader code!
    property bool uEnvironmentMappingEnabled: false
    property bool uShadowMappingEnabled: false
    property real roughness: 0.0
    property vector3d metal_color: Qt.vector3d(0.805, 0.395, 0.305)

    shaderInfo: DemonCustomMaterialShaderInfo {
        version: "330"
        type: "GLSL"
        shaderKey: DemonCustomMaterialShaderInfo.Glossy
        layers: 1
    }

    textures: [ DemonCustomMaterialTexture {
            id: uEnvironmentTexture
            enabled: uEnvironmentMappingEnabled
            type: DemonCustomMaterialTexture.Environment
            name: "uEnvironmentTexture"
            image: DemonImage {
                id: envImage
                source: "maps/spherical_checker.png"
            }
    }, DemonCustomMaterialTexture {
            id: uBakedShadowTexture
            enabled: uShadowMappingEnabled
            type: DemonCustomMaterialTexture.LightmapShadow
            name: "uBakedShadowTexture"
            image: DemonImage {
                id: shadowImage
                source: "maps/shadow.png"
            }
    } ]

    shaders: DemonCustomMaterialShader {
        stage: DemonCustomMaterialShader.Fragment
        shader: "copper.frag"
    }
}