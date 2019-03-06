import QtQuick 2.12
import QtDemon 1.0

DemonNode {
    DemonModel {
        source: "#Cube"

        materials: [texturedCubeMaterial]
    }

    DemonDefaultMaterial {
        id: texturedCubeMaterial
        diffuseMap: cubeTexture
    }

    DemonImage {
        id: cubeTexture
        source: "texture.png"
    }

}
