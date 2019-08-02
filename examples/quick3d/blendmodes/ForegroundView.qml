import QtQuick 2.0
import QtQuick3D 1.0

View3D {


    Node {
        id: orbiter
        NumberAnimation {
            target: orbiter
            property: "rotation.y"
            duration: 5000
            from: 0
            to: 360
            loops: -1
            running: true
        }

        Light {

        }
        Camera {
            z: -350
        }
    }



    Model {
        id: cube1
        source: "#Cube"
        x: -200
        materials: DefaultMaterial {
            diffuseColor: "yellow"
        }
    }

    Model {
        id: cone1
        y: -100
        source: "#Cone"
        materials: DefaultMaterial {
            diffuseColor: "pink"
        }


        SequentialAnimation {
            NumberAnimation {
                target: cone1
                property: "y"
                duration: 5000
                easing.type: Easing.InOutQuad
                from: -200
                to: 200
            }
            NumberAnimation {
                target: cone1
                property: "y"
                duration: 5000
                easing.type: Easing.InOutQuad
                from: 200
                to: -200
            }
            running: true
            loops: -1
        }


    }

    Model {
        id: cylinder1
        x: 200
        source: "#Cylinder"
        materials: DefaultMaterial {
            diffuseColor: "grey"
        }
    }

}
