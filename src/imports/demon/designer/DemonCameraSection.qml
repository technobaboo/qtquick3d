import QtQuick 2.12
import HelperWidgets 2.0
import QtQuick.Layouts 1.12

Section {
    caption: qsTr("Camera")

    SectionLayout {
        Label {
            text: qsTr("Clip Near")
            tooltip: qsTr("Near distance at which objects disappear")
        }
        SecondColumnLayout {
            SpinBox {
                maximumValue: 9999999
                minimumValue: -9999999
                decimals: 0
                backendValue: backendValues.clipNear
                Layout.fillWidth: true
            }
        }

        Label {
            text: qsTr("Clip Far")
            tooltip: qsTr("Far distance at which objects disappear")
        }
        SecondColumnLayout {
            SpinBox {
                maximumValue: 9999999
                minimumValue: -9999999
                decimals: 2
                backendValue: backendValues.clipFar
                Layout.fillWidth: true
            }
        }

        Label {
            text: qsTr("Field of View")
            tooltip: qsTr("Viewing angle of the camera (how much it can see)")
        }
        SecondColumnLayout {
            SpinBox {
                maximumValue: 1
                minimumValue: 180
                decimals: 2
                backendValue: backendValues.fieldOfView
                Layout.fillWidth: true
            }
        }

        Label {
            text: "FOV Horizontal"
            tooltip: qsTr("Field of view angle orientation")
        }
        SecondColumnLayout {
            CheckBox {
                text: backendValues.isFieldOFViewHorizontal.valueToString
                backendValue: backendValues.isFieldOFViewHorizontal
                Layout.fillWidth: true
            }
        }

        Label {
            text: qsTr("Projection Mode")
        }
        ComboBox {
            scope: "DemonCamera"
            model: ["Perspective", "Orthographic"]
            backendValue: backendValues.projectionMode
            Layout.fillWidth: true
        }
    }
}
