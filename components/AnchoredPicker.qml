import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Sketch.App

// Lightweight, centered-overlay picker popup — replaces ToolPanel.qml's three
// near-identical raw Popup blocks (ssPopup/fgPopup/libPopup), each of which
// hand-rolled its own title Text, "Loading…" Text, and centering math.
// Callers declare their body content (a GridView, or a ComboBox+GridView
// column) as direct children; `title`/`loading` cover what was duplicated.
//
// Built on Popup rather than folded into Dialog/TaskDialog: Popup and Dialog
// are different QQC2 types with different capabilities (Dialog owns
// standardButtons/accepted()/rejected()) — a single "universal" popup would
// have to hand-roll Dialog's button machinery on top of Popup, reintroducing
// the kind of bespoke per-screen logic this component exists to remove.
Popup {
    id: root
    parent: Overlay.overlay
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    padding: Theme.marginSmall

    property string title: ""
    property bool loading: false
    default property alias body: bodyContainer.data

    contentItem: ColumnLayout {
        spacing: 4

        PopupHeader {
            title: root.title
            visible: root.title !== ""
            Layout.alignment: Qt.AlignHCenter
        }

        Item {
            id: bodyContainer
            Layout.fillWidth: true
            Layout.fillHeight: true

            StatusPlaceholder {
                anchors.centerIn: parent
                mode: "loading"
                visible: root.loading
            }
        }
    }
}
