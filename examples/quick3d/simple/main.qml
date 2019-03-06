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
        }


        DemonNode {
            id: cameraSpinner
            position: Qt.vector3d(0, 0, 0);


            DemonCamera {
                id: camera
                position: Qt.vector3d(0, 0, -600)
            }

            rotation: Qt.vector3d(0, 90, 0)

            SequentialAnimation on rotation {
                loops: Animation.Infinite
                PropertyAnimation { duration: 5000; to: Qt.vector3d(0, 360, 0); from: Qt.vector3d(0, 0, 0) }
            }
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

        Timer {
            property real range: 100
            property var instances: []
            property bool reverse: false

            running: true
            repeat: true
            interval: 500
            onTriggered: {
                if (!reverse) {
                    // Create a new weirdShape at random postion
                    var xPos = Math.random() * (range - (-range)) + -range;
                    var yPos = Math.random() * (range - (-range)) + -range;
                    var zPos = Math.random() * (range - (-range)) + -range;
                    var weirdShapeComponent = Qt.createComponent("WeirdShape.qml");
                    let instance = weirdShapeComponent.createObject(layer1, {"x": xPos, "y": yPos, "z": zPos, "scale": Qt.vector3d(0.25, 0.25, 0.25)})
                    instances.push(instance);
                    console.log("created WeirdShape[" + instances.length + "] at: (" + xPos + ", " + yPos + ", " + zPos + ")");
                    if (instances.length === 10)
                        reverse = true;
                } else {
                    // remove last item in instances list
                    console.log("removed WeirdShape[" + instances.length + "]");
                    let instance = instances.pop();
                    instance.destroy();
                    if (instances.length === 0) {
                        reverse = false;
                    }
                }
            }
        }

        WeirdShape {
            id: weirdShape1
            color: "red"
        }

        WeirdShape {
            id: weirdShape2
            color: "orange"
            x: 100
            y: 100
            z: 100
        }

        //        WeirdShape {
        //            id: weirdShape
        //        }

        //        DemonModel {
        //            id: floor
        //            source: "#Rectangle"

        //            y: -100

        //            scale: Qt.vector3d(100, 100, 0);
        //            rotation: Qt.vector3d(90, 0, 0);

        //            materials: [floorMaterial]

        //            DemonDefaultMaterial {
        //                id: floorMaterial
        //            }
        //        }

    }
}
