import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    property Item canvas
    // Null-safe accessors: `canvas` is bound to activeCanvas, which is null for a
    // moment at startup before the per-document Repeater instantiates. Every binding
    // below reads these instead of dereferencing canvas directly.
    readonly property var selAtom: canvas ? canvas.selectedAtom : null
    readonly property var selBond: canvas ? canvas.selectedBond : null
    readonly property var selArrow: canvas ? canvas.selectedRxnArrow : null

    property string molName: ""
    property var sdfProps: ({})
    property double molMW:       0
    property double molMono:     0
    property string molFormula:  ""
    property int    molAtoms:    0
    property int    molBonds:    0
    property double molTPSA:     0
    property double molLogP:     0
    property double molMolarRefractivity: 0
    property double molPka: 0
    property int    molHeavyAtoms: 0
    property bool   molIsChiral: false
    property double molMostAbundantMass: 0
    property int    molFragmentCount: 0
    property int    molRingCount: 0
    property int    molHBA:      0
    property int    molHBD:      0
    property int    molRotBonds: 0

    function _fmtNum(v, decimals) { return v.toFixed(decimals) }
    function _formulaHtml(f) { return f.replace(/([0-9]+)/g, "<sub>$1</sub>") }

    color: Theme.surface

    ColumnLayout {
        anchors {
            fill: parent
            margins: Theme.marginLarge
        }
        spacing: Theme.spacingLarge

        Text {
            textFormat: Text.PlainText
            text: "Properties"
            color: Theme.textSecondary
            font {
                pixelSize: Theme.fontSizeCaption
                bold: true
                letterSpacing: 2
                family: Theme.fontDisplay
            }
            Layout.alignment: Qt.AlignHCenter
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Theme.outline
        }

        // Molecular Properties (auto-updated)
        ColumnLayout {
            Layout.fillWidth: true
            visible: root.molAtoms > 0
            spacing: 6

            TextField {
                id: molNameInput
                Layout.fillWidth: true
                placeholderText: "Untitled"
                text: root.molName
                font { pixelSize: Theme.fontSizeBody; family: Theme.fontFamily }
                onEditingFinished: {
                    if (root.canvas && root.canvas.sketch) {
                        root.canvas.sketch.sendCommand("setMoleculeName", [text])
                    }
                    focus = false
                }
                Keys.onEscapePressed: {
                    text = root.molName
                    focus = false
                }
            }

            Text {
                textFormat: Text.RichText
                text: root._formulaHtml(root.molFormula)
                color: Theme.textPrimary
                font {
                    pixelSize: Theme.fontSizeDisplay
                    bold: true
                    family: Theme.fontDisplay
                }
                Layout.alignment: Qt.AlignHCenter
            }

            Grid {
                columns: 2
                columnSpacing: 12
                rowSpacing: 4
                Layout.fillWidth: true

                Text { textFormat: Text.PlainText; text: "MW";        color: Theme.textSecondary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontDisplay } }
                Text { textFormat: Text.PlainText; text: root._fmtNum(root.molMW, 3) + " g/mol"; color: Theme.textPrimary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontMono } }
                Text { textFormat: Text.PlainText; text: "Exact";     color: Theme.textSecondary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontDisplay } }
                Text { textFormat: Text.PlainText; text: root._fmtNum(root.molMono, 4) + " Da";   color: Theme.textPrimary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontMono } }
                Text { textFormat: Text.PlainText; text: "Atoms";     color: Theme.textSecondary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontDisplay } }
                Text { textFormat: Text.PlainText; text: root.molAtoms.toString();                 color: Theme.textPrimary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontMono } }
                Text { textFormat: Text.PlainText; text: "Bonds";     color: Theme.textSecondary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontDisplay } }
                Text { textFormat: Text.PlainText; text: root.molBonds.toString();                 color: Theme.textPrimary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontMono } }
            }

            Rectangle { Layout.fillWidth: true; height: 1; color: Theme.outline; opacity: 0.5 }

            Text {
                textFormat: Text.PlainText
                text: "Drug Properties"
                color: Theme.textSecondary
                font { pixelSize: Theme.fontSizeCaption; bold: true; letterSpacing: 1; family: Theme.fontDisplay }
            }

            Grid {
                columns: 2
                columnSpacing: 12
                rowSpacing: 4
                Layout.fillWidth: true

                Text { textFormat: Text.PlainText; text: "TPSA";   color: Theme.textSecondary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontDisplay } }
                Text { textFormat: Text.PlainText; text: root._fmtNum(root.molTPSA, 2) + " Å²";  color: Theme.textPrimary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontMono } }

                Text { textFormat: Text.PlainText; text: "LogP";   color: Theme.textSecondary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontDisplay } }
                Text { textFormat: Text.PlainText; text: root._fmtNum(root.molLogP, 2);            color: Theme.textPrimary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontMono } }

                Text { textFormat: Text.PlainText; text: "HBA";    color: Theme.textSecondary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontDisplay } }
                Text { textFormat: Text.PlainText; text: root.molHBA.toString();                   color: Theme.textPrimary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontMono } }

                Text { textFormat: Text.PlainText; text: "HBD";    color: Theme.textSecondary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontDisplay } }
                Text { textFormat: Text.PlainText; text: root.molHBD.toString();                   color: Theme.textPrimary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontMono } }

                Text { textFormat: Text.PlainText; text: "RotB";   color: Theme.textSecondary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontDisplay } }
                Text { textFormat: Text.PlainText; text: root.molRotBonds.toString();              color: Theme.textPrimary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontMono } }

                Text { textFormat: Text.PlainText; text: "MolRef";  color: Theme.textSecondary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontDisplay } }
                Text { textFormat: Text.PlainText; text: root._fmtNum(root.molMolarRefractivity, 2);  color: Theme.textPrimary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontMono } }

                Text { textFormat: Text.PlainText; text: "pKa";     color: Theme.textSecondary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontDisplay } }
                Text { textFormat: Text.PlainText; text: root._fmtNum(root.molPka, 2);            color: Theme.textPrimary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontMono } }

                Text { textFormat: Text.PlainText; text: "Heavy Atoms"; color: Theme.textSecondary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontDisplay } }
                Text { textFormat: Text.PlainText; text: root.molHeavyAtoms.toString(); color: Theme.textPrimary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontMono } }

                Text { textFormat: Text.PlainText; text: "Chiral"; color: Theme.textSecondary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontDisplay } }
                Text { textFormat: Text.PlainText; text: root.molIsChiral ? "Yes" : "No"; color: Theme.textPrimary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontMono } }

                Text { textFormat: Text.PlainText; text: "Abundant Mass"; color: Theme.textSecondary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontDisplay } }
                Text { textFormat: Text.PlainText; text: root._fmtNum(root.molMostAbundantMass, 3) + " g/mol"; color: Theme.textPrimary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontMono } }

                Text { textFormat: Text.PlainText; text: "Fragments"; color: Theme.textSecondary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontDisplay } }
                Text { textFormat: Text.PlainText; text: root.molFragmentCount.toString(); color: Theme.textPrimary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontMono } }

                Text { textFormat: Text.PlainText; text: "Rings (SSSR)"; color: Theme.textSecondary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontDisplay } }
                Text { textFormat: Text.PlainText; text: root.molRingCount.toString(); color: Theme.textPrimary; font { pixelSize: Theme.fontSizeLabel; family: Theme.fontMono } }
            }
        }

        // SDF Data Fields (read-only)
        ColumnLayout {
            Layout.fillWidth: true
            visible: Object.keys(root.sdfProps).length > 0
            spacing: 6

            Rectangle { Layout.fillWidth: true; height: 1; color: Theme.outline; opacity: 0.5 }

            Text {
                textFormat: Text.PlainText
                text: "SDF Data Fields"
                color: Theme.textSecondary
                font { pixelSize: Theme.fontSizeCaption; bold: true; letterSpacing: 1; family: Theme.fontDisplay }
            }

            Grid {
                columns: 2
                columnSpacing: 12
                rowSpacing: 4
                Layout.fillWidth: true

                Repeater {
                    model: {
                        var keys = Object.keys(root.sdfProps);
                        var arr = [];
                        for (var i = 0; i < keys.length; i++) {
                            arr.push({ isKey: true, val: keys[i] });
                            arr.push({ isKey: false, val: root.sdfProps[keys[i]] });
                        }
                        return arr;
                    }
                    delegate: Text {
                        textFormat: Text.PlainText
                        text: modelData.val
                        color: modelData.isKey ? Theme.textSecondary : Theme.textPrimary
                        font {
                            pixelSize: Theme.fontSizeLabel
                            family: modelData.isKey ? Theme.fontDisplay : Theme.fontMono
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Theme.outline
            visible: root.molAtoms > 0
        }

        // Sgroup (contracted abbreviation) info
        ColumnLayout {
            Layout.fillWidth: true
            visible: root.selAtom !== null && root.selAtom.isSgroup
            spacing: Theme.spacingMedium

            Text {
                textFormat: Text.PlainText
                text: "Abbreviation"
                color: Theme.textSecondary
                font { pixelSize: Theme.fontSizeCaption; bold: true; letterSpacing: 1; family: Theme.fontDisplay }
            }
            Text {
                textFormat: Text.PlainText
                text: root.selAtom ? root.selAtom.label : ""
                color: Theme.textPrimary
                font { pixelSize: Theme.fontSizeHeadline; bold: true; family: Theme.fontDisplay }
            }
        }

        // Atom Properties (real atoms only)
        ColumnLayout {
            Layout.fillWidth: true
            visible: root.selAtom !== null && !root.selAtom.isSgroup
            spacing: Theme.spacingMedium

            Text {
                textFormat: Text.PlainText
                text: "Atom Label"
                color: Theme.textSecondary
                font { pixelSize: Theme.fontSizeCaption; bold: true; letterSpacing: 1; family: Theme.fontDisplay }
            }
            TextField {
                id: atomLabelInput
                Layout.fillWidth: true
                text: root.selAtom ? root.selAtom.label : ""
                font { pixelSize: Theme.fontSizeBody; family: Theme.fontMono }
                onEditingFinished: {
                    if (root.selAtom && !root.selAtom.isSgroup) {
                        canvas.applyPropertyChange("atomLabel", root.selAtom.id, text)
                    }
                    focus = false
                }
                Keys.onEscapePressed: {
                    text = root.selAtom ? root.selAtom.label : ""
                    focus = false
                }
            }

            Text {
                textFormat: Text.PlainText
                text: "Charge"
                color: Theme.textSecondary
                font { pixelSize: Theme.fontSizeCaption; bold: true; letterSpacing: 1; family: Theme.fontDisplay }
            }
            TextField {
                id: atomChargeInput
                Layout.fillWidth: true
                text: root.selAtom ? (root.selAtom.charge > 0 ? "+" + root.selAtom.charge : root.selAtom.charge.toString()) : "0"
                font { pixelSize: Theme.fontSizeBody; family: Theme.fontMono }
                onEditingFinished: {
                    if (root.selAtom && !root.selAtom.isSgroup) {
                        canvas.applyPropertyChange("atomCharge", root.selAtom.id, text)
                    }
                    focus = false
                }
                Keys.onEscapePressed: {
                    text = root.selAtom ? (root.selAtom.charge > 0 ? "+" + root.selAtom.charge : root.selAtom.charge.toString()) : "0"
                    focus = false
                }
            }

            Text {
                textFormat: Text.PlainText
                text: "Atom List (e.g. C,N,O)"
                color: Theme.textSecondary
                font { pixelSize: Theme.fontSizeCaption; bold: true; letterSpacing: 1; family: Theme.fontDisplay }
            }
            RowLayout {
                Layout.fillWidth: true
                TextField {
                    id: atomListInput
                    Layout.fillWidth: true
                    placeholderText: "leave empty to clear"
                    text: root.selAtom ? root.selAtom.atomListElements : ""
                    font { pixelSize: Theme.fontSizeLabel; family: Theme.fontMono }
                    onEditingFinished: {
                        if (root.selAtom && !root.selAtom.isSgroup) {
                            canvas.applyPropertyChange("atomQueryList", root.selAtom.id,
                                { elements: text, notList: atomListNotCheck.checked })
                        }
                        focus = false
                    }
                    Keys.onEscapePressed: {
                        text = root.selAtom ? root.selAtom.atomListElements : ""
                        focus = false
                    }
                }
                CheckBox {
                    id: atomListNotCheck
                    text: "NOT"
                    checked: root.selAtom ? root.selAtom.atomListNot : false
                    onToggled: {
                        if (root.selAtom && !root.selAtom.isSgroup && atomListInput.text.trim().length > 0) {
                            canvas.applyPropertyChange("atomQueryList", root.selAtom.id,
                                { elements: atomListInput.text, notList: checked })
                        }
                    }
                }
            }
        }

        // Bond Properties
        ColumnLayout {
            Layout.fillWidth: true
            visible: root.selBond !== null
            spacing: Theme.spacingMedium

            Text {
                textFormat: Text.PlainText
                text: "Bond Type"
                color: Theme.textSecondary
                font { pixelSize: Theme.fontSizeCaption; bold: true; letterSpacing: 1; family: Theme.fontDisplay }
            }
            ComboBox {
                id: bondTypeCombo
                Layout.fillWidth: true
                model: ["Single", "Double", "Triple"]
                onActivated: {
                    if (root.selBond) {
                        canvas.applyPropertyChange("bondType", root.selBond.id, currentIndex + 1)
                    }
                }
                Binding on currentIndex {
                    value: root.selBond ? Math.max(0, Math.min(2, root.selBond.type - 1)) : 0
                    restoreMode: Binding.RestoreBinding
                }
            }
        }

        // Reaction Arrow Properties
        ColumnLayout {
            Layout.fillWidth: true
            visible: root.selArrow !== null
            spacing: Theme.spacingMedium

            Text {
                textFormat: Text.PlainText
                text: "Arrow Mode"
                color: Theme.textSecondary
                font { pixelSize: Theme.fontSizeCaption; bold: true; letterSpacing: 1; family: Theme.fontDisplay }
            }
            ComboBox {
                id: arrowModeCombo
                Layout.fillWidth: true
                model: ["Filled Triangle", "Open Angle", "Retrosynthetic", "Equilibrium (↔)", "Equilibrium (⇌)", "Curved Mechanism"]
                onActivated: {
                    if (root.selArrow) {
                        const modes = ["filled-triangle", "open-angle", "retrosynthetic", "equilibrium-FF", "equilibrium-FH", "curved-mechanism"]
                        canvas.applyPropertyChange("rxnArrowMode", root.selArrow.id, modes[currentIndex])
                    }
                }
                Binding on currentIndex {
                    value: {
                        if (!root.selArrow) return 0
                        const map = {"filled-triangle":0, "open-angle":1, "retrosynthetic":2, "equilibrium-FF":3, "equilibrium-FH":4, "curved-mechanism":5}
                        return map[root.selArrow.mode] !== undefined ? map[root.selArrow.mode] : 0
                    }
                    restoreMode: Binding.RestoreBinding
                }
            }

            Text {
                textFormat: Text.PlainText
                text: "Conditions Above"
                color: Theme.textSecondary
                font { pixelSize: Theme.fontSizeCaption; bold: true; letterSpacing: 1; family: Theme.fontDisplay }
            }
            TextField {
                id: condAboveField
                Layout.fillWidth: true
                placeholderText: "e.g. 25°C, Pd/C"
                font { pixelSize: Theme.fontSizeLabel; family: Theme.fontMono }
                onEditingFinished: {
                    if (root.selArrow) {
                        canvas.applyPropertyChange("rxnArrowConditions", root.selArrow.id, {above: text, below: condBelowField.text})
                    }
                    focus = false
                }
                Keys.onEscapePressed: {
                    text = root.selArrow ? root.selArrow.conditionsAbove : ""
                    focus = false
                }
                Binding on text {
                    value: root.selArrow ? root.selArrow.conditionsAbove : ""
                    restoreMode: Binding.RestoreBinding
                }
            }

            Text {
                textFormat: Text.PlainText
                text: "Conditions Below"
                color: Theme.textSecondary
                font { pixelSize: Theme.fontSizeCaption; bold: true; letterSpacing: 1; family: Theme.fontDisplay }
            }
            TextField {
                id: condBelowField
                Layout.fillWidth: true
                placeholderText: "e.g. EtOH, 12h"
                font { pixelSize: Theme.fontSizeLabel; family: Theme.fontMono }
                onEditingFinished: {
                    if (root.selArrow) {
                        canvas.applyPropertyChange("rxnArrowConditions", root.selArrow.id, {above: condAboveField.text, below: text})
                    }
                    focus = false
                }
                Keys.onEscapePressed: {
                    text = root.selArrow ? root.selArrow.conditionsBelow : ""
                    focus = false
                }
                Binding on text {
                    value: root.selArrow ? root.selArrow.conditionsBelow : ""
                    restoreMode: Binding.RestoreBinding
                }
            }
        }

        // Empty state — invitation to act, not just a label
        StatusPlaceholder {
            Layout.fillWidth: true
            visible: root.selAtom === null && root.selBond === null && root.selArrow === null
            mode: "empty"
            message: "Nothing selected"
            detail: "Click an atom, bond, or arrow\nto edit its properties"
        }

        Item { Layout.fillHeight: true }

        Text {
            textFormat: Text.PlainText
            text: "sketch"
            color: Theme.textSecondary
            opacity: 0.5
            font { pixelSize: Theme.fontSizeCaption; family: Theme.fontDisplay; italic: true }
        }
    }
}
