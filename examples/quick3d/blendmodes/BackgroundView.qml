import QtQuick 2.0
import QtQuick3D 1.0

View3D {

    environment: SceneEnvironment {
        clearColor: "tan"
        backgroundMode: SceneEnvironment.Color
    }


    Light {

    }

    Camera {
        z: -500
    }

    Model {
        id: cube1
        source: "#Cube"
        materials: DefaultMaterial {
            diffuseColor: "blue"
        }
    }

    Model {
        id: cone1
        y: 100
        source: "#Cone"
        materials: DefaultMaterial {
            diffuseColor: "salmon"
        }
    }

    Model {
        id: cylinder1
        x: -300
        source: "#Cylinder"
        materials: DefaultMaterial {
            diffuseColor: "green"
        }
    }

}
