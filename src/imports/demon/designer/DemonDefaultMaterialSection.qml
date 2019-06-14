import QtQuick 2.12
import HelperWidgets 2.0
import QtQuick.Layouts 1.12

Column {
    width: parent.width
    Section {
        caption: qsTr("Default Material")
        width: parent.width
        SectionLayout {
            Label {
                text: qsTr("Lighting")
                tooltip: qsTr("Light model")
            }
            ComboBox {
                scope: "DemonDefaultMaterial"
                model: ["NoLighting", "VertexLighting", "FragmentLighting"]
                backendValue: backendValues.lighting
                Layout.fillWidth: true
            }
            Label {
                text: qsTr("Blending Mode")
                tooltip: qsTr("How this material blends with content behind it.")
            }
            ComboBox {
                scope: "DemonDefaultMaterial"
                model: ["Normal", "Screen", "Multiply", "Overlay", "ColorBurn", "ColorDodge" ]
                backendValue: backendValues.blendMode
                Layout.fillWidth: true
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

    // ### diffuseMap
    // ### diffuseMap2
    // ### diffuseMap3

    Section {
        caption: qsTr("Emissive")
        width: parent.width
        SectionLayout {
            Label {
                text: qsTr("Emissive Power")
                tooltip: qsTr("Amount of self-illumination for this material. (will not light other objects)")
            }
            SecondColumnLayout {
                SpinBox {
                    maximumValue: 9999999
                    minimumValue: -9999999
                    decimals: 0
                    backendValue: backendValues.emissivePower
                    Layout.fillWidth: true
                }
            }
            // ### emissiveMap
        }
    }

    Section {
        caption: "Emissive Color"
        width: parent.width
        ColorEditor {
            caption: qsTr("Emissive Color")
            backendValue: backendValues.emissiveColor
            supportGradient: false
            Layout.fillWidth: true
        }
    }

    // ### specularReflectionMap
    // ### specularMap
    Section {
        caption: "Specular Tint"
        width: parent.width
        ColorEditor {
            caption: qsTr("Specular Tint")
            backendValue: backendValues.specularTint
            supportGradient: false
            Layout.fillWidth: true
        }
    }

    Section {
        caption: "Specular"
        width: parent.width
        SectionLayout {
            Label {
                text: qsTr("Specular Model")
                tooltip: qsTr("Equation to use when calculating specular highlights for CG lights")
            }
            ComboBox {
                scope: "DemonDefaultMaterial"
                model: ["Default", "KGGX", "KWard"]
                backendValue: backendValues.specularModel
                Layout.fillWidth: true
            }
            Label {
                text: qsTr("Index of Refraction")
                tooltip: qsTr("Index of refraction of the material")
            }
            SecondColumnLayout {
                SpinBox {
                    maximumValue: 9999999
                    minimumValue: 1
                    decimals: 2
                    backendValue: backendValues.indexOfRefraction
                    Layout.fillWidth: true
                }
            }
            Label {
                text: qsTr("Fresnel Power")
                tooltip: qsTr("Damping of head-on reflections")
            }
            SecondColumnLayout {
                SpinBox {
                    maximumValue: 9999999
                    minimumValue: -9999999
                    decimals: 2
                    backendValue: backendValues.fresnelPower
                    Layout.fillWidth: true
                }
            }
            Label {
                text: qsTr("Specular Amount")
                tooltip: qsTr("Amount of shine/gloss")
            }
            SecondColumnLayout {
                SpinBox {
                    maximumValue: 9999999
                    minimumValue: -9999999
                    decimals: 2
                    backendValue: backendValues.specularAmount
                    Layout.fillWidth: true
                }
            }
            Label {
                text: qsTr("Specular Roughness")
                tooltip: qsTr("Softening applied to reflections and highlights")
            }
            SecondColumnLayout {
                SpinBox {
                    maximumValue: 1
                    minimumValue: 0.001
                    decimals: 2
                    backendValue: backendValues.specularRoughness
                    Layout.fillWidth: true
                }
            }
            // ### roughnessMap
        }
    }

    Section {
        caption: "Opacity"
        width: parent.width
        SectionLayout {
            Label {
                text: qsTr("Opacity")
                tooltip: qsTr("Visibility of the geometry for this material.")
            }
            SecondColumnLayout {
                SpinBox {
                    maximumValue: 1
                    minimumValue: 0
                    decimals: 2
                    backendValue: backendValues.opacity
                    Layout.fillWidth: true
                }
            }

            // ### opacityMap
        }
    }

    Section {
        caption: "Bump"
        width: parent.width
        SectionLayout {
            Label {
                text: qsTr("Bump Amount")
                tooltip: qsTr("Strength of bump/normal map effect")
            }
            SecondColumnLayout {
                SpinBox {
                    maximumValue: 999999
                    minimumValue: -999999
                    decimals: 2
                    backendValue: backendValues.bumpAmount
                    Layout.fillWidth: true
                }
            }
            // ### blumpMap

        }
    }

    // ### normapMap

    Section {
        caption: "Translucency"
        width: parent.width
        SectionLayout {
            Label {
                text: qsTr("Translucency Falloff")
            }
            SecondColumnLayout {
                SpinBox {
                    maximumValue: 999999
                    minimumValue: -999999
                    decimals: 2
                    backendValue: backendValues.translucentFalloff
                    Layout.fillWidth: true
                }
            }
        }
    }

    Section {
        caption: "Diffuse Light Wrap"
        width: parent.width
        SectionLayout {
            Label {
                text: qsTr("Diffuse Light Wrap")
            }
            SecondColumnLayout {
                SpinBox {
                    maximumValue: 1
                    minimumValue: 0
                    decimals: 2
                    backendValue: backendValues.diffuseLightWrap
                    Layout.fillWidth: true
                }
            }
        }
    }

    Section {
        caption: "Vertex Colors"
        width: parent.width
        SectionLayout {
            Label {
                text: "Enable Vertex Colors"
                tooltip: qsTr("Use vertex colors from the mesh")
            }
            SecondColumnLayout {
                CheckBox {
                    text: backendValues.vertexColors.valueToString
                    backendValue: backendValues.vertexColors
                    Layout.fillWidth: true
                }
            }
        }
    }
}
