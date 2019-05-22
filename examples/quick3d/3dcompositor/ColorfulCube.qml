import QtQuick 2.12
import QtDemon 1.0

DemonNode {

    property vector3d rotationFrom: Qt.vector3d(0, 0, 0)
    property vector3d rotationTo: Qt.vector3d(0, 360, 360)

    DemonModel {
        id: cube
        source: "#Cube"
        materials: [defaultMaterial]
        //rotation: Qt.vector3d(0.4, 0.4, 0.4)

        SequentialAnimation on rotation {
            loops: Animation.Infinite
            PropertyAnimation { duration: 2000; to: rotationTo; from: rotationFrom }
            PropertyAnimation { duration: 2000; to: rotationFrom; from: rotationTo }
        }
    }

    DemonDefaultMaterial {
        id: defaultMaterial

        diffuseColor: "red"
        SequentialAnimation on diffuseColor {
            loops: Animation.Infinite
            ColorAnimation { duration: 2000; to: Qt.rgba(255, 0, 0, 255); from: Qt.rgba(0, 0, 255, 255) }
            ColorAnimation { duration: 2000; to: Qt.rgba(0, 0, 255, 255); from: Qt.rgba(255, 0, 0, 255) }
        }
    }

}
