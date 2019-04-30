import QtQuick 2.12
import HelperWidgets 2.0
import QtQuick.Layouts 1.12

Section {
    caption: qsTr("Model")

    SectionLayout {

        Label {
            text: qsTr("Source")
            tooltip: qsTr("Set the source of the mesh data file")
        }
        SecondColumnLayout {
            UrlChooser {
                backendValue: backendValues.source
                filter: "*.mesh"
            }
        }

        Label {
            text: qsTr("Tesselation Mode")
        }
        SecondColumnLayout {
            ComboBox {
                scope: "DemonModel."
                model: ["NoTess", "TessLinear", "TessPhong", "TessNPatch"]
                backendValue: backendValues.orientation
                Layout.fillWidth: true
            }
        }

        Label {
            text: qsTr("Edge Tesselation")
        }
        SecondColumnLayout {
            SpinBox {
                maximumValue: 64.0
                minimumValue: 0.0
                decimals: 0
                backendValue: backendValues.edgeTess
                Layout.fillWidth: true
            }
        }
        Label {
            text: qsTr("Inner Tesselation")
        }
        SecondColumnLayout {
            SpinBox {
                maximumValue: 64.0
                minimumValue: 0.0
                decimals: 0
                backendValue: backendValues.innerTess
                Layout.fillWidth: true
            }
        }

        Label {
            text: "Enable Wireframe Mode"
        }
        SecondColumnLayout {
            CheckBox {
                text: backendValues.isWireframeMode.valueToString
                backendValue: backendValues.isWireframeMode
                Layout.fillWidth: true
            }
        }
    }
}
