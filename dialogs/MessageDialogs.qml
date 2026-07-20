import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore
import "../js/Selection.js" as Selection
import "../components"
import ".."

Item {
    id: root
    required property var win

    property alias unsavedChangesDialog: unsavedChangesDialog
    property alias workerErrorDialog: workerErrorDialog
    property alias similarityResultDialog: similarityResultDialog
    property alias similarityRankResultDialog: similarityRankResultDialog

    MessageDialog {
        id: unsavedChangesDialog
        title: "Unsaved Changes"
        buttons: MessageDialog.Save | MessageDialog.Discard | MessageDialog.Cancel
        text: win._pendingCloseDocId >= 0 ? "This document has unsaved changes. Save before closing?" : "You have unsaved changes. Do you want to save them before exiting?"

        onButtonClicked: function(button) {
            if (button === MessageDialog.Save) {
                win.fileDialogsGroup.saveDialog.open()
            } else if (button === MessageDialog.Discard) {
                if (win._pendingCloseDocId >= 0) {
                    var docId = win._pendingCloseDocId
                    win._pendingCloseDocId = -1
                    DocumentManager.closeDocument(docId)
                } else {
                    win._forceQuit = true
                    Qt.quit()
                }
            }
            // Cancel: reset pending state
            if (button === MessageDialog.Cancel) {
                win._pendingCloseDocId = -1
            }
        }
    }

    MessageDialog {
        id: workerErrorDialog
        property string errorText: ""
        title: "Chemistry Engine Error"
        buttons: MessageDialog.Ok
        text: "The chemistry engine encountered an error and may have stopped.\n\n" + errorText +
              "\n\nPlease save your work and restart the application."
    }

    MessageDialog {
        id: similarityResultDialog
        title: "Similarity Result"
        text: ""
    }

    MessageDialog {
        id: similarityRankResultDialog
        title: "Similarity Ranking"
        text: ""
    }

}
