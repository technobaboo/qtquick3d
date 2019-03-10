import QtQuick 2.12
import QtQuick.Window 2.12
import QtDemon 1.0

Window {
    id: window
    width: 1280
    height: 720
    visible: true

    DemonView3D {
        id: leftView
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        width: parent.width * 0.5
        camera: camera

        DemonCamera {
            id: camera1
            z: -600
        }

//        DemonCamera {
//            id: camera2
//        }

        DemonLight {
            id: light
        }

        DemonModel {
            source: "#Cube"
            materials: [
                DemonDefaultMaterial {
                    id: cubeMaterial
                }
            ]
        }
    }

//    DemonView3D {
//        id: rightView
//        anchors.top: parent.top
//        anchors.right: parent.right
//        anchors.bottom: parent.bottom
//        width: parent.width * 0.5
//        scene: leftView.scene
//        camera: camera2
//    }

}
