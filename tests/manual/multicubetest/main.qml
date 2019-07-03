import QtQuick 2.12
import QtQuick.Window 2.12
import QtDemon 1.0


Window {
    id: window
    visible: true
    width: 640
    height: 480
    title: qsTr("10,000 Cubes (draw call overhead)")

    property int frames: 0
    property int fps: 0
    onFrameSwapped: {
        frames++;
    }

    property vector3d cubeRotation: Qt.vector3d(0, 0, 0)


    // Uncomment to see performance when animating 10,000 cubes individually
//    NumberAnimation {
//        target: window
//        property: "cubeRotation.x"
//        duration: 1000
//        from: 0
//        to: 360
//        running: true
//        loops: -1
//    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
                GradientStop { position: 0.0; color: "#d2ff52" }
                GradientStop { position: 1.0; color: "#91e842" }
            }
        Text {
            anchors.fill: parent
            text: fps
            fontSizeMode: Text.Fit
            color: "white"
            minimumPointSize: 128
            font.pointSize: 512
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            renderType: Text.NativeRendering
        }
    }

    Component {
        id: simpleCube
        DemonModel {
            source: "#Cube"
            scale: Qt.vector3d(0.1, 0.1, 0.1)
            rotation: window.cubeRotation
            materials: DemonDefaultMaterial {
                diffuseColor: "white"
            }
        }
    }


    DemonView3D {
        id: view
        anchors.fill: parent

        environment: DemonSceneEnvironment {
            isDepthPrePassDisabled: true
        }

        DemonLight {
            diffuseColor: "red"

        }

        DemonLight {
            diffuseColor: "green"
            rotation: Qt.vector3d(0, 180, 0)
        }

        DemonNode {
            id: cameraSpinner
            position: Qt.vector3d(0, 0, 0);


            DemonCamera {
                id: camera
                position: Qt.vector3d(0, 0, -700)
                // Frustum Culling is disabled because we always see everything
                enableFrustumCulling: false
            }

            rotation: Qt.vector3d(0, 90, 0)


            SequentialAnimation on rotation {
                loops: Animation.Infinite
                PropertyAnimation { duration: 10000; to: Qt.vector3d(0, 360, 0); from: Qt.vector3d(0, 0, 0) }
            }
        }

    }

    Timer {
        interval: 500
        running: true
        onTriggered: {
            let spacing = 25.0;
            let countX = 25
            let countY = 20
            let countZ = 20
            let offsetX = -spacing * countX * 0.5;
            let offsetY = -spacing * countY * 0.5;
            let offsetZ = -spacing * countZ * 0.5;

            for(var x = 0; x < countX; ++x)
            {
                for(var y = 0; y < countY; ++y)
                {
                    for(var z = 0; z < countZ; ++z)
                    {
                        let posX = offsetX + x * spacing;
                        let posY = offsetY + y * spacing;
                        let posZ = offsetZ + z * spacing;

                        simpleCube.createObject(view.scene, {"x": posX, "y": posY, "z": posZ })
                    }
                }
            }
        }
    }

    Timer {
        id: fpsTimer
        interval: 1000
        repeat: true
        running: true
        onTriggered: {
            fps = frames;
            frames = 0;
        }
    }

    Text {
        anchors.top: parent.top
        anchors.right: parent.right
        text: "fps: " + fps
    }

    Rectangle {
        id: greenRect
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        color: "green"
        height: 50
        width: 50

        Text {
            anchors.centerIn: parent
            text: fps
            font.pointSize: 20
        }


        NumberAnimation {
            target: greenRect
            property: "rotation"
            duration: 2000
            easing.type: Easing.InOutQuad
            from: 0
            to: 360
            running: true
            loops: -1
        }
    }
}