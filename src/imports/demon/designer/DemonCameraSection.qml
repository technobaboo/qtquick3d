import QtQuick 2.12
import HelperWidgets 2.0
import QtQuick.Layouts 1.12

Section {
    caption: qsTr("Camera")

    SectionLayout {
        Label {
            text: qsTr("Clip Near")
            tooltip: qsTr("Distance before camera frustum that will be clipped")
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
            tooltip: qsTr("Distance from camera frustum that will be clipped.")
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
            tooltip: qsTr("Angle of view of the camera")
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
    }
}
