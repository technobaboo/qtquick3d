import QtQuick 2.11
import QtQuick.Window 2.11

import QtQuick3D 1.0

Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Picking Example")


    View3D {
        id: view
        anchors.fill: parent

        Light {

        }

        Camera {
            z: -500
        }

        Model {
            id: cube
            source: "#Cube"
            property bool isTouched: false

            materials: DefaultMaterial {
                diffuseColor: cube.isTouched ? "green" : "blue"
            }
        }
    }

    MouseArea {
        anchors.fill: view

        onClicked: {
            console.log("picking (" + mouse.x + ", " + mouse.y + ")")
            var result = view.pick(mouse.x, mouse.y);
            if (result) {
                console.log(result.objectHit);
                if (result.objectHit) {
                    result.objectHit.isTouched = !result.objectHit.isTouched;
                }
            }
        }
    }
}
