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

    property alias renameDialog: renameDialog
    property alias textDialog: textDialog
    property alias checkResultDialog: checkResultDialog
    property alias smilesDialog: smilesDialog
    property alias similarityDialog: similarityDialog
    property alias ionizeDialog: ionizeDialog
    property alias inchiLoadDialog: inchiLoadDialog
    property alias shortcutsDialog: shortcutsDialog

    TaskDialog {
        id: renameDialog
        title: "Rename Tab"
        width: 300
        standardButtons: Dialog.Ok | Dialog.Cancel

        property int docId: -1

        TextField {
            id: renameInput
            width: parent.width
            Keys.onReturnPressed: renameDialog.accept()
        }
        onOpened: {
            renameInput.text = win.titleFor(renameDialog.docId)
            renameInput.selectAll()
            renameInput.forceActiveFocus()
        }
        onAccepted: {
            const name = renameInput.text.trim()
            if (name.length > 0 && docId >= 0) {
                win.docTitles[docId] = name
                win.titleRev++
            }
        }
    }

    TaskDialog {
        id: textDialog
        title: textId >= 0 ? "Edit Text" : "Add Text"
        width: 360
        standardButtons: Dialog.Ok | Dialog.Cancel

        property int textId: -1
        property real chemX: 0
        property real chemY: 0
        property string inputText: ""

        ColumnLayout {
            width: parent.width
            spacing: 8

            TextArea {
                id: textDialogInput
                Layout.fillWidth: true
                Layout.preferredHeight: 80
                wrapMode: TextArea.Wrap
                placeholderText: "Annotation text…"
            }
            Button {
                text: "Delete Text"
                visible: textDialog.textId >= 0
                onClicked: {
                    if (win.activeSketch) win.activeSketch.deleteText(textDialog.textId)
                    textDialog.close()
                }
            }
        }

        onOpened: {
            textDialogInput.text = inputText
            textDialogInput.forceActiveFocus()
        }
        onAccepted: {
            if (!win.activeSketch) return
            const content = textDialogInput.text.trim()
            if (textId >= 0) {
                if (content.length > 0) win.activeSketch.updateText(textId, content)
                else win.activeSketch.deleteText(textId)
            } else if (content.length > 0) {
                win.activeSketch.addText(content, chemX, chemY)
            }
        }
    }

    TaskDialog {
        id: checkResultDialog
        title: "Structure Validation"
        standardButtons: Dialog.Ok
        width: 480

        property string reportText: ""

        ScrollView {
            width: 450
            height: Math.min(300, checkResultContent.implicitHeight + 20)
            clip: true

            Text {
                id: checkResultContent
                width: 440
                text: {
                    const r = checkResultDialog.reportText
                    if (!r || r === "{}") return "No issues found."
                    try {
                        const obj = JSON.parse(r)
                        const keys = Object.keys(obj)
                        if (keys.length === 0) return "No issues found."
                        return keys.map(function(k) {
                            const v = obj[k]
                            if (Array.isArray(v))
                                return v.map(function(e) { return (e.text || e) }).join("\n")
                            return String(v)
                        }).join("\n\n")
                    } catch (_) {
                        return r
                    }
                }
                color: Theme.textPrimary
                font { pixelSize: Theme.fontSizeBody; family: Theme.fontDisplay }
                wrapMode: Text.WordWrap
            }
        }
    }

    TaskDialog {
        id: smilesDialog
        title: "Load from SMILES"
        standardButtons: Dialog.Ok | Dialog.Cancel

        TextField {
            id: smilesInput
            width: 380
            placeholderText: "e.g. c1ccccc1 or CC(=O)Oc1ccccc1C(=O)O"
            Keys.onReturnPressed: smilesDialog.accept()
        }

        onOpened: { smilesInput.text = ""; smilesInput.forceActiveFocus() }
        onAccepted: {
            const smi = smilesInput.text.trim()
            if (smi) {
                win.isProcessing = true
                win.indigoSvc.layout(smi)
            }
        }
    }

    TaskDialog {
        id: similarityDialog
        title: "Compare Similarity"
        standardButtons: Dialog.Ok | Dialog.Cancel

        TextField {
            id: similarityRefInput
            width: 380
            placeholderText: "Reference SMILES, e.g. c1ccccc1"
            Keys.onReturnPressed: similarityDialog.accept()
        }

        onOpened: { similarityRefInput.text = ""; similarityRefInput.forceActiveFocus() }
        onAccepted: {
            const ref = similarityRefInput.text.trim()
            if (ref && win.activeSketch) {
                win.pendingSimilarityRef = ref
                win.activeSketch.requestSerialize("similarity")
            }
        }
    }

    TaskDialog {
        id: ionizeDialog
        title: "Ionize at pH"
        standardButtons: Dialog.Ok | Dialog.Cancel

        TextField {
            id: ionizePhInput
            width: 380
            placeholderText: "pH, e.g. 7.4"
            validator: DoubleValidator { bottom: 0; top: 14; decimals: 2 }
            Keys.onReturnPressed: ionizeDialog.accept()
        }

        onOpened: { ionizePhInput.text = "7.4"; ionizePhInput.forceActiveFocus() }
        onAccepted: {
            const pH = parseFloat(ionizePhInput.text)
            if (!isNaN(pH) && win.activeSketch) {
                win.isProcessing = true
                win.pendingIonizePh = pH
                win.activeSketch.requestSerialize("ionize")
            }
        }
    }

    TaskDialog {
        id: inchiLoadDialog
        title: "Load from InChI"
        standardButtons: Dialog.Ok | Dialog.Cancel

        ColumnLayout {
            spacing: 4

            TextField {
                id: inchiLoadInput
                Layout.preferredWidth: 460
                placeholderText: "InChI=1S/C6H6/c1-2-4-6-5-3-1/h1-6H"
                Keys.onReturnPressed: inchiLoadDialog.accept()
            }

            // InChIKey (the 27-char hash, e.g. UHOVQNZJYSORNB-UHFFFAOYSA-N) is a
            // one-way hash of an InChI -- there is no algorithm that reverses it
            // back into a structure, unlike the full InChI string. This is exactly
            // the mix-up this dialog exists to prevent, so it's flagged live
            // rather than only after a confusing load failure.
            Text {
                visible: /^[A-Z]{14}-[A-Z]{10}-[A-Z]$/.test(inchiLoadInput.text.trim())
                text: "That looks like an InChIKey, not a full InChI — InChIKey is a one-way hash and can't be loaded back into a structure. Paste the full \"InChI=1S/...\" string instead."
                color: "#c0392b"
                wrapMode: Text.WordWrap
                Layout.preferredWidth: 460
                font.pixelSize: Theme.fontSizeCaption
            }
        }

        onOpened: { inchiLoadInput.text = ""; inchiLoadInput.forceActiveFocus() }
        onAccepted: {
            // Indigo's generic loader auto-detects and parses InChI directly
            // (confirmed: no separate indigo-inchi-plugin call needed for this
            // direction, unlike generating an InChI/InChIKey from a structure,
            // which does need the plugin) -- same layout() call "Load from
            // SMILES" already uses. InChI carries no 2D coordinates, so the
            // layout step here isn't optional the way it might seem.
            const txt = inchiLoadInput.text.trim()
            if (txt) {
                win.isProcessing = true
                win.indigoSvc.layout(txt)
            }
        }
    }

    TaskDialog {
        id: shortcutsDialog
        title: "Keyboard Shortcuts"
        standardButtons: Dialog.Ok
        width: 420
        // Explicit height, not left to Dialog's own implicit sizing: an inner
        // item's explicit `height:` override does not retroactively change
        // its own implicitHeight, so Dialog's outer frame (sized from content
        // implicitHeight) came out too short for the ScrollView's real
        // (explicit) height, and content rendered past the frame's bottom
        // edge, outside the modal panel entirely -- confirmed via a live
        // screenshot before adding this. Same bug class already hit and
        // fixed once in components/StatusPlaceholder.qml.
        height: 420

        // Sourced verbatim from MainWindow.qml's real Shortcut{} blocks and
        // AppMenus.qml's MenuItemRow shortcutHint values -- not invented.
        ScrollView {
            width: 380
            height: Math.min(340, shortcutsGrid.implicitHeight + 8)
            clip: true

            Grid {
                id: shortcutsGrid
                columns: 2
                columnSpacing: 16
                rowSpacing: 4

                // Flat, alternating key/action list -- same pattern PropertyPanel.qml
                // uses for its SDF Data Fields grid (one Repeater with an isKey flag
                // per entry), since a two-column Grid lays out its children in flat
                // document order, not by pairing two separate Repeaters' outputs.
                Repeater {
                    model: {
                        const shortcuts = [
                            ["S", "Selection tool"],
                            ["E", "Erase tool"],
                            ["B", "Single bond tool"],
                            ["R", "Benzene ring tool"],
                            ["Ctrl+N", "New Document"],
                            ["Ctrl+O", "Open…"],
                            ["Ctrl+S", "Save"],
                            ["Ctrl+Shift+S", "Save As…"],
                            ["Ctrl+I", "Load from SMILES…"],
                            ["Ctrl+B", "Biopolymer…"],
                            ["Ctrl+R", "R-Groups…"],
                            ["Ctrl+W", "Close current tab"],
                            ["Ctrl+Z", "Undo"],
                            ["Ctrl+Shift+Z / Ctrl+Y", "Redo"],
                            ["Ctrl+X", "Cut"],
                            ["Ctrl+C", "Copy"],
                            ["Ctrl+V", "Paste"],
                            ["Ctrl+Shift+C", "Copy as Image"],
                            ["Ctrl+0", "Fit to Screen"]
                        ]
                        const arr = []
                        for (let i = 0; i < shortcuts.length; i++) {
                            arr.push({ isKey: true, val: shortcuts[i][0] })
                            arr.push({ isKey: false, val: shortcuts[i][1] })
                        }
                        return arr
                    }
                    delegate: Text {
                        required property var modelData
                        text: modelData.val
                        color: modelData.isKey ? Theme.textSecondary : Theme.textPrimary
                        font {
                            pixelSize: Theme.fontSizeBody
                            family: modelData.isKey ? Theme.fontMono : Theme.fontDisplay
                        }
                    }
                }
            }
        }
    }

}
