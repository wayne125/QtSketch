import QtQuick
import QtQuick.Templates as T
import Sketch.App
import "../js/ThumbnailPainter.js" as ThumbnailPainter

// Consolidates ToolPanel.qml's three near-duplicate popup grid delegates
// (ssDelegate, fgDelegate, libDelegate) — same hover Rectangle + Canvas
// thumbnail + label Text + click handling, previously copy-pasted with a
// forked paint function per delegate. thumbData is null while its thumbnail
// hasn't loaded yet (async request); the Canvas simply stays blank until then.
T.AbstractButton {
    id: root

    property string label: ""
    property var thumbData: null

    implicitWidth: 105
    implicitHeight: 44
    hoverEnabled: true
    focusPolicy: Qt.TabFocus

    Accessible.role: Accessible.Button
    Accessible.name: root.label

    Keys.onReturnPressed: root.clicked()
    Keys.onSpacePressed: root.clicked()

    background: Rectangle {
        radius: 3
        color: root.hovered ? Theme.hover : "transparent"
        border.color: root.hovered ? Theme.accent : Theme.outline
        border.width: root.hovered ? 2 : 1
        Behavior on border.color { ColorAnimation { duration: 150 } }
        Behavior on color { ColorAnimation { duration: 150 } }
    }

    contentItem: Item {
        Canvas {
            id: thumbCanvas
            anchors { left: parent.left; top: parent.top; margins: 2 }
            width: 101
            height: 24
            renderStrategy: Canvas.Threaded
            renderTarget: Canvas.FramebufferObject
            onPaint: ThumbnailPainter.paint(getContext("2d"), width, height, root.thumbData, Theme.textPrimary.toString())
            Component.onCompleted: requestPaint()
            Connections {
                target: root
                function onThumbDataChanged() { thumbCanvas.requestPaint() }
            }
        }
        Text {
            anchors { left: parent.left; bottom: parent.bottom; leftMargin: 4; bottomMargin: 2 }
            text: root.label
            color: Theme.textPrimary
            font.pixelSize: 9
            elide: Text.ElideRight
            width: parent.width - 8
            horizontalAlignment: Text.AlignHCenter
        }
    }

    FocusRing {
        anchors.fill: parent
        visible: root.activeFocus
        radius: 3
    }
}
