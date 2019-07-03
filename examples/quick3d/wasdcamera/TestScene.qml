import QtQuick 2.12
import QtDemon 1.0

DemonNode {
    id: rootItem
    property DemonCamera activeCamera: camera

    DemonCamera {
        id: camera
        y: 200
        z: -600
    }

    DemonLight {
        id: light
        rotation: Qt.vector3d(0, 0, 0)
    }

    DemonLight {
        lightType: DemonLight.Point

        z: 200
    }

    DemonNode {
        z: 45
        rotation: Qt.vector3d(90, 0, 0)


        DemonModel {
            id: cone
            source: "#Cone"
            materials: [
                DemonDefaultMaterial {
                    id: coneMaterial
                    diffuseColor: "pink"
                }
            ]

        }
    }
}
