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
            id: camera2

            x: 300
            z: 300
            //rotation: Qt.vector3d(0, 45, 0)
            SequentialAnimation on rotation {
                loops: Animation.Infinite
                PropertyAnimation { duration: 5000; to: Qt.vector3d(0, 360, 0); from: Qt.vector3d(0, 0, 0) }
            }
        }

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
        id: topLeftView
        anchors.top: parent.top
        anchors.left: parent.left
        width: parent.width * 0.5
        height: parent.height * 0.5
        camera: camera3
        scene: standAloneScene
    }

    DemonView3D {
        id: topRightView
        anchors.top: parent.top
        anchors.right: parent.right
        width: parent.width * 0.5
        height: parent.height * 0.5
        camera: camera4
        scene: standAloneScene
    }

    DemonView3D {
        id: bottomLeftView
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        width: parent.width * 0.5
        height: parent.height * 0.5
        scene: standAloneScene
        camera: camera1

        DemonCamera {
            id: camera1
            z: 600
            projectionMode: DemonCamera.Orthographic
            rotation: Qt.vector3d(0, -180, 0)
        }

        DemonLight {
            id: debugLight
            ambientColor: Qt.rgba(0.5, 0.5, 0.5, 1.0);
        }
    }

    DemonView3D {
        id: bottomRightView
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: parent.width * 0.5
        height: parent.height * 0.5
        scene: standAloneScene
        camera: camera2
    }
}
