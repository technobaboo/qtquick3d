import QtQuick 2.12
import QtDemon 1.0

DemonNode {
    id: weirdShape

    property alias color: weirdShapeMaterial.diffuseColor

    property real xRotation: Math.random() * (360 - (-360)) + -360;
    property real yRotation: Math.random() * (360 - (-360)) + -360;
    property real zRotation: Math.random() * (360 - (-360)) + -360;

    DemonModel {
        source: "weirdShape.mesh"
        scale: Qt.vector3d(100, 100, 100)
        rotation: Qt.vector3d(90, 0, 0)

        SequentialAnimation on rotation {
            loops: Animation.Infinite
            PropertyAnimation {
                duration: Math.random() * (10000 - 1) + 1
                to: Qt.vector3d(xRotation -  360, yRotation - 360, zRotation - 360);
                from: Qt.vector3d(xRotation, yRotation, zRotation)
            }
        }

        materials: [weirdShapeMaterial]

        DemonDefaultMaterial {
            id: weirdShapeMaterial
            diffuseColor: "purple"
        }
    }
}
