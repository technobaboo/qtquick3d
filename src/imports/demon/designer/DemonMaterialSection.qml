import QtQuick 2.12
import HelperWidgets 2.0
import QtQuick.Layouts 1.12

Section {
    caption: qsTr("Material")

    SectionLayout {

        // Baked Lighting properties (may be internal eventually)
        // ### lightmapIndirect
        // ### lightmapRadiosity
        // ### lightmapShadow

        // ### iblProbe override

        // ### emissiveMap2 (common between DefaultMaterial and CustomMaterial for some reason now

        Label {
            text: qsTr("Displacement Amount")
            tooltip: qsTr("Distance to offset vertices")
        }
        SecondColumnLayout {
            SpinBox {
                maximumValue: 9999999
                minimumValue: -9999999
                decimals: 0
                backendValue: backendValues.displacementAmount
                Layout.fillWidth: true
            }
        }
    }
}
