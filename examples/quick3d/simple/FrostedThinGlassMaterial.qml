import QtQuick 2.12
import QtDemon 1.0

DemonCustomMaterial {

    //    <Property formalName="Glass Bump map" name="glass_bump" description="Additional bump map for surface" type="Texture" clamp="repeat" category="Material"/>
    //    <Property formalName="Gradient1D Map" description="Gradient texture of the material" hidden="True" name="randomGradient1D" type="Texture" filter="linear" minfilter="linearMipmapLinear" clamp="repeat" usage="gradient" default="./maps/materials/randomGradient1D.jpg"/>
    //    <Property formalName="Gradient2D Map" description="Gradient texture of the material" hidden="True" name="randomGradient2D" type="Texture" filter="linear" minfilter="linearMipmapLinear" clamp="repeat" usage="gradient" default="./maps/materials/randomGradient2D.jpg"/>
    //    <Property formalName="Gradient3D Map" description="Gradient texture of the material" hidden="True" name="randomGradient3D" type="Texture" filter="linear" minfilter="linearMipmapLinear" clamp="repeat" usage="gradient" default="./maps/materials/randomGradient3D.jpg"/>
    //    <Property formalName="Gradient4D Map" description="Gradient texture of the material" hidden="True" name="randomGradient4D" type="Texture" filter="linear" minfilter="linearMipmapLinear" clamp="repeat" usage="gradient" default="./maps/materials/randomGradient4D.jpg"/>

    // These properties names need to match the ones in the shader code!
    property real roughness: 1.0
    property real blur_size: 8.0
    property real refract_depth: 5
    property bool uEnvironmentMappingEnabled: true
    property bool uShadowMappingEnabled: false
    property real glass_bfactor: 0.0
    property bool glass_binside: false
    property real uFresnelPower: 1.0
    property real reflectivity_amount: 0.1
    property real glass_ior: 1.1
    property real intLightFall: 2.0
    property real intLightRot: 0.0
    property real intLightBrt: 0.0
    property real bumpScale: 0.5
    property int bumpBands: 1
    property vector3d bumpCoords: Qt.vector3d(1.0, 1.0, 1.0)
    property vector2d intLightPos: Qt.vector2d(0.5, 0.0)
    property vector3d glass_color: Qt.vector3d(0.9, 0.9, 0.9)
    property vector3d intLightCol: Qt.vector3d(0.9, 0.9, 0.9)
    hasTransparency: true

    shaderInfo: DemonCustomMaterialShaderInfo {
        version: "330"
        type: "GLSL"
        shaderKey: DemonCustomMaterialShaderInfo.Refraction | DemonCustomMaterialShaderInfo.Glossy
        layers: 1
    }

    textures: [ DemonCustomMaterialTexture {
            id: uEnvironmentTexture
            type: DemonCustomMaterialTexture.Environment
            name: "uEnvironmentTexture"
            enabled: uEnvironmentMappingEnabled
            image: DemonImage {
                id: envImage
                source: "maps/spherical_checker.png"
            }
        }, DemonCustomMaterialTexture {
            id: uBakedShadowTexture
            type: DemonCustomMaterialTexture.LightmapShadow
            name: "uBakedShadowTexture"
            enabled: uShadowMappingEnabled
            image: DemonImage {
                id: shadowImage
                source: "maps/shadow.png"
            }
        }, DemonCustomMaterialTexture {
            id: randomGradient1D
            type: DemonCustomMaterialTexture.Gradient
            name: "randomGradient1D"
            // hidden = true
            image: DemonImage {
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/randomGradient1D.png"
            }
        }, DemonCustomMaterialTexture {
            id: randomGradient2D
            type: DemonCustomMaterialTexture.Gradient
            name: "randomGradient2D"
            // hidden = true
            image: DemonImage {
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/randomGradient2D.png"
            }
        }, DemonCustomMaterialTexture {
            id: randomGradient3D
            type: DemonCustomMaterialTexture.Gradient
            name: "randomGradient3D"
            // hidden = true
            image: DemonImage {
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/randomGradient3D.png"
            }
        }, DemonCustomMaterialTexture {
            id: randomGradient4D
            type: DemonCustomMaterialTexture.Gradient
            name: "randomGradient4D"
            // hidden = true
            image: DemonImage {
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/randomGradient4D.png"
            }
        } ]

    shaders: [ DemonCustomMaterialShader {
            stage: DemonCustomMaterialShader.Fragment
            shader: "frostedThinGlass.frag"
        }, DemonCustomMaterialShader {
            stage: DemonCustomMaterialShader.Fragment
            shader: "frostedThinGlassPreBlur.frag"
        }, DemonCustomMaterialShader {
            stage: DemonCustomMaterialShader.Fragment
            shader: "frostedThinGlassBlurX.frag"
        }, DemonCustomMaterialShader {
            stage: DemonCustomMaterialShader.Fragment
            shader: "frostedThinGlassBlurY.frag"
        }
    ]

    passes: [ DemonCustomMaterialPass {
            // NOOP
            // output => dummy_buffer
            commands: [ DemonCustomMaterialBufferBlit {
                    destination: "frame_buffer"
                }
            ]
        }, DemonCustomMaterialPass {
            // PREBLUR
            // output => temp_buffer
            commands: [ DemonCustomMaterialBufferInput {
                    bufferName: "frame_buffer"
                    param: "OriginBuffer"
                }
            ]
        }, DemonCustomMaterialPass {
            // BLURX
            // output => temp_blurX
            commands: [ DemonCustomMaterialBufferInput {
                    bufferName: "temp_buffer"
                    param: "BlurBuffer"
                }
            ]
        }, DemonCustomMaterialPass {
            // BLURY
            // output => temp_blurY
            commands: [ DemonCustomMaterialBufferInput {
                    bufferName: "temp_blurX"
                    param: "BlurBuffer"
                }, DemonCustomMaterialBufferInput {
                    bufferName: "temp_buffer"
                    param: "OriginBuffer"
                }
            ]
        }, DemonCustomMaterialPass {
            // MAIN
            // output => dummy_buffer
            commands: [DemonCustomMaterialBufferInput {
                    bufferName: "temp_blurY"
                    param: "refractiveTexture"
                }, DemonCustomMaterialBlending {
                    srcBlending: DemonCustomMaterialBlending.SrcAlpha
                    destBlending: DemonCustomMaterialBlending.OneMinusSrcAlpha
                }
            ]
        }
    ]
}
