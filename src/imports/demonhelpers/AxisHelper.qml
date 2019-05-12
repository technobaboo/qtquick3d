import QtDemon 1.0
import QtQuick 2.12

DemonNode {
    id: axisGrid_obj

    property alias gridColor: gridMaterial.diffuseColor
    property alias gridOpacity: gridMaterial.opacity
    property alias enableXZGrid: gridXZ.visible
    property alias enableXYGrid: gridXY.visible
    property alias enableYZGrid: gridYZ.visible
    property bool enableAxisLines: true

    // Axis Lines
    DemonModel {
        id: xAxis
        source: "#Cube"
        position: Qt.vector3d(5000, 0, 0)
        scale: Qt.vector3d(100, .05, .05)
        visible: enableAxisLines

        materials: DemonDefaultMaterial {
            lighting: DemonDefaultMaterial.NoLighting
            diffuseColor: "red"
        }
    }

    DemonModel {
        id: yAxis
        source: "#Cube"
        position: Qt.vector3d(0, 5000, 0)
        scale: Qt.vector3d(0.05, 100, 0.05)
        visible: enableAxisLines
        materials: DemonDefaultMaterial {
            lighting: DemonDefaultMaterial.NoLighting
            diffuseColor: "green"
        }
    }

    DemonModel {
        id: zAxis
        source: "#Cube"
        position: Qt.vector3d(0, 0, 5000)
        scale: Qt.vector3d(0.05, 0.05, 100)
        visible: enableAxisLines
        materials: DemonDefaultMaterial {
            lighting: DemonDefaultMaterial.NoLighting
            diffuseColor: "blue"
        }
    }

    // Grid Lines
    DemonDefaultMaterial {
        id: gridMaterial
        lighting: DemonDefaultMaterial.NoLighting
        opacity: 0.5
        diffuseColor: Qt.rgba(0.8, 0.8, 0.8, 1)
    }

    DemonModel {
        id: gridXZ
        source: "meshes/AxisGrid_axisGrid.mesh"
        scale: Qt.vector3d(100, 100, 100)
        materials: [
            gridMaterial
        ]
    }

    DemonModel {
        id: gridXY
        visible: false
        source: "meshes/AxisGrid_axisGrid.mesh"
        scale: Qt.vector3d(100, 100, 100)
        rotation: Qt.vector3d(90, 0, 0)
        materials: [
            gridMaterial
        ]
    }

    DemonModel {
        id: gridYZ
        visible: false
        source: "meshes/AxisGrid_axisGrid.mesh"
        scale: Qt.vector3d(100, 100, 100)
        rotation: Qt.vector3d(0, 0, 90)
        materials: [
            gridMaterial
        ]
    }
}
