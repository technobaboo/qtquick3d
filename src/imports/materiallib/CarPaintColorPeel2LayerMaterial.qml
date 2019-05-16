import QtQuick 2.12
import QtDemon 1.0

DemonCustomMaterial {

    property bool uEnvironmentMappingEnabled: true
    property bool uShadowMappingEnabled: false
    property real coating_ior: 1.5
    property real coat_weight: 1.0
    property real coat_roughness: 0
    property vector3d coat_color: Qt.vector3d(1, 1, 1)

    property real flake_intensity: 0.5
    property real flake_size: 0.002
    property real flake_amount: 0.22
    property real flake_roughness: 0.2
    property vector3d flake_color: Qt.vector3d(1, 0.7, 0.02)

    property vector3d base_color: Qt.vector3d(0.1, 0.001, 0.001)
    property real flake_bumpiness: 0.6
    property real peel_size: 1.0
    property real peel_amount: 0.1

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

    property DemonCustomMaterialTexture randomGradient1D: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Unknown; //Gradient
            image: DemonImage {
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/randomGradient1D.png"
            }
    }
    property DemonCustomMaterialTexture randomGradient2D: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Unknown; //Gradient
            image: DemonImage {
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/randomGradient2D.png"
            }
    }
    property DemonCustomMaterialTexture randomGradient3D: DemonCustomMaterialTexture {
        type: DemonCustomMaterialTexture.Unknown; //Gradient
        image: DemonImage {
            tilingmodehorz: DemonImage.Repeat
            tilingmodevert: DemonImage.Repeat
            source: "maps/randomGradient3D.png"
        }
    }
    property DemonCustomMaterialTexture randomGradient4D: DemonCustomMaterialTexture {
        type: DemonCustomMaterialTexture.Unknown; //Gradient
        image: DemonImage {
            tilingmodehorz: DemonImage.Repeat
            tilingmodevert: DemonImage.Repeat
            source: "maps/randomGradient4D.png"
        }
    }

    DemonCustomMaterialShader {
        id: carpaint2LayerFragShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/carPaintColorPeel2Layer.frag"
    }

    passes: [
        DemonCustomMaterialPass {
            shader: carpaint2LayerFragShader
        }
    ]
}
