import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

TaskDialog {
    id: root
    title: "Atom Properties"
    width: Theme.dialogWidthSmall
    standardButtons: Dialog.Ok | Dialog.Cancel

    property int atomId: -1
    property string atomLabel: ""

    signal propertiesChanged(int atomId, int isotope, int radical, int valence)

    function openForAtom(id, props) {
        atomId    = id
        atomLabel = props.label || ""
        titleLabel.text = "Element: " + (props.label || "?")
        isotopeBox.value      = props.isotope || 0
        radicalCombo.currentIndex    = props.radical || 0
        // valence: -1 → index 0 (Auto), 0..8 → index 1..9
        valenceCombo.currentIndex = props.explicitValence >= 0 ? props.explicitValence + 1 : 0
        open()
    }

    onAccepted: {
        const valence = valenceCombo.currentIndex > 0 ? valenceCombo.currentIndex - 1 : -1
        root.propertiesChanged(atomId, isotopeBox.value, radicalCombo.currentIndex, valence)
    }

    ColumnLayout {
        anchors { fill: parent; margins: 8 }
        spacing: 10

        Label {
            id: titleLabel
            font { pixelSize: 13; bold: true }
            color: Theme.textPrimary
        }

        GridLayout {
            columns: 3
            columnSpacing: 8
            rowSpacing: 10
            Layout.fillWidth: true

            Label { text: "Isotope:"; color: Theme.textSecondary; font.pixelSize: 12 }
            SpinBox {
                id: isotopeBox
                from: 0; to: 300; value: 0
                Layout.preferredWidth: 90
                textFromValue: function(v) { return v === 0 ? "natural" : v.toString() }
                valueFromText: function(t) { return t === "natural" ? 0 : (parseInt(t) || 0) }
            }
            Label {
                text: isotopeBox.value === 0 ? "" : ("Mass: " + isotopeBox.value)
                color: Theme.textSecondary; font.pixelSize: 10
            }

            Label { text: "Radical:"; color: Theme.textSecondary; font.pixelSize: 12 }
            ComboBox {
                id: radicalCombo
                model: ["None", "Monoradical  ·", "Singlet  ↑↓", "Triplet  ↑↑"]
                Layout.columnSpan: 2
                Layout.fillWidth: true
            }

            Label { text: "Valence:"; color: Theme.textSecondary; font.pixelSize: 12 }
            ComboBox {
                id: valenceCombo
                model: ["Auto", "0", "1", "2", "3", "4", "5", "6", "7", "8"]
                Layout.columnSpan: 2
                Layout.fillWidth: true
            }
        }

        Label {
            text: "Tip: isotope 0 = natural abundance, radical 0 = no radical, valence Auto = computed."
            font.pixelSize: 10
            color: Theme.textSecondary
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }
}
