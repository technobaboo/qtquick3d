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

        DemonLight {
            id: directionalLight
            lightType: DemonLight.Directional
        }

        DemonCamera {
            id: camera
            position: Qt.vector3d(0, 0, -200)
        }

        DemonModel {
            id: cube
            source: "#Cube"
            materials: [defaultMaterial]
            rotation: Qt.vector3d(0.4, 0.4, 0.4)


            SequentialAnimation on rotation {
                loops: Animation.Infinite
                PropertyAnimation { duration: 2000; to: Qt.vector3d(0, 2, 2) }
            }
        }
    }

    DemonDefaultMaterial {
        id: defaultMaterial
    }


}
