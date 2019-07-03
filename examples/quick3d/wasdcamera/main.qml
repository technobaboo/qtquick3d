import QtQuick 2.12
import QtQuick.Window 2.12
import QtDemon 1.0
import QtDemonHelpers 1.0

Window {
    id: window
    width: 1280
    height: 720
    visible: true

    DemonView3D {
        id: sceneView
        anchors.fill: parent
        camera: sceneContent.activeCamera
        environment: DemonSceneEnvironment {
            backgroundMode: DemonSceneEnvironment.Color
            clearColor: "black"
        }

        AxisHelper {
            enableAxisLines: true
            enableXZGrid: true
            enableYZGrid: false
            enableXYGrid: false
        }

        TestScene {
            id: sceneContent
        }
    }

    WasdController {
        id: wasdController
        controlledObject: sceneContent.activeCamera
    }
}
