import QtQuick
import QtQuick.Layouts
import Sketch.App

Rectangle {
    id: root
    color: Theme.surface
    Layout.preferredHeight: Theme.menuBarHeight

    Rectangle {
        width: parent.width
        height: 1
        color: Theme.outline
        anchors.bottom: parent.bottom
    }

    property var activePopup: null

    function requestOpen(popup) {
        if (activePopup && activePopup !== popup) {
            activePopup.close()
        }
        activePopup = popup
        if (popup) {
            popup.open()
        }
    }

    default property alias items: row.data

    Row {
        id: row
        anchors.fill: parent
        spacing: 0
    }
}
