import QtQuick 2.12
import QtDemon 1.0

DemonNode {
    id: weirdShape

    property alias color: weirdShapeMaterial.diffuseColor

    DemonModel {
        source: "weirdShape.mesh"
        scale: Qt.vector3d(100, 100, 100)
        rotation: Qt.vector3d(90, 0, 0)

        materials: [weirdShapeMaterial]

        DemonDefaultMaterial {
            id: weirdShapeMaterial
            diffuseColor: "purple"
        }
    }
}
