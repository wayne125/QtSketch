import QtQuick
import Sketch.App

// Shared caption style for popup/dialog titles — centralizes what was
// previously copy-pasted at each of ToolPanel.qml's three popups.
Text {
    property string title: ""
    text: title
    color: Theme.textSecondary
    font { pixelSize: Theme.fontSizeCaption; bold: true; letterSpacing: 1; family: Theme.fontDisplay }
}
