pragma Singleton
import QtQuick

QtObject {
    // ── Every preset below is sourced from a real file/measurement, not guessed ──
    // baseBondLength/bondWidth/doubleBondSpacing/wedgeTaperRatio/hashSpacingFactor are all
    // derived from real ChemDraw document-property data (kCDXProp_BondLength/BoldWidth/
    // LineWidth/BondSpacing/HashSpacing) extracted directly from the binary .cdx/.cds files in
    // "Journal Templates/", or (ACD/ChemSketch) measured against that app's own on-screen ruler.
    // Conversion: 1pt = 0.352778mm; Theme.pixelsPerCm = 37.8 -> 3.78px/mm -> 1.3335px/pt.
    // wedgeTaperRatio has no direct ChemDraw equivalent (we render wedge width as a ratio of
    // bond length; ChemDraw's Bold Width is an absolute value) - derived per preset as
    // BoldWidth_px / baseBondLength_px so a full-length wedge's tip equals the real Bold Width.
    // atomRadius/selectionWidth/hoverWidth/baseFontSize/wavyAmplitude have no source in either
    // the CDX files or a ChemSketch ruler reading (UI affordances / font size wasn't
    // extractable) - derived from this app's own pre-existing size ratio against bond length
    // (fontSize=0.4L, atomRadius=0.25L, selectionWidth=0.30L, hoverWidth=0.35L,
    // wavyAmplitude=0.075L), not an independent per-journal source.
    property string currentName: "ACD/ChemSketch"

    readonly property var presets: ({
        // Measured directly off ACD/ChemSketch (Freeware)'s own mm ruler at 100% zoom: two
        // independent ring measurements (4.6mm, 5.36mm) averaged to 5.0mm bond length. Secondary
        // params (line/bold width, hash spacing, bond spacing) aren't independently measurable
        // from a screenshot, so they're carried over from the closest verified journal spec
        // (JCE below, whose bond length is nearly identical) at the same ratio - wedgeTaperRatio
        // is scale-invariant so it comes out identical to JCE's.
        "ACD/ChemSketch": { bondWidth: 0.92, atomRadius: 4.7, selectionWidth: 5.7, hoverWidth: 6.6,
                            baseFontSize: 7.6, baseBondLength: 18.9, doubleBondSpacing: 3.40,
                            wedgeTaperRatio: 0.139, hashSpacingFactor: 3.28, wavyAmplitude: 1.42 },
        // Beilstein Journal of Organic Chemistry ChemDraw template
        // (Beilstein_ChemDraw_Template.cds): BondLength 11.5pt, LineWidth 0.5pt, BoldWidth
        // 1.6pt, BondSpacing 18%, HashSpacing 2.0pt.
        "Beilstein": { bondWidth: 0.67, atomRadius: 3.8, selectionWidth: 4.6, hoverWidth: 5.4,
                       baseFontSize: 6.1, baseBondLength: 15.34, doubleBondSpacing: 2.76,
                       wedgeTaperRatio: 0.139, hashSpacingFactor: 2.67, wavyAmplitude: 1.15 },
        // Journal of Chemical Education template (JCE_2018_ChemDraw.cdx): BondLength 14.4pt,
        // LineWidth 0.7pt, BoldWidth 2.0pt, BondSpacing 18%, HashSpacing 2.5pt - this file's own
        // embedded values, used as-is since they're the more directly-verifiable ground truth
        // than a generic spec. 14.4pt/5.08mm bond length happens to match ACS's own published
        // "ACS Document 1996" style sheet exactly, but JCE tunes LineWidth slightly heavier
        // (0.7pt vs ACS's own 0.6pt) - that's the one real difference from the ACS 1996 preset
        // below, split out into its own entry rather than folded together.
        "JCE": { bondWidth: 0.93, atomRadius: 4.8, selectionWidth: 5.8, hoverWidth: 6.7,
                 baseFontSize: 7.7, baseBondLength: 19.2, doubleBondSpacing: 3.46,
                 wedgeTaperRatio: 0.139, hashSpacingFactor: 3.33, wavyAmplitude: 1.44 },
        // ACS's own published "ACS Document 1996" style sheet (pubs.acs.org/page/4authors/
        // submission/graphics_prep.html, cross-confirmed via a second independent source):
        // Fixed Length 14.4pt, Chain Angle 120 deg, Bond Spacing 18% of length, Bold Width
        // 2.0pt, Line Width 0.6pt, Margin Width 1.6pt, Hash Spacing 2.5pt. Identical to JCE
        // above except LineWidth (0.6pt here vs JCE's file-embedded 0.7pt).
        "ACS 1996": { bondWidth: 0.80, atomRadius: 4.8, selectionWidth: 5.8, hoverWidth: 6.7,
                      baseFontSize: 7.7, baseBondLength: 19.2, doubleBondSpacing: 3.46,
                      wedgeTaperRatio: 0.139, hashSpacingFactor: 3.33, wavyAmplitude: 1.44 },
        // Royal Society of Chemistry author templates (ga.cdx/single.cdx/double.cdx - identical
        // values across all three): BondLength 12.15pt, LineWidth 0.4pt, BoldWidth 1.55pt,
        // BondSpacing 20%, HashSpacing 1.75pt. (AR+SPR.cdx is near-identical, 12.1pt/0.35pt/
        // 1.5pt/20%/1.75pt - not split into its own preset.)
        "RSC": { bondWidth: 0.53, atomRadius: 4.1, selectionWidth: 4.9, hoverWidth: 5.7,
                 baseFontSize: 6.5, baseBondLength: 16.20, doubleBondSpacing: 3.24,
                 wedgeTaperRatio: 0.128, hashSpacingFactor: 2.33, wavyAmplitude: 1.22 }
    })

    readonly property var currentSheet: presets[currentName] || presets["ACD/ChemSketch"]

    property var sheetNames: ["ACD/ChemSketch", "Beilstein", "JCE", "ACS 1996", "RSC"]

    function applySheet(name) {
        if (presets[name]) {
            currentName = name
        }
    }
}
