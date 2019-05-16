import QtQuick 2.12
import QtDemon 1.0

DemonCustomMaterial {
    property bool uEnvironmentMappingEnabled: true
    property bool uShadowMappingEnabled: false
    property vector3d tiling: Qt.vector3d(3, 3, 3)
    property real brushing_strength: 0.5
    property real reflection_stretch: 0.5
    property vector3d metal_color: Qt.vector3d(0.95, 0.95, 0.95)
    property real bump_amount: 0.4

//                                         <Property formalName="Roughness Map U" name="roughness_texture_u" description="Horizontal roughness texture" type="Texture" filter="linear" minfilter="linearMipmapLinear" clamp="repeat" usage="roughness" default="./maps/materials/brushed_full_contrast.png" category="Material"/>
//                                         <Property formalName="Roughness Map V" name="roughness_texture_v" description="Vertical roughness texture" type="Texture" filter="linear" minfilter="linearMipmapLinear" clamp="repeat" usage="roughness" default="./maps/materials/brushed_full_contrast.png" category="Material"/>


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

    property DemonCustomMaterialTexture brush_texture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Unknown
            enabled: true
            image: DemonImage {
                id: brushTexture
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/brushed_full_contrast.png"
            }
    }

    property DemonCustomMaterialTexture roughness_texture_u: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Unknown
            enabled: true
            image: DemonImage {
                id: roughnessUTexture
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/brushed_full_contrast.png"
            }
    }

    property DemonCustomMaterialTexture roughness_texture_v: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Unknown
            enabled: true
            image: DemonImage {
                id: roughnessVTexture
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/brushed_full_contrast.png"
            }
    }


    property DemonCustomMaterialTexture bump_texture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Bump
            enabled: true
            image: DemonImage {
                id: bumpTexture
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/brushed_a.png"
            }
    }

    DemonCustomMaterialShader {
        id: aluminumBrushedFragShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/aluminumBrushed.frag"
    }

    passes: [
        DemonCustomMaterialPass {
            shader: aluminumBrushedFragShader
        }
    ]
}
