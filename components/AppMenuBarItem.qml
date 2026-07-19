import QtQuick
import QtQuick.Controls
import QtQuick.Templates as T
import Sketch.App

T.AbstractButton {
    id: control
    default property alias menu: popupContent.data

    function close() { popup.close() }

    width: contentText.implicitWidth + 24
    height: parent.height

    Accessible.role: Accessible.MenuItem
    Keys.onReturnPressed: clicked()
    Keys.onSpacePressed: clicked()

    property var menuBar: {
        var p = parent
        while (p && typeof p.requestOpen === "undefined") {
            p = p.parent
        }
        return p
    }

    onClicked: {
        if (popup.opened) {
            popup.close()
            if (menuBar) menuBar.activePopup = null
        } else {
            if (menuBar) menuBar.requestOpen(popup)
        }
    }

    onHoveredChanged: {
        if (hovered && menuBar && menuBar.activePopup && menuBar.activePopup !== popup) {
            menuBar.requestOpen(popup)
        }
    }

    background: Rectangle {
        color: popup.opened ? Theme.selected : (control.hovered ? Theme.hover : "transparent")
    }

    contentItem: Text {
        id: contentText
        text: control.text
        color: Theme.textPrimary
        font { pixelSize: Theme.fontSizeBody; family: Theme.fontFamily }
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    FocusRing {
        anchors.fill: parent
        visible: control.activeFocus
        radius: 0
    }

    Popup {
        id: popup
        y: control.height
        padding: 4
        background: Rectangle {
            color: Theme.surface
            border.color: Theme.outline
            border.width: 1
            radius: 4
        }

        onClosed: {
            if (menuBar && menuBar.activePopup === popup) {
                menuBar.activePopup = null
            }
        }

        contentItem: Column {
            id: popupContent
            spacing: 2
        }
    }
}
