import QtQuick 2.12
import HelperWidgets 2.0
import QtQuick.Layouts 1.12

Section {
    caption: qsTr("Image")
    width: parent.width
    SectionLayout {
        Label {
            text: qsTr("Source")
            tooltip: qsTr("Source of image data")
        }
        SecondColumnLayout {
            UrlChooser {
                backendValue: backendValues.source
            }
        }

        Label {
            text: qsTr("U Repeat")
            tooltip: qsTr("Number of times the image is tiled on the U direction of the material")
        }
        SecondColumnLayout {
            SpinBox {
                maximumValue: 999999
                minimumValue: -999999
                decimals: 0
                backendValue: backendValues.scaleu
                Layout.fillWidth: true
            }
        }

        Label {
            text: qsTr("V Repeat")
            tooltip: qsTr("Number of times the image is tiled on the V direction of the material")
        }
        SecondColumnLayout {
            SpinBox {
                maximumValue: 999999
                minimumValue: -999999
                decimals: 0
                backendValue: backendValues.scalev
                Layout.fillWidth: true
            }
        }

        Label {
            text: qsTr("Texture Mapping")
            tooltip: qsTr("How the image is applied to thematerial")
        }
        SecondColumnLayout {
            ComboBox {
                scope: "DemonImage"
                model: ["Normal", "Environment", "LightProbe"]
                backendValue: backendValues.mappingmode
                Layout.fillWidth: true
            }
        }

        Label {
            text: qsTr("U Tiling")
            tooltip: qsTr("How the image is tiled in the U direction")
        }
        SecondColumnLayout {
            ComboBox {
                scope: "DemonImage"
                model: ["Unknown", "ClampToEdge", "MirroredRepeat", "Repeat"]
                backendValue: backendValues.tilingmodehorz
                Layout.fillWidth: true
            }
        }

        Label {
            text: qsTr("V Tiling")
            tooltip: qsTr("How the image is tiled in the V direction")
        }
        SecondColumnLayout {
            ComboBox {
                scope: "DemonImage"
                model: ["Unknown", "ClampToEdge", "MirroredRepeat", "Repeat"]
                backendValue: backendValues.tilingmodevert
                Layout.fillWidth: true
            }
        }

        Label {
            text: qsTr("UV Rotation")
            tooltip: qsTr("Rotate the image's coordinates")
        }
        SecondColumnLayout {
            SpinBox {
                maximumValue: 999999
                minimumValue: -999999
                decimals: 0
                backendValue: backendValues.rotationuv
                Layout.fillWidth: true
            }
        }

        Label {
            text: qsTr("U Position")
            tooltip: qsTr("Offset of the image along the U direction of the material")
        }
        SecondColumnLayout {
            SpinBox {
                maximumValue: 999999
                minimumValue: -999999
                decimals: 0
                backendValue: backendValues.positionu
                Layout.fillWidth: true
            }
        }

        Label {
            text: qsTr("V Position")
            tooltip: qsTr("Offset of the image along the V direction of the material")
        }
        SecondColumnLayout {
            SpinBox {
                maximumValue: 999999
                minimumValue: -999999
                decimals: 0
                backendValue: backendValues.positionv
                Layout.fillWidth: true
            }
        }

        Label {
            text: qsTr("U Pivot")
            tooltip: qsTr("Offset the image in the U direction without affecting rotation center")
        }
        SecondColumnLayout {
            SpinBox {
                maximumValue: 999999
                minimumValue: -999999
                decimals: 0
                backendValue: backendValues.pivotu
                Layout.fillWidth: true
            }
        }

        Label {
            text: qsTr("V Pivot")
            tooltip: qsTr("Offset the image in the V direction without affecting rotation center")
        }
        SecondColumnLayout {
            SpinBox {
                maximumValue: 999999
                minimumValue: -999999
                decimals: 0
                backendValue: backendValues.pivotv
                Layout.fillWidth: true
            }
        }

    }

}
