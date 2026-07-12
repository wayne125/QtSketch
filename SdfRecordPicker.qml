import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCore

AnchoredPicker {
    id: root
    width: 280
    height: 400
    title: "Browse records..."

    property alias model: recordList.model
    property int recordCount: 0

    signal recordChosen(int index)

    ColumnLayout {
        anchors.fill: parent
        spacing: 4

        Label {
            Layout.fillWidth: true
            text: root.recordCount > 500 ? "showing first 500 of " + root.recordCount + " records" : root.recordCount + " records"
            font.pixelSize: Theme.fontSizeLabel
            color: Theme.textSecondary
            horizontalAlignment: Text.AlignHCenter
            visible: root.recordCount > 1
        }

        GridView {
            id: recordList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            cellWidth: 110
            cellHeight: 48
            model: []
            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
            delegate: ThumbnailGridItem {
                required property var modelData
                required property int index
                label: modelData.label
                thumbData: modelData.thumb
                onClicked: {
                    root.recordChosen(modelData.index)
                    root.close()
                }
            }
        }
    }
}
