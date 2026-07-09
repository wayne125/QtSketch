import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

AnchoredPicker {
    id: root

    signal elementSelected(string label)

    property var elementsModel: []

    width: 850
    height: 520
    title: "Periodic Table"

    Component.onCompleted: {
        const allElems = Theme.elementsList
        const validElems = []
        for (let i = 0; i < allElems.length; i++) {
            if (allElems[i].number <= 118) {
                validElems.push(allElems[i])
            }
        }
        elementsModel = validElems
    }

    function getIUPACCoords(z) {
        if (z === 1) return { c: 0, r: 0 };
        if (z === 2) return { c: 17, r: 0 };
        if (z >= 3 && z <= 4) return { c: z - 3, r: 1 };
        if (z >= 5 && z <= 10) return { c: z - 5 + 12, r: 1 };
        if (z >= 11 && z <= 12) return { c: z - 11, r: 2 };
        if (z >= 13 && z <= 18) return { c: z - 13 + 12, r: 2 };
        if (z >= 19 && z <= 36) return { c: z - 19, r: 3 };
        if (z >= 37 && z <= 54) return { c: z - 37, r: 4 };
        if (z >= 55 && z <= 56) return { c: z - 55, r: 5 };
        if (z >= 57 && z <= 71) return { c: z - 57 + 3, r: 7.5 }; // Lanthanides
        if (z >= 72 && z <= 86) return { c: z - 72 + 3, r: 5 };
        if (z >= 87 && z <= 88) return { c: z - 87, r: 6 };
        if (z >= 89 && z <= 103) return { c: z - 89 + 3, r: 8.5 }; // Actinides
        if (z >= 104 && z <= 118) return { c: z - 104 + 3, r: 6 };
        return { c: 0, r: 0 };
    }

    function getColorForType(type, number) {
        if (number === 1) return Theme.getColorForType("nonmetal")
        return Theme.getColorForType(type)
    }

    Item {
        anchors.fill: parent

        Repeater {
            model: root.elementsModel
            delegate: Rectangle {
                required property var modelData
                required property int index
                property var coords: root.getIUPACCoords(modelData.number)
                x: coords.c * 44 + 10
                y: coords.r * 44 + 10
                width: 42
                height: 42
                radius: 4
                color: ptMouse.containsMouse ? Qt.lighter(root.getColorForType(modelData.type, modelData.number), 1.1) : root.getColorForType(modelData.type, modelData.number)
                border {
                    color: ptMouse.containsMouse ? Theme.accent : Qt.darker(color, 1.2)
                    width: ptMouse.containsMouse ? 2 : 1
                }

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0
                    Text {
                        text: modelData.number
                        font.pixelSize: 8
                        color: Theme.textSecondary
                        Layout.alignment: Qt.AlignLeft
                        Layout.leftMargin: 2
                        Layout.topMargin: 2
                    }
                    Text {
                        text: modelData.label
                        font {
                            pixelSize: 14
                            bold: true
                        }
                        color: Theme.textPrimary
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Item { Layout.fillHeight: true }
                }

                MouseArea {
                    id: ptMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        root.elementSelected(modelData.label)
                        root.close()
                    }
                }
            }
        }
    }
}
