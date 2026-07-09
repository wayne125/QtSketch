import QtQuick
import Sketch.App

// Visible keyboard-focus indicator. Custom controls built on AbstractButton draw
// their own `background`, so there is no OS-native focus rectangle — every
// interactive component in components/ owns one of these, shown only while
// `visible` (bound by the caller to `activeFocus`).
Item {
    id: root
    property real radius: 4

    Rectangle {
        anchors.fill: parent
        radius: root.radius
        color: "transparent"
        border.color: Theme.focusRingColor
        border.width: Theme.focusRingWidth
    }
}
