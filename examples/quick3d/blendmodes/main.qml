import QtQuick 2.11
import QtQuick.Window 2.11
import QtGraphicalEffects 1.0

Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Blend Modes Example")

    Item {
        id: controlArea
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left

        width: 100

        ListView {
            id: modeList
            anchors.fill: parent
            model: modeModel
            delegate: Item {
                height: 20
                width: 100
                Text {
                    text: mode
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: modeList.currentIndex = index
                }
            }
            highlight: Rectangle {
                color: "lightgreen"
            }
            focus: true
        }
    }

    ListModel {
        id: modeModel
        ListElement {
            mode: "normal"
        }
        ListElement {
            mode: "addition"
        }
        ListElement {
            mode: "average"
        }
        ListElement {
            mode: "color"
        }
        ListElement {
            mode: "colorBurn"
        }
        ListElement {
            mode: "colorDodge"
        }
        ListElement {
            mode: "darken"
        }
        ListElement {
            mode: "darkenColor"
        }
        ListElement {
            mode: "difference"
        }
        ListElement {
            mode: "divide"
        }
        ListElement {
            mode: "exclusion"
        }
        ListElement {
            mode: "hardLight"
        }
        ListElement {
            mode: "hue"
        }
        ListElement {
            mode: "lighten"
        }
        ListElement {
            mode: "lighterColor"
        }
        ListElement {
            mode: "lightness"
        }
        ListElement {
            mode: "multiply"
        }
        ListElement {
            mode: "negation"
        }
        ListElement {
            mode: "saturation"
        }
        ListElement {
            mode: "screen"
        }
        ListElement {
            mode: "subtract"
        }
        ListElement {
            mode: "softLight"
        }
    }

    Item {
        id: viewArea
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.left: controlArea.right

        BackgroundView {
            id: background
            anchors.fill: parent
        }

        ForegroundView {
            id: foreground
            anchors.fill: parent
        }

        Blend {
            anchors.fill: parent
            source: ShaderEffectSource {
                sourceItem: background
                hideSource: true
            }
            foregroundSource: ShaderEffectSource {
                sourceItem: foreground
                hideSource: true
            }
            mode: modeModel.get(modeList.currentIndex).mode
        }
    }


}
