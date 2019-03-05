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

        // Light always points the same direction as camera
        DemonLight {
            id: directionalLight
            lightType: DemonLight.Directional
            rotation: Qt.vector3d(0, 0, 0)
            SequentialAnimation on rotation {
                loops: Animation.Infinite
                PropertyAnimation { duration: 5000; to: Qt.vector3d(0, 360, 0); from: Qt.vector3d(0, 0, 0) }
            }
            onRotationChanged: {
                console.log(rotation);
            }
        }


        DemonNode {
            id: cameraSpinner
            position: Qt.vector3d(0, 0, 0);


            DemonCamera {
                id: camera
                //position: Qt.vector3d(0, 0, -600)
                y: 600
                rotation: Qt.vector3d(90, 0, 0)
            }

            //rotation: Qt.vector3d(0, 90, 0)

//            SequentialAnimation on rotation {
//                loops: Animation.Infinite
//                PropertyAnimation { duration: 5000; to: Qt.vector3d(0, 360, 0); from: Qt.vector3d(0, 0, 0) }
//            }
        }


//        ColorfulCube {
//            id: cube1
//            position: Qt.vector3d(-100, 0, 0);
//            rotationTo: Qt.vector3d(-360, 0, 0);
//        }

//        ColorfulCube {
//            id: cube2
//            position: Qt.vector3d(100, 0, 0);
//        }

        WeirdShape {
            id: weirdShape
        }

        DemonModel {
            id: floor
            source: "#Rectangle"

            y: -100

            scale: Qt.vector3d(100, 100, 0);
            rotation: Qt.vector3d(90, 0, 0);

            materials: [floorMaterial]

            DemonDefaultMaterial {
                id: floorMaterial
            }
        }

    }
}
