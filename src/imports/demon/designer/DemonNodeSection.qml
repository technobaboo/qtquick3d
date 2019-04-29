import QtQuick 2.12
import HelperWidgets 2.0
import QtQuick.Layouts 1.12

Section {
    caption: qsTr("Node")
    SectionLayout {
        Label {
            text: qsTr("Position X")
            tooltip: qsTr("X Position Translation")
        }
        SecondColumnLayout {
            SpinBox {
                maximumValue: 9999999
                minimumValue: -9999999
                decimals: 5
                backendValue: backendValues.x
                Layout.fillWidth: true
            }
        }

        Label {
            text: qsTr("Position Y")
            tooltip: qsTr("Y Position Translation")
        }
        SecondColumnLayout {
            SpinBox {
                maximumValue: 9999999
                minimumValue: -9999999
                decimals: 5
                backendValue: backendValues.y
                Layout.fillWidth: true
            }
        }

        Label {
            text: qsTr("Position Z")
            tooltip: qsTr("Z Position Translation")
        }
        SecondColumnLayout {
            SpinBox {
                maximumValue: 9999999
                minimumValue: -9999999
                decimals: 5
                backendValue: backendValues.z
                Layout.fillWidth: true
            }
        }

        Label {
            text: qsTr("Rotation X")
            tooltip: qsTr("X Rotation")
        }
        SecondColumnLayout {
            SpinBox {
                maximumValue: 9999999
                minimumValue: -9999999
                decimals: 5
                backendValue: backendValues.rotation_x
                Layout.fillWidth: true
            }
        }

        Label {
            text: qsTr("Rotation Y")
            tooltip: qsTr("Y Rotation")
        }
        SecondColumnLayout {
            SpinBox {
                maximumValue: 9999999
                minimumValue: -9999999
                decimals: 5
                backendValue: backendValues.rotation_y
                Layout.fillWidth: true
            }
        }

        Label {
            text: qsTr("Rotation Z")
            tooltip: qsTr("Z Rotation")
        }
        SecondColumnLayout {
            SpinBox {
                maximumValue: 9999999
                minimumValue: -9999999
                decimals: 5
                backendValue: backendValues.rotation_z
                Layout.fillWidth: true
            }
        }
    }
}
