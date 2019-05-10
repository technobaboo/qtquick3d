import QtQuick 2.12
import QtDemon 1.0

DemonCustomMaterial {
    // These properties names need to match the ones in the shader code!
    property bool uEnvironmentMappingEnabled: false
    property bool uShadowMappingEnabled: false
    property real material_ior: 15.0
    property real glossy_weight: 0.5
    property real roughness: 0.25
    property real bump_amount: 2.0
    property vector2d texture_tiling: Qt.vector2d(3.0, 3.0)

    shaderInfo: DemonCustomMaterialShaderInfo {
        version: "330"
        type: "GLSL"
        shaderKey: DemonCustomMaterialShaderInfo.Cutout | DemonCustomMaterialShaderInfo.Glossy | DemonCustomMaterialShaderInfo.Diffuse
        layers: 2
    }

    property DemonCustomMaterialTexture uEnvironmentTexture: DemonCustomMaterialTexture {
            enabled: uEnvironmentMappingEnabled
            type: DemonCustomMaterialTexture.Environment
            image: DemonImage {
                id: envImage
                source: "maps/spherical_checker.jpg"
            }
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
        //    <Property formalName="Diffuse Map" name="diffuse_texture" description="Diffuse texture of the material" type="Texture" filter="linear" minfilter="linearMipmapLinear" clamp="repeat" usage="diffuse" default="./maps/materials/cyclone_mesh_fencing.png" category="Material"/>
        type: DemonCustomMaterialTexture.Diffuse
        enabled: true
        image: DemonImage {
            tilingmodehorz: DemonImage.Repeat
            tilingmodevert: DemonImage.Repeat
            source: "maps/cyclone_mesh_fencing.jpg"
        }
    }

    property DemonCustomMaterialTexture bump_texture: DemonCustomMaterialTexture {
        //    <Property formalName="Bump Map" name="bump_texture" description="Bump texture of the material" type="Texture" filter="linear" minfilter="linearMipmapLinear" clamp="repeat" usage="bump" default="./maps/materials/cyclone_mesh_fencing_normal.png" category="Material"/>
        type: DemonCustomMaterialTexture.Bump
        enabled: true
        image: DemonImage {
            tilingmodehorz: DemonImage.Repeat
            tilingmodevert: DemonImage.Repeat
            source: "maps/cyclone_mesh_fencing_normal.jpg"
        }
    }

    DemonCustomMaterialShader {
        id: meshFenceFragShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/meshFence.frag"
    }

    passes: [ DemonCustomMaterialPass {
            shader: meshFenceFragShader
        }
    ]
}
