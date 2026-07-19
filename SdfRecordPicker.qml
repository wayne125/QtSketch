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
    signal findScaffoldRequested()
    signal decomposeRequested()
    signal rankBySimilarityRequested()
    signal alignToScaffoldRequested()
    signal exportGridRequested()
    signal exportBatchFileRequested()

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

        Button {
            Layout.fillWidth: true
            text: "Find Common Scaffold"
            visible: root.recordCount > 1
            onClicked: root.findScaffoldRequested()
        }

        Button {
            Layout.fillWidth: true
            text: "Decompose to R-Groups"
            visible: root.recordCount > 1
            onClicked: root.decomposeRequested()
        }

        Button {
            Layout.fillWidth: true
            text: "Rank by Similarity to Active Structure"
            visible: root.recordCount > 1
            onClicked: root.rankBySimilarityRequested()
        }

        Button {
            Layout.fillWidth: true
            text: "Align Batch to Common Scaffold"
            visible: root.recordCount > 1
            onClicked: root.alignToScaffoldRequested()
        }

        Button {
            Layout.fillWidth: true
            text: "Export Batch as Image Grid…"
            visible: root.recordCount > 1
            onClicked: root.exportGridRequested()
        }

        Button {
            Layout.fillWidth: true
            text: "Export Batch to File…"
            visible: root.recordCount > 1
            onClicked: root.exportBatchFileRequested()
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
