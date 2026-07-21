import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Sketch.App

// Replaces three different loading/empty-state idioms: ToolPanel.qml's plain
// "Loading…" Text (gated on list.count === 0), and PropertyPanel.qml's
// bespoke "Nothing selected" block. message defaults per mode but callers
// may override it (e.g. PropertyPanel's two-line empty-state copy).
Item {
    id: root
    property string mode: "loading"  // "loading" | "empty"
    property string message: mode === "loading" ? "Loading…" : "Nothing selected"
    property string detail: ""       // optional second line, used by "empty" mode

    implicitWidth: col.implicitWidth
    implicitHeight: col.implicitHeight
    // A plain Item's implicitWidth/Height only becomes its actual width/height
    // automatically inside a Layout (which is how PropertyPanel.qml already
    // used this component, inside a ColumnLayout) -- a non-Layout usage
    // anchored directly (e.g. `StatusPlaceholder { anchors.centerIn: parent }`)
    // stays 0x0 without this explicit binding, which made this component
    // render with zero size and no visible content despite `visible: true`.
    width: implicitWidth
    height: implicitHeight

    Accessible.role: Accessible.Indicator
    Accessible.name: root.message + (root.detail ? (" " + root.detail) : "")

    ColumnLayout {
        id: col
        anchors.centerIn: parent
        spacing: 8

        BusyIndicator {
            visible: root.mode === "loading"
            running: visible
            Layout.alignment: Qt.AlignHCenter
        }
        Text {
            textFormat: Text.PlainText
            text: root.message
            color: root.mode === "loading" ? Theme.textSecondary : Theme.textPrimary
            font { pixelSize: Theme.fontSizeBody; family: Theme.fontDisplay }
            Layout.alignment: Qt.AlignHCenter
        }
        Text {
            textFormat: Text.PlainText
            visible: root.detail !== ""
            text: root.detail
            color: Theme.textSecondary
            font.pixelSize: Theme.fontSizeLabel
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
