import QtQuick 2.12
import QtDemon 1.0

Item {
    property DemonNode controlledObject: undefined
    property DemonView3D view: undefined

    property real forwardSpeed: 5
    property real backSpeed: 5
    property real rightSpeed: 5
    property real leftSpeed: 5
    property real upSpeed: 5
    property real downSpeed: 5
    property real xSpeed: 0.1
    property real ySpeed: 0.1

    property bool xInvert: false
    property bool yInvert: true

    Component.onCompleted: {
        if (!view)
            return;

        view.focus = true
        view.Keys.onPressed.connect(handleKeyPress)
        view.Keys.onReleased.connect(handleKeyRelease)

        mouseAreaComponent.createObject(view);
    }

    function mousePressed(mouse) {
        status.currentPos = Qt.vector2d(mouse.x, mouse.y)
        status.lastPos = Qt.vector2d(mouse.x, mouse.y)
        status.useMouse = true;
    }

    function mouseReleased(mouse) {
        status.useMouse = false;
    }

    function mouseMoved(newPos) {
        status.currentPos = newPos;
    }

    function forwardPressed() {
        status.moveForward = true
        status.moveBack = false
    }

    function forwardReleased() {
        status.moveForward = false
    }

    function backPressed() {
        status.moveBack = true
        status.moveForward = false
    }

    function backReleased() {
        status.moveBack = false
    }

    function rightPressed() {
        status.moveRight = true
        status.moveLeft = false
    }

    function rightReleased() {
        status.moveRight = false
    }

    function leftPressed() {
        status.moveLeft = true
        status.moveRight = false
    }

    function leftReleased() {
        status.moveLeft = false
    }

    function upPressed() {
        status.moveUp = true
        status.moveDown = false
    }

    function upReleased() {
        status.moveUp = false
    }

    function downPressed() {
        status.moveDown = true
        status.moveUp = false
    }

    function downReleased() {
        status.moveDown = false
    }

    function handleKeyPress(event)
    {
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

    function handleKeyRelease(event)
    {
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

    Item {
        id: status

        property bool moveForward: false
        property bool moveBack: false
        property bool moveLeft: false
        property bool moveRight: false
        property bool moveUp: false
        property bool moveDown: false
        property bool useMouse: false
        property bool moving: moveForward | moveBack | moveLeft | moveRight | moveUp | moveDown | useMouse

        property vector2d lastPos: Qt.vector2d(0, 0)
        property vector2d currentPos: Qt.vector2d(0, 0)

        function updatePosition(vector, speed, position)
        {
            var direction = vector;
            var velocity = Qt.vector3d(direction.x * speed,
                                       direction.y * speed,
                                       direction.z * speed);
            controlledObject.position = Qt.vector3d(position.x + velocity.x,
                                                    position.y + velocity.y,
                                                    position.z + velocity.z);
        }

        function negate(vector) {
            return Qt.vector3d(-vector.x, -vector.y, -vector.z)
        }

        function updateInput() {
            if (controlledObject == undefined)
                return;

            if (moveForward)
                updatePosition(controlledObject.forward, forwardSpeed, controlledObject.position);
            else if (moveBack)
                updatePosition(negate(controlledObject.forward), backSpeed, controlledObject.position);

            if (moveRight)
                updatePosition(controlledObject.right, rightSpeed, controlledObject.position);
            else if (moveLeft)
                updatePosition(negate(controlledObject.right), leftSpeed, controlledObject.position);

            if (moveDown)
                updatePosition(negate(controlledObject.up), downSpeed, controlledObject.position);
            else if (moveUp)
                updatePosition(controlledObject.up, upSpeed, controlledObject.position);

            if (useMouse) {
                // Get the delta
                var rotationVector = controlledObject.rotation;
                var delta = Qt.vector2d(lastPos.x - currentPos.x,
                                        lastPos.y - currentPos.y);
                // rotate x
                var rotateX = delta.x * -xSpeed
                if (xInvert)
                    rotateX = -rotateX;
                rotationVector.y += rotateX;

                // rotate y
                var rotateY = delta.y * ySpeed
                if (yInvert)
                    rotateY = -rotateY;
                rotationVector.x += rotateY;
                controlledObject.setRotation(rotationVector);
                lastPos = currentPos;
            }
        }

        Timer {
            id: updateTimer
            interval: 16
            repeat: true
            running: status.moving
            onTriggered: status.updateInput();
        }
    }

    Component {
        id: mouseAreaComponent
        MouseArea {
            anchors.fill: parent
            onPressed: mousePressed(mouse);
            onReleased: mouseReleased(mouse);
            onPositionChanged: mouseMoved(Qt.vector2d(mouse.x, mouse.y));
        }
    }

}
