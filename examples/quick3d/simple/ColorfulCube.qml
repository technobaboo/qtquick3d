import QtQuick 2.12
import QtDemon 1.0

DemonNode {

    DemonModel {
        id: cube
        source: "#Cube"
        materials: [defaultMaterial]
        //rotation: Qt.vector3d(0.4, 0.4, 0.4)

//        SequentialAnimation on rotation {
//            loops: Animation.Infinite
//            PropertyAnimation { duration: 2000; to: Qt.vector3d(0, 2, 2); from: Qt.vector3d(0, 0, 0) }
//            PropertyAnimation { duration: 2000; to: Qt.vector3d(0, 0, 0); from: Qt.vector3d(0, 2, 2) }
//        }
    }

    DemonDefaultMaterial {
        id: defaultMaterial

        SequentialAnimation on diffuseColor {
            loops: Animation.Infinite
            ColorAnimation { duration: 2000; from: Qt.rgba(255, 0, 0, 255); to: Qt.rgba(0, 0, 255, 255) }
            ColorAnimation { duration: 2000; from: Qt.rgba(0, 0, 255, 255); to: Qt.rgba(255, 0, 0, 255) }
        }
    }

}
