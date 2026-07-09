import QtQuick
import QtQuick.Controls
import Sketch.App

// Modal, centered task dialog — the shared base for every "form + OK/Cancel"
// dialog in this app (previously each one repeated `modal: true` and
// `anchors.centerIn: parent` on its own root Dialog). Deliberately minimal:
// it does not auto-inject a status label into contentItem/footer, since that
// slot varies per dialog's own layout — callers that want the optional
// statusText/statusColor idiom place a Label bound to them in their own
// layout, same as BiopolymerDialog's existing local statusLabel already does.
Dialog {
    modal: true
    anchors.centerIn: parent

    property string statusText: ""
    property color statusColor: Theme.textSecondary
    property bool busy: false
}
