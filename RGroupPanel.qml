import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "js/Selection.js" as Selection

TaskDialog {
    id: root
    title: "R-Groups"
    width: 560
    // Bound by MainWindow to the currently-active document's V8Process instance
    // (read-only lookups, e.g. findRGroup below) and its ChemCanvas (mutations,
    // routed through applyPropertyChange — the one documented mutation entry point).
    property var sketch: null
    property var canvas: null
    height: 420
    standardButtons: Dialog.Close

    // Looks up the live rgroup entry (number, range, resth, ifthen, members) for a given
    // R-number from sketch.primitives.rgroups, or null if that R-group isn't defined yet.
    function findRGroup(number) {
        if (!sketch || !sketch.primitives) return null
        const list = sketch.primitives.rgroups || []
        for (let i = 0; i < list.length; i++) {
            if (list[i].number === number) return list[i]
        }
        return null
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Text {
            text: "Define alternate substituents for R1-R8: draw a member structure as an unconnected fragment elsewhere on the canvas, select it, then click \"Add Selected\" under the matching R-group."
            wrapMode: Text.WordWrap
            color: Theme.textSecondary
            font.pixelSize: 11
            Layout.fillWidth: true
        }

        ListView {
            id: rgList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: 8
            spacing: 4

            delegate: Rectangle {
                width: rgList.width
                height: rowLayout.implicitHeight + 12
                color: index % 2 === 0 ? Theme.background : "transparent"
                radius: 4

                property int rgNumber: index + 1
                property var rgEntry: root.findRGroup(rgNumber)

                RowLayout {
                    id: rowLayout
                    anchors.fill: parent
                    anchors.margins: 6
                    spacing: 8

                    Text {
                        text: "R" + rgNumber
                        color: Theme.rgroupColor
                        font.bold: true
                        Layout.preferredWidth: 28
                    }

                    Button {
                        text: rgEntry ? "Remove Group" : "Define"
                        onClicked: {
                            if (rgEntry) canvas.applyPropertyChange("rgroupDelete", rgNumber)
                            else canvas.applyPropertyChange("rgroupDefine", rgNumber)
                        }
                    }

                    TextField {
                        Layout.preferredWidth: 70
                        placeholderText: "Range"
                        enabled: rgEntry !== null
                        text: rgEntry ? rgEntry.range : ""
                        onEditingFinished: {
                            if (rgEntry) canvas.applyPropertyChange("rgroupLogic", rgNumber, { range: text, resth: rgEntry.resth, ifthen: rgEntry.ifthen })
                        }
                    }

                    CheckBox {
                        text: "ResH"
                        enabled: rgEntry !== null
                        checked: rgEntry ? rgEntry.resth : false
                        onToggled: {
                            if (rgEntry) canvas.applyPropertyChange("rgroupLogic", rgNumber, { range: rgEntry.range, resth: checked, ifthen: rgEntry.ifthen })
                        }
                    }

                    Text {
                        text: rgEntry ? (rgEntry.members.length + " member(s)") : "not defined"
                        color: Theme.textSecondary
                        Layout.fillWidth: true
                    }

                    Button {
                        text: "Add Selected"
                        enabled: rgEntry !== null && Selection.hasAtoms(sketch, 1)
                        onClicked: canvas.applyPropertyChange("rgroupAddMember", rgNumber)
                    }

                    Button {
                        text: "Remove Last"
                        enabled: rgEntry !== null && rgEntry.members.length > 0
                        onClicked: {
                            if (rgEntry && rgEntry.members.length > 0)
                                canvas.applyPropertyChange("rgroupRemoveMember", rgNumber, rgEntry.members[rgEntry.members.length - 1].fragId)
                        }
                    }
                }
            }
        }
    }
}
