import QtQuick 2.12
import QtDemon 1.0

DemonCustomMaterial {
    // These properties names need to match the ones in the shader code!
    property bool uEnvironmentMappingEnabled: false
    property bool uShadowMappingEnabled: false
    property real uFresnelPower: 1.0
    property real uMinOpacity: 0.5
    property real reflectivity_amount: 1.0
    property real glass_ior: 1.1
    property vector3d glass_color: Qt.vector3d(0.9, 0.9, 0.9)
    hasTransparency: true
//    property alias environmentTexture: envImage.source
//    property alias shadowTexture: shadowImage.source

    shaderInfo: DemonCustomMaterialShaderInfo {
        version: "330"
        type: "GLSL"
        shaderKey: DemonCustomMaterialShaderInfo.Transparent | DemonCustomMaterialShaderInfo.Glossy
        layers: 1
    }

    property DemonCustomMaterialTexture uEnvironmentTexture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Environment
            name: "uEnvironmentTexture"
            enabled: uEnvironmentMappingEnabled
            image: DemonImage {
                id: envImage
                source: "maps/spherical_checker.png"
            }
    }
    property DemonCustomMaterialTexture uBakedShadowTexture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.LightmapShadow
            name: "uBakedShadowTexture"
            enabled: uShadowMappingEnabled
            image: DemonImage {
                id: shadowImage
                source: "maps/shadow.png"
            }
    }

    DemonCustomMaterialShader {
        id: simpleGlassFragShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/simpleGlass.frag"
    }

    passes: [ DemonCustomMaterialPass {
            shader: simpleGlassFragShader
            commands: [ DemonCustomMaterialBlending {
                    srcBlending: DemonCustomMaterialBlending.SrcAlpha
                    destBlending: DemonCustomMaterialBlending.OneMinusSrcAlpha
                }, DemonCustomMaterialRenderState {
                    renderState: DemonCustomMaterialRenderState.CullFace
                }
            ]
        }
    ]
}
