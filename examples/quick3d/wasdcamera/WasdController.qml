import QtQuick 2.12
import QtDemon 1.0

Item {
    property DemonNode controlledObject: undefined

    property real forwardSpeed: 5
    property real backSpeed: 5
    property real rightSpeed: 5
    property real leftSpeed: 5
    property real xSpeed: 0.05
    property real ySpeed: 0.05

    property bool xInvert: false
    property bool yInvert: true

    function mousePressed(mouse) {
        status.lastPos = Qt.vector2d(mouse.x, mouse.y)
        status.useMouse = true;
    }

    function mouseReleased(mouse) {
        status.useMouse = false;
    }

    function mouseMoved(newPos) {
        status.currentPos = newPos;
        console.log(newPos)
    }

    function forwardPressed() {
        status.moveForward++
    }

    function forwardReleased() {
        status.moveForward--
    }

    function backPressed() {
        status.moveBack++
    }

    function backReleased() {
        status.moveBack--
    }

    function rightPressed() {
        status.moveRight++
    }

    function rightReleased() {
        status.moveRight--
    }

    function leftPressed() {
        status.moveLeft++
    }

    function leftReleased() {
        status.moveLeft--
    }


    Item {
        id: status

        property int moveForward: 0
        property int moveBack: 0
        property int moveLeft: 0
        property int moveRight: 0
        property bool useMouse: false
        property vector2d lastPos: undefined
        property vector2d currentPos: undefined

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
            if (moveBack)
                updatePosition(negate(controlledObject.forward), backSpeed, controlledObject.position);
            if (moveRight)
                updatePosition(controlledObject.right, rightSpeed, controlledObject.position);
            if (moveLeft)
                updatePosition(negate(controlledObject.right), leftSpeed, controlledObject.position);

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
                console.log(controlledObject.forward)
            }
        }

        Timer {
            id: updateTimer
            interval: 16
            repeat: true
            running: true
            onTriggered: {
                status.updateInput();
            }
        }
    }

}
