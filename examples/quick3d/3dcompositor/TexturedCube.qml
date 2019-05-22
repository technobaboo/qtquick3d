import QtQuick 2.12
import QtDemon 1.0

DemonNode {
    DemonModel {
        source: "#Cube"

        materials: [
            DemonDefaultMaterial {
                id: texturedCubeMaterial
                diffuseMap: DemonImage {
                    id: cubeTexture
                    source: "texture.png"
                }
            }
        ]
    }
}
