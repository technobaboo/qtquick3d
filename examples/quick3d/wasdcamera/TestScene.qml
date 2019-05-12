import QtQuick 2.12
import QtDemon 1.0

DemonNode {
    id: rootItem
    property DemonCamera activeCamera: camera

    DemonCamera {
        id: camera
        z: -600
    }

    DemonLight {
        id: light
    }

    DemonNode {
        z: 45
        rotation: Qt.vector3d(90, 0, 0)


    DemonModel {
        id: cone
        source: "#Cone"
        materials: [
            DemonDefaultMaterial {
                id: coneMaterial
                diffuseColor: "pink"
            }
        ]


        NumberAnimation {
            target: cone
            property: "y"
            duration: 2000
            easing.type: Easing.InOutQuad
            from: 0
            to: 500
            running: true

        }

        onForwardChanged: {
            console.log("forward: " + forward);
        }
        onUpChanged: {
            console.log("up: " + up);
        }
        onRightChanged: {
            console.log("right: " + right);
        }

        onGlobalPositionChanged: {
            console.log("globalPos: " + globalPosition)
        }
    }
    }
}
