import QtQuick
import QtQuick.Controls
import QtQuick.Templates as T
import Sketch.App

// The icon-button/grid-cell primitive: replaces the ~15 hand-rolled
// "Rectangle + Image/Text + MouseArea(hoverEnabled) + ToolTip" blocks across
// ToolPanel.qml and MainWindow.qml. Built on AbstractButton (not
// Rectangle+MouseArea) so it is natively Tab-focusable and Space/Enter-
// activatable — custom MouseArea-based "buttons" are invisible to keyboard
// navigation entirely, which is the accessibility gap this replaces.
T.AbstractButton {
    id: root

    // Content: set exactly one. iconSource for SVG icons, passed the same
    // "icons/xxx.svg" root-relative way every other file in this codebase
    // uses — resolved below with a "../" prefix since this component lives
    // one directory down, in components/.
    property string iconSource: ""
    property string glyph: ""
    // Font family + icon-mode flag for glyph-mode call sites that want an
    // icon-font pictogram (e.g. Material Design Icons) rather than a short
    // bold text label. Defaults preserve every existing glyph-mode call site
    // (ToolPanel ring digits, R-group labels, atom-bar labels) unchanged.
    property string glyphFontFamily: Theme.fontDisplay
    property bool glyphIsIcon: false

    // tip doubles as the ToolTip text and the default accessible name.
    property string tip: ""
    property string accessibleName: root.tip

    // Sizing — defaults match the existing ToolCell/toolbar-delegate convention.
    property real cellWidth: Theme.toolCellSize
    property real cellHeight: Theme.toolCellSize
    property real iconSize: Theme.toolIconSize

    // Selection/emphasis, driven by the caller (currentTool comparisons etc).
    property bool selected: false
    property bool showSelectionBorder: true
    property bool borderAlwaysVisible: false
    property color accentColor: Theme.accent
    property bool mirrorIcon: false
    // Category color that always wins over the normal selected/hover text color
    // (e.g. R-group cells: always Theme.rgroupColor, regardless of selection).
    // hasGlyphColorOverride is a real flag, not a color-vs-string sentinel check —
    // QML `color` values and JS strings are never strictly equal via !==/===, so an
    // earlier "transparent" sentinel comparison here was always true, silently
    // making every non-overridden glyph render fully transparent (invisible text).
    property bool hasGlyphColorOverride: false
    property color glyphColor: Theme.textPrimary

    implicitWidth: cellWidth
    implicitHeight: cellHeight
    hoverEnabled: true
    focusPolicy: Qt.TabFocus

    Accessible.role: Accessible.Button
    Accessible.name: root.accessibleName
    Accessible.onPressAction: root.clicked()

    Keys.onReturnPressed: root.clicked()
    Keys.onSpacePressed: root.clicked()

    // Fluent tactile press cue: a subtle scale-down while held, springing back on release.
    scale: root.pressed ? 0.94 : 1.0
    Behavior on scale { NumberAnimation { duration: Theme.durationFast; easing.type: Easing.BezierSpline; easing.bezierCurve: root.pressed ? Theme.easingAccelerate : Theme.easingDecelerate } }

    background: Rectangle {
        radius: 4
        color: root.selected ? Theme.selected : (root.hovered && root.enabled ? Theme.hover : "transparent")
        border.color: root.accentColor
        border.width: (root.borderAlwaysVisible || (root.selected && root.showSelectionBorder)) ? 1 : 0
        Behavior on color { ColorAnimation { duration: Theme.durationFast; easing.type: Easing.BezierSpline; easing.bezierCurve: Theme.easingDecelerate } }
    }

    contentItem: Item {
        Image {
            anchors.centerIn: parent
            visible: root.iconSource !== ""
            source: root.iconSource !== "" ? "../" + root.iconSource : ""
            width: root.iconSize
            height: root.iconSize
            sourceSize: Qt.size(root.iconSize, root.iconSize)
            mirror: root.mirrorIcon
            opacity: !root.enabled ? Theme.disabledOpacity
                     : root.selected ? 1.0
                     : (root.hovered ? Theme.hoverOpacity : Theme.restOpacity)
            onStatusChanged: {
                if (status === Image.Error) {
                    console.warn("Failed to load icon:", root.iconSource, "(glyphFontFamily: " + root.glyphFontFamily + ")")
                }
            }
        }
        Text {
            textFormat: Text.PlainText
            anchors.centerIn: parent
            visible: root.iconSource === ""
            text: root.glyph
            color: !root.enabled ? Theme.textSecondary
                   : root.hasGlyphColorOverride ? root.glyphColor
                   : root.selected ? root.accentColor : Theme.textPrimary
            font {
                pixelSize: root.glyphIsIcon ? root.iconSize : (root.glyph.length > 2 ? 12 : 18)
                bold: !root.glyphIsIcon
                family: root.glyphFontFamily
            }
        }
    }

    FocusRing {
        anchors.fill: parent
        visible: root.activeFocus
        radius: 4
    }

    ToolTip { text: root.tip; visible: root.hovered && root.tip !== ""; delay: 500 }
}
