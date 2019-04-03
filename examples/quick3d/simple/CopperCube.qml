import QtQuick 2.12
import QtDemon 1.0

DemonModel {
    source: "#Cube"
    property alias metalColor: copperMaterial.metal_color
    materials: [ CopperMaterial {
            id: copperMaterial
        }
    ]
}
