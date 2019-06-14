import QtQuick 2.12
import HelperWidgets 2.0
import QtQuick.Layouts 1.12

Section {
    caption: qsTr("Node")

    SectionLayout {

        Label {
            text: qsTr("Opacity")
            tooltip: qsTr("Set local opacity on node")
        }

        // ### should be a slider
        SpinBox {
            maximumValue: 1.0
            minimumValue: 0.0
            decimals: 2
            backendValue: backendValues.opacity
            Layout.fillWidth: true
        }

        Label {
            text: qsTr("is Visible")
            tooltip: qsTr("Set local visiblity of the item")
        }
        SecondColumnLayout {
            // ### should be a slider
            CheckBox {
                text: backendValues.visible.valueToString
                backendValue: backendValues.visible
                Layout.fillWidth: true
            }
        }

        Label {
            text: qsTr("Orientation")
            tooltip: qsTr("The handedness of the transformation")
        }
        SecondColumnLayout {
            ComboBox {
                scope: "DemonNode"
                model: ["LeftHanded", "RightHanded"]
                backendValue: backendValues.orientation
                Layout.fillWidth: true
            }
        }

        Label {
            text: qsTr("Translation")
            tooltip: qsTr("Position Translation")
        }
        SecondColumnLayout {
            ColumnLayout {
                RowLayout {
                    Label {
                        text: qsTr("X")
                    }
                    SpinBox {
                        maximumValue: 9999999
                        minimumValue: -9999999
                        decimals: 2
                        backendValue: backendValues.x
                        Layout.fillWidth: true
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("Y")
                    }
                    SpinBox {
                        maximumValue: 9999999
                        minimumValue: -9999999
                        decimals: 2
                        backendValue: backendValues.y
                        Layout.fillWidth: true
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("Z")
                    }
                    SpinBox {
                        maximumValue: 9999999
                        minimumValue: -9999999
                        decimals: 2
                        backendValue: backendValues.z
                        Layout.fillWidth: true
                    }
                }
            }
        }

        Label {
            text: qsTr("Rotation")
            tooltip: qsTr("Rotation in degrees")
        }
        SecondColumnLayout {
            ColumnLayout {
                RowLayout {
                    Label {
                        text: qsTr("X")
                    }
                    SpinBox {
                        maximumValue: 9999999
                        minimumValue: -9999999
                        decimals: 2
                        backendValue: backendValues.rotation_x
                        Layout.fillWidth: true
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("Y")
                    }
                    SpinBox {
                        maximumValue: 9999999
                        minimumValue: -9999999
                        decimals: 2
                        backendValue: backendValues.rotation_y
                        Layout.fillWidth: true
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("Z")
                    }
                    SpinBox {
                        maximumValue: 9999999
                        minimumValue: -9999999
                        decimals: 2
                        backendValue: backendValues.rotation_z
                        Layout.fillWidth: true
                    }
                }
            }
        }

        Label {
            text: qsTr("Rotation Order")
            tooltip: qsTr("Order that rotation operations are performed")
        }
        SecondColumnLayout {
            ComboBox {
                Layout.fillWidth: true
                scope: "DemonNode"
                model: ["XYZ", "YZX", "ZXY", "XZY", "YXZ", "ZYX", "XYZr", "YZXr", "ZXYr", "XZYr", "YXZr", "ZYXr"]
                backendValue: backendValues.rotationOrder
            }
        }

        Label {
            text: qsTr("Scale")
            tooltip: qsTr("Scale")
        }

        SecondColumnLayout {
            ColumnLayout {
                RowLayout {
                    Label {
                        text: qsTr("X")
                    }
                    SpinBox {
                        maximumValue: 9999999
                        minimumValue: -9999999
                        decimals: 2
                        backendValue: backendValues.scale_x
                        Layout.fillWidth: true
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("Y")
                    }
                    SpinBox {
                        maximumValue: 9999999
                        minimumValue: -9999999
                        decimals: 2
                        backendValue: backendValues.scale_y
                        Layout.fillWidth: true
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("Z")
                    }
                    SpinBox {
                        maximumValue: 9999999
                        minimumValue: -9999999
                        decimals: 2
                        backendValue: backendValues.scale_z
                        Layout.fillWidth: true
                    }
                }
            }
        }

        Label {
            text: qsTr("Pivot")
        }
        SecondColumnLayout {
            ColumnLayout {
                RowLayout {
                    Label {
                        text: qsTr("X")
                    }
                    SpinBox {
                        maximumValue: 9999999
                        minimumValue: -9999999
                        decimals: 2
                        backendValue: backendValues.pivot_x
                        Layout.fillWidth: true
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("Y")
                    }
                    SpinBox {
                        maximumValue: 9999999
                        minimumValue: -9999999
                        decimals: 2
                        backendValue: backendValues.pivot_y
                        Layout.fillWidth: true
                    }
                }
                RowLayout {
                    Label {
                        text: qsTr("Z")
                    }
                    SpinBox {
                        maximumValue: 9999999
                        minimumValue: -9999999
                        decimals: 2
                        backendValue: backendValues.pivot_z
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
