import QtQuick 2.12
import QtQuick.Window 2.12
import QtDemon 1.0
import QtQuick.Controls 2.4

Window {
    id: window
    width: 1280
    height: 720
    visible: true
    color: "black"

    DemonNode {
        id: standAloneScene
        DemonCamera {
            id: camera3
            z: -600
        }

        DemonCamera {
            id: camera4

            x: 200
            rotation: Qt.vector3d(0, -90, 0)
        }

        DemonLight {
            id: light2
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0);
        }

        DemonModel {
            source: "#Cube"
            materials: [
                DemonDefaultMaterial {
                    id: cubeMaterial2
                    diffuseColor: "salmon"
                }
            ]
            SequentialAnimation on rotation {
                loops: Animation.Infinite
                PropertyAnimation { duration: 5000; to: Qt.vector3d(0, 360, 0); from: Qt.vector3d(0, 0, 0) }
            }
        }
    }

        DemonView3D {
            id: leftView
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            width: parent.width * 0.5
            camera: camera3
            scene: standAloneScene
        }

        DemonView3D {
            id: rightView
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            width: parent.width * 0.5
            camera: camera4
            scene: standAloneScene
        }

//    DemonView3D {
//        id: leftView
//        anchors.top: parent.top
//        anchors.left: parent.left
//        anchors.bottom: parent.bottom
//        width: parent.width * 0.5
//        camera: camera1
//        environment: DemonSceneEnvironment {
//            multisampleAAMode: DemonSceneEnvironment.X2
//        }

//        DemonCamera {
//            id: camera1
//            z: -600
//        }

//        DemonCamera {
//            id: camera2

//            x: 200
//            rotation: Qt.vector3d(0, -90, 0)
//        }

//        DemonLight {
//            id: light
//            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0);
//        }

//        DemonModel {
//            source: "#Cube"
//            materials: [
//                DemonDefaultMaterial {
//                    id: cubeMaterial

//                }
//            ]
//            SequentialAnimation on rotation {
//                loops: Animation.Infinite
//                PropertyAnimation { duration: 5000; to: Qt.vector3d(0, 360, 0); from: Qt.vector3d(0, 0, 0) }
//            }
//        }
//    }

//    Row {
//        Button {
//            text: "camera1"
//            onClicked: {
//                leftView.camera = camera1
//            }
//        }
//        Button {
//            text: "camera2"
//            onClicked: {
//                leftView.camera = camera2
//            }
//        }
//    }

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
