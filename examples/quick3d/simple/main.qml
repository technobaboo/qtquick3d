import QtQuick 2.12
import QtDemon 1.0

DemonWindow {
    id: window
    width: 1280
    height: 720
    color: "blue"
    visible: true

    DemonLayer {
        id: layer1
        clearColor: "green"
        backgroundMode: DemonLayer.Color
        width: 100
        height: 100
        activeCamera: camera

        DemonNode {
            id: cameraSpinner
            position: Qt.vector3d(0, 0, 0);

            // Light always points the same direction as camera
            DemonLight {
                id: directionalLight
                lightType: DemonLight.Directional
            }

            DemonCamera {
                id: camera
                position: Qt.vector3d(0, 0, -600)
            }

            SequentialAnimation on rotation {
                loops: Animation.Infinite
                PropertyAnimation { duration: 5000; to: Qt.vector3d(0, 360, 0); from: Qt.vector3d(0, 0, 0) }
            }
        }


        ColorfulCube {
            id: cube1
            position: Qt.vector3d(-200, 0, 0);
        }

        ColorfulCube {
            id: cube2
            position: Qt.vector3d(200, 0, 0);
        }

    }
}
