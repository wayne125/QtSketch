import QtQuick
import QtQuick.Controls
import QtQuick.Templates as T
import Sketch.App

// Exclusive-selection, joined-corner control — replaces MainWindow.qml's
// hand-rolled STYLE preset switcher (Row + Repeater + manual first/last-corner
// rounding). Deliberately separate from IconCell: this is a different
// selection model (exactly-one-of-N, joined visually into one control) with
// no icon/glyph content axis, not a variant of the icon-cell primitive.
Row {
    id: root
    property var model: []
    property string currentValue: ""
    signal valueSelected(string value)

    spacing: 0

    Repeater {
        model: root.model
        delegate: T.AbstractButton {
            id: seg
            required property string modelData
            required property int index

            readonly property bool isSelected: root.currentValue === seg.modelData
            readonly property bool isFirst: seg.index === 0
            readonly property bool isLast: seg.index === root.model.length - 1

            implicitWidth: segText.implicitWidth + 16
            implicitHeight: 28
            hoverEnabled: true
            focusPolicy: Qt.TabFocus

            Accessible.role: Accessible.Button
            Accessible.name: seg.modelData
            Accessible.onPressAction: root.valueSelected(seg.modelData)

            Keys.onReturnPressed: root.valueSelected(seg.modelData)
            Keys.onSpacePressed: root.valueSelected(seg.modelData)

            onClicked: root.valueSelected(seg.modelData)

            background: Rectangle {
                color: seg.isSelected ? Theme.accent : Theme.surface
                border.color: seg.isSelected ? Theme.accent : Theme.outline
                border.width: 1
                topLeftRadius: seg.isFirst ? 4 : 0
                bottomLeftRadius: seg.isFirst ? 4 : 0
                topRightRadius: seg.isLast ? 4 : 0
                bottomRightRadius: seg.isLast ? 4 : 0
                Behavior on color { ColorAnimation { duration: 150 } }
            }

            contentItem: Text {
                id: segText
                text: seg.modelData
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color: seg.isSelected ? Theme.surface : Theme.textPrimary
                font { pixelSize: Theme.fontSizeLabel; family: Theme.fontDisplay; bold: seg.isSelected }
            }

            FocusRing {
                anchors.fill: parent
                visible: seg.activeFocus
                radius: 4
            }
        }
    }
}
