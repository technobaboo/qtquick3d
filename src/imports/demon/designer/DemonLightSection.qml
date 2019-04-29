import QtQuick 2.12
import HelperWidgets 2.0
import QtQuick.Layouts 1.12

Column {
    width: parent.width

    Section {
        caption: qsTr("Light")
        width: parent.width

        SectionLayout {
            Label {
                text: qsTr("Light Type")
                tooltip: qsTr("Type of illumination to use")
            }
            ComboBox {
                scope: "DemonLight."
                model: ["Directional", "Point", "Area"]
                backendValue: backendValues.lightType
                Layout.fillWidth: true
            }

            Label {
                text: qsTr("Brightness")
                tooltip: qsTr("Strength of the light")
            }
            SecondColumnLayout {
                SpinBox {
                    maximumValue: 9999999
                    minimumValue: -9999999
                    decimals: 0
                    backendValue: backendValues.brightness
                    Layout.fillWidth: true
                }
            }

            // ### only for Point Lights
            Label {
                text: qsTr("Linear Fade")
                tooltip: qsTr("Falloff of the point light")
            }
            SecondColumnLayout {
                SpinBox {
                    minimumValue: 0
                    maximumValue: 1000
                    decimals: 0
                    backendValue: backendValues.linearFade
                    Layout.fillWidth: true
                }
            }


            // ### only for Point Lights
            Label {
                text: qsTr("Exponential Fade")
                tooltip: qsTr("Additional falloff")
            }
            SecondColumnLayout {
                SpinBox {
                    minimumValue: 0
                    maximumValue: 1000
                    decimals: 0
                    backendValue: backendValues.exponentialFade
                    Layout.fillWidth: true
                }
            }

            // ### only for Area Lights
            Label {
                text: qsTr("Area Width")
                tooltip: qsTr("Width of the surface of the area light")
            }
            SecondColumnLayout {
                SpinBox {
                    minimumValue: 0
                    maximumValue: 100
                    decimals: 0
                    backendValue: backendValues.areaWidth
                    Layout.fillWidth: true
                }
            }

            // ### only for Area Lights
            Label {
                text: qsTr("Area Height")
                tooltip: qsTr("Height of the surface of the area light")
            }
            SecondColumnLayout {
                SpinBox {
                    minimumValue: 0
                    maximumValue: 100
                    decimals: 0
                    backendValue: backendValues.areaHeight
                    Layout.fillWidth: true
                }
            }
        }

    }
    Section {
        caption: qsTr("Diffuse Color")
        width: parent.width

        ColorEditor {
            caption: qsTr("Diffuse Color")
            backendValue: backendValues.diffuseColor
            supportGradient: false
            Layout.fillWidth: true
        }
    }

    Section {
        caption: qsTr("Emissive Color")
        width: parent.width
        ColorEditor {
            caption: qsTr("Emissive Color")
            backendValue: backendValues.emissiveColor
            supportGradient: false
            Layout.fillWidth: true
        }
    }

    Section {
        caption: qsTr("Specular Tint Color")
        width: parent.width
        ColorEditor {
            caption: qsTr("Specular Tint Color")
            backendValue: backendValues.specularTint
            supportGradient: false
            Layout.fillWidth: true
        }
    }

    Section {
        caption: qsTr("Shadows")
        width: parent.width

        SectionLayout {

            Label {
                text: "Casts Shadow"
                tooltip: qsTr("Enable shadow casting for this light")
            }
            SecondColumnLayout {
                CheckBox {
                    text: backendValues.castShadow.valueToString
                    backendValue: backendValues.castShadow
                    Layout.fillWidth: true
                }
            }

            // ### all the following should only be shown when shadows are enabled
            Label {
                text: qsTr("Shadow Darkness")
                tooltip: qsTr("Factor used to darken shadows")
            }
            SecondColumnLayout {
                SpinBox {
                    minimumValue: 1.0
                    maximumValue: 100.0
                    decimals: 0
                    backendValue: backendValues.shadowFactor
                    Layout.fillWidth: true
                }
            }

            Label {
                text: qsTr("Shadow Softness")
                tooltip: qsTr("Width of the blur filter on the shadow map")
            }
            SecondColumnLayout {
                SpinBox {
                    minimumValue: 1.0
                    maximumValue: 100.0
                    decimals: 0
                    backendValue: backendValues.shadowFilter
                    Layout.fillWidth: true
                }
            }

            Label {
                text: qsTr("Shadow Resolution")
                tooltip: qsTr("Resolution of shadow map (powers of two)")
            }
            SecondColumnLayout {
                ComboBox {
                    model: [7, 8, 9, 10, 11, 12]
                    backendValue: backendValues.shadowMapResolution
                    Layout.fillWidth: true
                }
            }

            Label {
                text: qsTr("Shadow Depth Bias")
                tooltip: qsTr("Slight offset to avoid self-shadowing artifacts")
            }
            SecondColumnLayout {
                SpinBox {
                    minimumValue: 0
                    maximumValue: 100
                    decimals: 0
                    backendValue: backendValues.shadowBias
                    Layout.fillWidth: true
                }
            }

            Label {
                text: qsTr("Shadow Far Clip")
                tooltip: qsTr("Affects the maximum distance for the shadow depth map")
            }
            SecondColumnLayout {
                SpinBox {
                    maximumValue: 9999999
                    minimumValue: -9999999
                    decimals: 0
                    backendValue: backendValues.shadowMapFar
                    Layout.fillWidth: true
                }
            }

            Label {
                text: qsTr("Shadow Field of View")
                tooltip: qsTr("Affects the field of view of the shadow camera")
            }
            SecondColumnLayout {
                SpinBox {
                    minimumValue: 1.0
                    maximumValue: 179.0
                    decimals: 0
                    backendValue: backendValues.shadowMapFieldOfView
                    Layout.fillWidth: true
                }
            }
        }
    }
}
