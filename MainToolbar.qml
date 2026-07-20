import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "js/Selection.js" as Selection

Rectangle {
    id: root
    required property var win
    property alias pageSizeCombo: pageSizeCombo

    // Material Design Icons font, used only by the align/distribute glyph
    // icons in this toolbar's Repeater below (moved here from MainWindow.qml
    // during the toolbar extraction -- it had no other consumer).
    FontLoader {
        id: mdiFont
        source: "fonts/materialdesignicons-webfont.ttf"
    }

    Layout.fillWidth: true
    Layout.preferredHeight: Theme.toolbarHeight + 6
    color: Theme.surface
    border {
        color: Theme.outline
        width: 1
    }

    RowLayout {
        anchors {
            fill: parent
            leftMargin: 16
        }
        spacing: 4

        // FILE GROUP
        RowLayout {
            spacing: 4
            Repeater {
                model: [
                    { id: "OPEN", icon: "open.svg", label: "Open" },
                    { id: "SAVE", icon: "save.svg", label: "Save" }
                ]
                delegate: IconCell {
                    id: fileToolbarDelegate
                    required property var modelData
                    required property int index
                    Layout.alignment: Qt.AlignVCenter
                    iconSource: "icons/" + fileToolbarDelegate.modelData.icon
                    tip: fileToolbarDelegate.modelData.label
                    onClicked: {
                        if (fileToolbarDelegate.modelData.id === "SAVE") root.win.saveActive(false)
                        else if (fileToolbarDelegate.modelData.id === "OPEN") root.win.openDialog.open()
                    }
                }
            }
        }

        Rectangle { width: 1; height: 24; color: Theme.outline; Layout.alignment: Qt.AlignVCenter; Layout.leftMargin: 4; Layout.rightMargin: 4 }

        // EDIT GROUP
        RowLayout {
            spacing: 4
            Repeater {
                model: [
                    { id: "UNDO", icon: "undo.svg", label: "Undo" },
                    { id: "REDO", icon: "redo.svg", label: "Redo" },
                    { id: "CUT", icon: "cut.svg", label: "Cut" },
                    { id: "COPY", icon: "copy.svg", label: "Copy" },
                    { id: "PASTE", icon: "paste.svg", label: "Paste" }
                ]
                delegate: IconCell {
                    id: editToolbarDelegate
                    required property var modelData
                    required property int index
                    Layout.alignment: Qt.AlignVCenter
                    iconSource: "icons/" + editToolbarDelegate.modelData.icon
                    tip: editToolbarDelegate.modelData.label
                    enabled: {
                        if (editToolbarDelegate.modelData.id === "UNDO") return !!(root.win.activeCanvas && root.win.activeCanvas.canUndo)
                        if (editToolbarDelegate.modelData.id === "REDO") return !!(root.win.activeCanvas && root.win.activeCanvas.canRedo)
                        return true
                    }
                    onClicked: {
                        if (editToolbarDelegate.modelData.id === "UNDO") root.win.activeCanvas.undo()
                        else if (editToolbarDelegate.modelData.id === "REDO") root.win.activeCanvas.redo()
                        else if (editToolbarDelegate.modelData.id === "COPY") root.win.activeCanvas.copySelection()
                        else if (editToolbarDelegate.modelData.id === "CUT") root.win.activeCanvas.cutSelection()
                        else if (editToolbarDelegate.modelData.id === "PASTE") root.win.activeCanvas.pasteSelection()
                    }
                }
            }
        }

        Rectangle { width: 1; height: 24; color: Theme.outline; Layout.alignment: Qt.AlignVCenter; Layout.leftMargin: 4; Layout.rightMargin: 4 }

        // STRUCTURE GROUP
        Repeater {
            model: [
                { id: "check",       icon: "check.svg",        tip: "Validate structure" },
                { id: "rgroups",     icon: "rgroup-label.svg", tip: "R-Groups… (Ctrl+R)" }
            ]
            delegate: IconCell {
                id: structOpDelegate
                required property var modelData
                Layout.alignment: Qt.AlignVCenter
                iconSource: "icons/" + structOpDelegate.modelData.icon
                tip: structOpDelegate.modelData.tip
                enabled: {
                    return structOpDelegate.modelData.id === "rgroups" || !root.win.isProcessing
                }
                onClicked: {
                    root.win.executeStructureOp(structOpDelegate.modelData.id)
                }
            }
        }

        Rectangle { width: 1; height: 24; color: Theme.outline; Layout.alignment: Qt.AlignVCenter; Layout.leftMargin: 4; Layout.rightMargin: 4 }

        Text {
            text: "STYLE"
            color: Theme.textSecondary
            font { pixelSize: Theme.fontSizeCaption; bold: true; letterSpacing: 2; family: Theme.fontDisplay }
            Layout.alignment: Qt.AlignVCenter
        }
        SegmentedControl {
            Layout.alignment: Qt.AlignVCenter
            model: StyleSheets.sheetNames
            currentValue: StyleSheets.currentName
            onValueSelected: (value) => StyleSheets.applySheet(value)
        }

        Text {
            text: "CHIRAL"
            color: Theme.textSecondary
            font { pixelSize: Theme.fontSizeCaption; bold: true; letterSpacing: 2; family: Theme.fontDisplay }
            Layout.alignment: Qt.AlignVCenter
        }
        ComboBox {
            id: stereoFlagsCombo
            model: ["Absolute", "Relative"]
            currentIndex: 0
            onActivated: if (root.win.activeSketch) root.win.activeSketch.setStereoFlags(currentIndex === 0 ? "abs" : "rel", 0)
            Binding on currentIndex {
                value: (root.win.activeSketch && root.win.activeSketch.primitives && root.win.activeSketch.primitives.stereoFlags && root.win.activeSketch.primitives.stereoFlags.type === "rel") ? 1 : 0
                restoreMode: Binding.RestoreBinding
            }
        }

        Rectangle { width: 1; height: 20; color: Theme.outline; opacity: 0.6 }

        Repeater {
            model: [
                { mode: "left",       fn: "align",       glyph: "\u{f11c2}", tip: "Align left edges",  minAtoms: 2 },
                { mode: "right",      fn: "align",       glyph: "\u{f11c4}", tip: "Align right edges", minAtoms: 2 },
                { mode: "top",        fn: "align",       glyph: "\u{f11c7}", tip: "Align top edges",   minAtoms: 2 },
                { mode: "bottom",     fn: "align",       glyph: "\u{f11c5}", tip: "Align bottom edges", minAtoms: 2 },
                { mode: "horizontal", fn: "distribute",  glyph: "\u{f11c9}", tip: "Distribute horizontally", minAtoms: 3 },
                { mode: "vertical",   fn: "distribute",  glyph: "\u{f11cc}", tip: "Distribute vertically",   minAtoms: 3 }
            ]
            delegate: IconCell {
                id: alignDelegate
                required property var modelData
                Layout.alignment: Qt.AlignVCenter
                glyph: alignDelegate.modelData.glyph
                glyphFontFamily: mdiFont.name
                glyphIsIcon: true
                tip: alignDelegate.modelData.tip
                enabled: Selection.hasAtoms(root.win.activeSketch, alignDelegate.modelData.minAtoms)
                onClicked: {
                    if (!root.win.activeSketch) return
                    if (alignDelegate.modelData.fn === "align") root.win.activeSketch.alignAtoms(alignDelegate.modelData.mode)
                    else root.win.activeSketch.distributeAtoms(alignDelegate.modelData.mode)
                }
            }
        }

        Rectangle { width: 1; height: 20; color: Theme.outline; opacity: 0.6 }

        Repeater {
            model: [
                { mode: "rotate_ccw", icon: "transform-rotate.svg", tip: "Rotate selection 90° counter-clockwise", mirror: false },
                { mode: "rotate_cw", icon: "transform-rotate.svg", tip: "Rotate selection 90° clockwise", mirror: true },
                { mode: "flip_h", icon: "transform-flip-h.svg", tip: "Flip selection horizontally", mirror: false },
                { mode: "flip_v", icon: "transform-flip-v.svg", tip: "Flip selection vertically", mirror: false }
            ]
            delegate: IconCell {
                id: xformDelegate
                required property var modelData
                Layout.alignment: Qt.AlignVCenter
                iconSource: "icons/" + xformDelegate.modelData.icon
                tip: xformDelegate.modelData.tip
                mirrorIcon: xformDelegate.modelData.mirror
                enabled: Selection.hasAtoms(root.win.activeSketch, 2)
                onClicked: if (root.win.activeSketch) root.win.activeSketch.transformSelection(xformDelegate.modelData.mode)
            }
        }

        Item { Layout.fillWidth: true } // spacer

        Text {
            text: "PAGE"
            color: Theme.textSecondary
            font { pixelSize: Theme.fontSizeCaption; bold: true; letterSpacing: 2; family: Theme.fontDisplay }
            Layout.alignment: Qt.AlignVCenter
        }

        ComboBox {
            id: pageSizeCombo
            model: ["A4", "A3", "A5", "Letter"]
            currentIndex: 0
        }
    } // end RowLayout
}
