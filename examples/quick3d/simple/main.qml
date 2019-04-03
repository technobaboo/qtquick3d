import QtQuick 2.12
import QtQuick.Window 2.11
import QtDemon 1.0

Window {
    id: window
    width: 1280
    height: 720
    color: "green"
    visible: true

    DemonView3D {
        id: layer1
        anchors.fill: parent
        camera: camera

        // Light always points the same direction as camera
//        DemonLight {
//            id: directionalLight
//            lightType: DemonLight.Directional
//            rotation: Qt.vector3d(0, 0, 0)
//            SequentialAnimation on rotation {
//                loops: Animation.Infinite
//                PropertyAnimation { duration: 5000; to: Qt.vector3d(0, 360, 0); from: Qt.vector3d(0, 0, 0) }
//            }
//        }

        environment: DemonSceneEnvironment {
            probeBrightness: 1000
            lightProbe: DemonImage {
                source: ":/maps/OpenfootageNET_garage-1024.hdr"
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

        DemonNode {
            id: shapeSpawner
            Timer {
                property real range: 300
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
                        let instance = weirdShapeComponent.createObject(shapeSpawner, {"x": xPos, "y": yPos, "z": zPos, "scale": Qt.vector3d(0.25, 0.25, 0.25)})
                        instances.push(instance);
                        //console.log("created WeirdShape[" + instances.length + "] at: (" + xPos + ", " + yPos + ", " + zPos + ")");
                        if (instances.length === 10)
                            reverse = true;
                    } else {
                        // remove last item in instances list
                        //console.log("removed WeirdShape[" + instances.length + "]");
                        let instance = instances.pop();
                        instance.destroy();
                        if (instances.length === 0) {
                            reverse = false;
                        }
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


        TexturedCube {
            x: -300
        }

        CopperCube {
            id: copperCube
            SequentialAnimation on metalColor {
                loops: Animation.Infinite
                PropertyAnimation { duration: 2000; to: Qt.vector3d(0.805, 0.0, 0.305) }
                PropertyAnimation { duration: 2000; to: Qt.vector3d(0.805, 1.0, 0.305) }
            }
        }

        DemonModel {
            position: Qt.vector3d(300, 0, 0)
            source: "#Cube"
            materials: [ SimpleGlassMaterial {
                    id: texturedCubeMaterial
                }
            ]
        }
    }
}
