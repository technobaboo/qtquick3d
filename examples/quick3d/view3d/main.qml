import QtQuick 2.12
import QtQuick.Window 2.12
import QtDemon 1.0
import QtQuick.Controls 2.4

Window {
    id: window
    width: 1280
    height: 720
    visible: true

    DemonNode {
        id: standAloneScene

        DemonNode {
            id: orbitingCamera
            DemonCamera {
                id: camera1
                z: -600
            }
            SequentialAnimation on rotation {
                loops: Animation.Infinite
                PropertyAnimation { duration: 5000; to: Qt.vector3d(360, 0, 0); from: Qt.vector3d(0, 0, 0) }
            }
        }



        DemonCamera {
            id: camera2
            z: -600
        }

        DemonNode {
            id: orbitingCamera2

            DemonCamera {
                id: camera3

                x: 500
                rotation: Qt.vector3d(0, -90, 0)
            }
            SequentialAnimation on rotation {
                loops: Animation.Infinite
                PropertyAnimation { duration: 5000; to: Qt.vector3d(0, 0, 0); from: Qt.vector3d(0, 360, 0) }
            }
        }


        DemonLight {
            id: light2
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0);
        }

        DemonModel {
            source: "teapot.mesh"
            y: -100
            scale: Qt.vector3d(100, 100, 100)
            materials: [
                DemonDefaultMaterial {
                    id: cubeMaterial2
                    diffuseColor: "salmon"
                }
            ]

            SequentialAnimation on rotation {
                loops: Animation.Infinite
                PropertyAnimation { duration: 5000; to: Qt.vector3d(0, 0, 0); from: Qt.vector3d(0, 360, 0) }
            }
        }
    }

    Rectangle {
        id: topLeft
        anchors.top: parent.top
        anchors.left: parent.left
        width: parent.width * 0.5
        height: parent.height * 0.5
        color: "grey"
        border.color: "black"

        DemonView3D {
            id: topLeftView
            anchors.fill: parent
            scene: standAloneScene
            camera: camerafront

            DemonCamera {
                id: camerafront
                z: -600
                projectionMode: DemonCamera.Orthographic
                rotation: Qt.vector3d(0, 0, 0)
            }
        }

        Label {
            text: "Front"
            anchors.top: parent.top
            anchors.right: parent.right
            color: "limegreen"
            font.pointSize: 14
        }

    }

    Rectangle {
        id: topRight
        anchors.top: parent.top
        anchors.right: parent.right
        width: parent.width * 0.5
        height: parent.height * 0.5
        color: "grey"
        border.color: "black"

        Label {
            text: "Perspective"
            anchors.top: parent.top
            anchors.right: parent.right
            color: "limegreen"
            font.pointSize: 14
        }

        DemonView3D {
            id: topRightView
            anchors.top: controlsContainer.top
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.bottom: parent.bottom;
            camera: camera1
            scene: standAloneScene
        }

        Row {
            id: controlsContainer
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.left: parent.left
            Button {
                text: "Camera 1"
                onClicked: {
                    topRightView.camera = camera1
                }
            }
            Button {
                text: "Camera 2"
                onClicked: {
                    topRightView.camera = camera2
                }
            }

            Button {
                text: "Camera 3"
                onClicked: {
                    topRightView.camera = camera3
                }
            }
        }
    }

    Rectangle {
        id: bottomLeft
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        width: parent.width * 0.5
        height: parent.height * 0.5
        color: "grey"
        border.color: "black"

        DemonView3D {
            id: bottomLeftView
            anchors.fill: parent
            scene: standAloneScene
            camera: cameratop

            DemonCamera {
                id: cameratop
                y: 600
                projectionMode: DemonCamera.Orthographic
                rotation: Qt.vector3d(90, 0, 0)
            }
        }

        Label {
            text: "Top"
            anchors.top: parent.top
            anchors.right: parent.right
            color: "limegreen"
            font.pointSize: 14
        }
    }

    Rectangle {
        id: bottomRight
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: parent.width * 0.5
        height: parent.height * 0.5
        color: "grey"
        border.color: "black"

        DemonView3D {
            id: bottomRightView
            anchors.fill: parent
            scene: standAloneScene
            camera: cameratop

            DemonCamera {
                id: cameraLeft
                x: -600
                projectionMode: DemonCamera.Orthographic
                rotation: Qt.vector3d(0, 90, 0)
            }
        }

        Label {
            text: "Left"
            anchors.top: parent.top
            anchors.right: parent.right
            color: "limegreen"
            font.pointSize: 14
        }
    }
}
