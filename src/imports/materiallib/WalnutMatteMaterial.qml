import QtQuick 2.12
import QtDemon 1.0

DemonCustomMaterial {
    // These properties names need to match the ones in the shader code!
    property bool uEnvironmentMappingEnabled: true
    property bool uShadowMappingEnabled: false
    property real bump_amount: 0.4
    property real material_ior: 1.5
    property real roughness: 0.25
    property vector2d texture_tiling: Qt.vector2d(10.0, 7.0)

    shaderInfo: DemonCustomMaterialShaderInfo {
        version: "330"
        type: "GLSL"
        shaderKey: DemonCustomMaterialShaderInfo.Glossy | DemonCustomMaterialShaderInfo.Diffuse
        layers: 2
    }

    property DemonCustomMaterialTexture uEnvironmentTexture: DemonCustomMaterialTexture {
        type: DemonCustomMaterialTexture.Environment
        enabled: true
        image: DemonImage {
            tilingmodehorz: DemonImage.Repeat
            tilingmodevert: DemonImage.Repeat
            source: "maps/spherical_checker.jpg"
        }
    }
    property DemonCustomMaterialTexture uBakedShadowTexture: DemonCustomMaterialTexture {
            enabled: uShadowMappingEnabled
            type: DemonCustomMaterialTexture.LightmapShadow
            image: DemonImage {
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                id: shadowImage
                source: "maps/shadow.jpg"
            }
    }
    property DemonCustomMaterialTexture diffuse_texture: DemonCustomMaterialTexture {
        // <Property formalName="Diffuse Map" name="diffuse_texture" description="Diffuse texture of the material" type="Texture" filter="linear" minfilter="linearMipmapLinear" clamp="repeat" usage="diffuse" default="./maps/materials/walnut.png" category="Material"/>
        type: DemonCustomMaterialTexture.Diffuse
        enabled: true
        image: DemonImage {
            tilingmodehorz: DemonImage.Repeat
            tilingmodevert: DemonImage.Repeat
            source: "maps/walnut.jpg"
        }
    }
    property DemonCustomMaterialTexture bump_texture: DemonCustomMaterialTexture {
        // <Property formalName="Bump Map" name="bump_texture" description="Bump texture of the material" type="Texture" filter="linear" minfilter="linearMipmapLinear" clamp="repeat" usage="bump" default="./maps/materials/walnut_bump.png" category="Material"/>
        type: DemonCustomMaterialTexture.Bump
        enabled: true
        image: DemonImage {
            tilingmodehorz: DemonImage.Repeat
            tilingmodevert: DemonImage.Repeat
            source: "maps/walnut_bump.jpg"
        }
    }
    property DemonCustomMaterialTexture reflect_texture: DemonCustomMaterialTexture {
        // <Property formalName="Reflectivity Map" name="reflect_texture" description="Reflectivity texture for the material" type="Texture" filter="linear" minfilter="linearMipmapLinear" clamp="repeat" usage="specular" default="./maps/materials/walnut_spec.png" category="Material"/>
        type: DemonCustomMaterialTexture.Specular
        enabled: true
        image: DemonImage {
            tilingmodehorz: DemonImage.Repeat
            tilingmodevert: DemonImage.Repeat
            source: "maps/walnut_spec.jpg"
        }
    }

    DemonCustomMaterialShader {
        id: walnutMatteFragShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/walnutMatte.frag"
    }

    passes: [ DemonCustomMaterialPass {
            shader: walnutMatteFragShader
        }
    ]
}
