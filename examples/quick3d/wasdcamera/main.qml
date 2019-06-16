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
        focus: true
        camera: sceneContent.activeCamera
        environment: DemonSceneEnvironment {
            backgroundMode: DemonSceneEnvironment.Color
            clearColor: "black"
        }

        Keys.onPressed: {
            switch (event.key) {
            case Qt.Key_W:
            case Qt.Key_Up:
                wasdController.forwardPressed();
                break;
            case Qt.Key_S:
            case Qt.Key_Down:
                wasdController.backPressed();
                break;
            case Qt.Key_A:
            case Qt.Key_Left:
                wasdController.leftPressed();
                break;
            case Qt.Key_D:
            case Qt.Key_Right:
                wasdController.rightPressed();
                break;
            case Qt.Key_R:
            case Qt.Key_PageUp:
                wasdController.upPressed();
                break;
            case Qt.Key_F:
            case Qt.Key_PageDown:
                wasdController.downPressed();
                break;
            }
        }

        Keys.onReleased: {
            switch (event.key) {
            case Qt.Key_W:
            case Qt.Key_Up:
                wasdController.forwardReleased();
                break;
            case Qt.Key_S:
            case Qt.Key_Down:
                wasdController.backReleased();
                break;
            case Qt.Key_A:
            case Qt.Key_Left:
                wasdController.leftReleased();
                break;
            case Qt.Key_D:
            case Qt.Key_Right:
                wasdController.rightReleased();
                break;
            case Qt.Key_R:
            case Qt.Key_PageUp:
                wasdController.upReleased();
                break;
            case Qt.Key_F:
            case Qt.Key_PageDown:
                wasdController.downReleased();
                break;
            }
        }

        AxisHelper {
            enableAxisLines: true
            enableXZGrid: false
            enableYZGrid: false
            enableXYGrid: true
        }

        WasdController {
            id: wasdController
            controlledObject: sceneContent.activeCamera
        }

        TestScene {
            id: sceneContent
        }
    }

    MouseArea {
        anchors.fill: sceneView
        onPressed: {
            wasdController.mousePressed(mouse);
        }
        onReleased: {
            wasdController.mouseReleased(mouse);
        }

        onPositionChanged: {
            wasdController.mouseMoved(Qt.vector2d(mouse.x, mouse.y));
        }
    }

}
