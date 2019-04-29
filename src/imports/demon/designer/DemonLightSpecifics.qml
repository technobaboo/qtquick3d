import QtQuick 2.12
import HelperWidgets 2.0
import QtQuick.Layouts 1.12

Column {
    width: parent.width

     Section {
        width: parent.width
        caption: qsTr("Diffuse Color")

        ColorEditor {
            caption: qsTr("Diffuse Color")
            backendValue: backendValues.diffuseColor
            supportGradient: false
        }
    }

    Section {
        width: parent.width
        caption: qsTr("Emissive Color")

        ColorEditor {
            caption: qsTr("Emissive Color")
            backendValue: backendValues.emissiveColor
            supportGradient: false
        }
    }

    Section {
        width: parent.width
        caption: qsTr("Specular Tint Color")

        ColorEditor {
            caption: qsTr("Specular Tint Color")
            backendValue: backendValues.specularTint
            supportGradient: false
        }
    }

    DemonLightSection {
        width: parent.width
    }

    DemonNodeSection {
        width: parent.width
    }

    DemonObjectSection {
        width: parent.width
    }
}
