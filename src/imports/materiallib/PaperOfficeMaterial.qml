import QtQuick 2.12
import QtDemon 1.0

DemonCustomMaterial {
    // These properties names need to match the ones in the shader code!
    property bool uShadowMappingEnabled: false
    property real bump_amount: 0.5
    property real uTranslucentFalloff: 0.0
    property real uDiffuseLightWrap: 0.0
    property real uOpacity: 100.0
    property real transmission_weight: 0.2
    property real reflection_weight: 0.8
    property vector2d texture_tiling: Qt.vector2d(1.0, 1.0)
    property vector3d paper_color: Qt.vector3d(0.531, 0.531, 0.531)

    shaderInfo: DemonCustomMaterialShaderInfo {
        version: "330"
        type: "GLSL"
        shaderKey: DemonCustomMaterialShaderInfo.Transmissive | DemonCustomMaterialShaderInfo.Diffuse
        layers: 1
    }

    property DemonCustomMaterialTexture uBakedShadowTexture: DemonCustomMaterialTexture {
            enabled: uShadowMappingEnabled
            type: DemonCustomMaterialTexture.LightmapShadow
            image: DemonImage {
                id: shadowImage
                source: "maps/shadow.jpg"
            }
    }
    property DemonCustomMaterialTexture diffuse_texture: DemonCustomMaterialTexture {
        // <Property formalName="Diffuse Map" name="diffuse_texture" description="Diffuse texture of the material" type="Texture" filter="linear" minfilter="linearMipmapLinear" clamp="repeat" usage="diffuse" default="./maps/materials/paper_diffuse.png" category="Textures"/>
        type: DemonCustomMaterialTexture.Diffuse
        enabled: true
        image: DemonImage {
            tilingmodehorz: DemonImage.Repeat
            tilingmodevert: DemonImage.Repeat
            source: "maps/paper_diffuse.jpg"
        }
    }
    property DemonCustomMaterialTexture bump_texture: DemonCustomMaterialTexture {
        // <Property formalName="Bump Map" name="bump_texture" description="Bump texture of the material" type="Texture" filter="linear" minfilter="linearMipmapLinear" clamp="repeat" usage="bump" default="./maps/materials/art_paper_normal.png" category="Textures"/>
        type: DemonCustomMaterialTexture.Bump
        enabled: true
        image: DemonImage {
            tilingmodehorz: DemonImage.Repeat
            tilingmodevert: DemonImage.Repeat
            source: "maps/paper_diffuse.jpg"
        }
    }
    property DemonCustomMaterialTexture transmission_texture: DemonCustomMaterialTexture {
        // <Property formalName="Transmission Map" name="transmission_texture" description="Transmission texture of the material" type="Texture" filter="linear" minfilter="linearMipmapLinear" clamp="repeat" usage="transmission" default="./maps/materials/paper_trans.png" category="Textures"/>
        type: DemonCustomMaterialTexture.Bump
        enabled: true
        image: DemonImage {
            tilingmodehorz: DemonImage.Repeat
            tilingmodevert: DemonImage.Repeat
            source: "maps/paper_trans.jpg"
        }
    }

    DemonCustomMaterialShader {
        id: paperOfficeFragShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/paperOffice.frag"
    }

    passes: [ DemonCustomMaterialPass {
            shader: paperOfficeFragShader
        }
    ]
}
