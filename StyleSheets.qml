pragma Singleton
import QtQuick

QtObject {
    property string currentName: "Default"

    readonly property var presets: ({
        "Default":  { bondWidth: 2.0, atomRadius: 10, selectionWidth: 12, hoverWidth: 14,
                      baseFontSize: 16, baseBondLength: 40, doubleBondSpacing: 3.0,
                      wedgeTaperRatio: 0.12, hashSpacingFactor: 4.0, wavyAmplitude: 3.0 },
        "ACS 1996": { bondWidth: 1.5, atomRadius: 8,  selectionWidth: 10, hoverWidth: 12,
                      baseFontSize: 12, baseBondLength: 30, doubleBondSpacing: 2.5,
                      wedgeTaperRatio: 0.10, hashSpacingFactor: 3.5, wavyAmplitude: 2.5 },
        "ACS 2009": { bondWidth: 1.0, atomRadius: 7,  selectionWidth: 9,  hoverWidth: 11,
                      baseFontSize: 11, baseBondLength: 28, doubleBondSpacing: 2.0,
                      wedgeTaperRatio: 0.10, hashSpacingFactor: 3.0, wavyAmplitude: 2.0 },
        "Nature":   { bondWidth: 0.8, atomRadius: 6,  selectionWidth: 8,  hoverWidth: 10,
                      baseFontSize: 10, baseBondLength: 24, doubleBondSpacing: 2.0,
                      wedgeTaperRatio: 0.08, hashSpacingFactor: 3.0, wavyAmplitude: 2.0 },
        "Large":    { bondWidth: 3.0, atomRadius: 14, selectionWidth: 16, hoverWidth: 18,
                      baseFontSize: 20, baseBondLength: 50, doubleBondSpacing: 4.0,
                      wedgeTaperRatio: 0.14, hashSpacingFactor: 5.0, wavyAmplitude: 4.0 }
    })

    readonly property var currentSheet: presets[currentName] || presets["Default"]

    property var sheetNames: ["Default", "ACS 1996", "ACS 2009", "Nature", "Large"]

    function applySheet(name) {
        if (presets[name]) {
            currentName = name
        }
    }
}
