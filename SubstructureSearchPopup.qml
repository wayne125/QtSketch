import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

AnchoredPicker {
    id: root
    width: 320
    height: 160
    title: "Search Substructure (SMARTS)"

    signal searchRequested(string smarts)
    signal clearRequested()

    property string statusText: ""
    property bool statusIsError: false

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.marginLarge
        spacing: Theme.spacingMedium

        TextField {
            id: smartsInput
            Layout.fillWidth: true
            placeholderText: "e.g. c1ccccc1 (aromatic ring)"
            onAccepted: root.searchRequested(text)
        }
        RowLayout {
            Layout.fillWidth: true
            Button {
                text: "Search"
                onClicked: root.searchRequested(smartsInput.text)
            }
            Button {
                text: "Clear"
                onClicked: { root.clearRequested(); root.statusText = "" }
            }
            Item { Layout.fillWidth: true }
        }
        Text {
            textFormat: Text.PlainText
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            text: root.statusText
            color: root.statusIsError ? Theme.error : Theme.textSecondary
            font { pixelSize: Theme.fontSizeLabel; family: Theme.fontDisplay }
        }
    }
}
