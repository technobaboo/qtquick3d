import QtQuick 2.12
import QtQuick.Window 2.11
import QtDemon 1.0
import QtDemonMaterialLibrary 1.0

Window {
    id: window
    width: 640
    height: 640
    visible: true
    color: "black"

    Image {
        source: "qt_logo.png"
        x: 50
        SequentialAnimation on y {
            loops: Animation.Infinite
            PropertyAnimation { duration: 3000; to: 400; from: 50 }
        }
    }

    DemonView3D {
        id: layer1
        anchors.fill: parent
        anchors.margins: 50
        camera: camera
        renderMode: DemonView3D.Overlay

        environment: DemonSceneEnvironment {
            probeBrightness: 1000
            backgroundMode: DemonSceneEnvironment.Transparent
            lightProbe: DemonImage {
                source: "maps/OpenfootageNET_garage-1024.hdr"
            }
        }
        DemonCamera {
            id: camera
            position: Qt.vector3d(0, 200, -300)
            rotation: Qt.vector3d(30, 0, 0)
        }
        DemonModel {
            position: Qt.vector3d(0, 0, 0)
            source: "#Cube"
            materials: [ GlassMaterial {
                }
            ]
            rotation: Qt.vector3d(0, 90, 0)

            SequentialAnimation on rotation {
                loops: Animation.Infinite
                PropertyAnimation { duration: 5000;
                    to: Qt.vector3d(0, 360, 0);
                    from: Qt.vector3d(0, 0, 0) }
            }
         }
    }


}
