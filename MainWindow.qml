import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore
import "js/Selection.js" as Selection

ApplicationWindow {
    id: window

    flags: Qt.Window | Qt.FramelessWindowHint

    // Material Design Icons font — vendored-but-unused until the toolbar
    // alignment/distribute icon group (below) started using it. Loaded once
    // here; IconCell glyph-mode cells reference mdiFont.name as their family.
    FontLoader {
        id: mdiFont
        source: "fonts/materialdesignicons-webfont.ttf"
    }

    header: Rectangle {
        height: 32
        color: Theme.surface
        RowLayout {
            anchors.fill: parent
            Text {
                text: window.title
                color: Theme.textPrimary
                font { pixelSize: Theme.fontSizeBody; family: Theme.fontDisplay }
                Layout.leftMargin: 16
                Layout.fillWidth: true
            }
            Button { 
                text: "−"
                flat: true
                onClicked: window.showMinimized() 
            }
            Button {
                text: window.visibility === Window.Maximized ? "❐" : "□"
                flat: true
                onClicked: {
                    if (window.visibility === Window.Maximized)
                        window.showNormal()
                    else
                        window.showMaximized()
                }
            }
            Button { 
                text: "✕"
                flat: true
                background: Rectangle {
                    color: parent.down ? "#B22222" : (parent.hovered ? "#E81123" : "transparent")
                }
                contentItem: Text {
                    text: parent.text
                    color: parent.hovered ? "white" : Theme.textPrimary
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: window.close() 
            }
        }
        TapHandler {
            acceptedButtons: Qt.LeftButton
            onTapped: if (tapCount === 2) {
                if (window.visibility === Window.Maximized)
                    window.showNormal()
                else
                    window.showMaximized()
            }
        }
        DragHandler {
            onActiveChanged: if(active) window.startSystemMove()
        }
    }

    // Resize handlers for frameless window. DragHandler is not an Item — it has no
    // x/y/width/height/anchors of its own (Qt 6.11 QML type reference, verified) — so
    // each edge/corner hit-strip needs its own wrapping Item; the handler then acts
    // within that Item's bounds (its documented "parent" scope), not the handler's.
    Item {
        anchors.fill: parent
        Item {
            anchors { top: parent.top; left: parent.left; right: parent.right }
            height: 4
            DragHandler { target: null; cursorShape: Qt.SizeVerCursor; onActiveChanged: if (active) window.startSystemResize(Qt.TopEdge) }
        }
        Item {
            anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
            height: 4
            DragHandler { target: null; cursorShape: Qt.SizeVerCursor; onActiveChanged: if (active) window.startSystemResize(Qt.BottomEdge) }
        }
        Item {
            anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
            width: 4
            DragHandler { target: null; cursorShape: Qt.SizeHorCursor; onActiveChanged: if (active) window.startSystemResize(Qt.LeftEdge) }
        }
        Item {
            anchors { right: parent.right; top: parent.top; bottom: parent.bottom }
            width: 4
            DragHandler { target: null; cursorShape: Qt.SizeHorCursor; onActiveChanged: if (active) window.startSystemResize(Qt.RightEdge) }
        }
        Item {
            anchors { top: parent.top; left: parent.left }
            width: 6; height: 6
            DragHandler { target: null; cursorShape: Qt.SizeFDiagCursor; onActiveChanged: if (active) window.startSystemResize(Qt.TopLeftCorner) }
        }
        Item {
            anchors { top: parent.top; right: parent.right }
            width: 6; height: 6
            DragHandler { target: null; cursorShape: Qt.SizeBDiagCursor; onActiveChanged: if (active) window.startSystemResize(Qt.TopRightCorner) }
        }
        Item {
            anchors { bottom: parent.bottom; left: parent.left }
            width: 6; height: 6
            DragHandler { target: null; cursorShape: Qt.SizeBDiagCursor; onActiveChanged: if (active) window.startSystemResize(Qt.BottomLeftCorner) }
        }
        Item {
            anchors { bottom: parent.bottom; right: parent.right }
            width: 6; height: 6
            DragHandler { target: null; cursorShape: Qt.SizeFDiagCursor; onActiveChanged: if (active) window.startSystemResize(Qt.BottomRightCorner) }
        }
    }

    property bool isProcessing: false
    property bool explicitCheckPending: false
    property real canvasScaleX: 1.0
    property real canvasScaleY: 1.0
    property real zoomLevel: 1.0
    property real pixelsPerCm: 37.8
    property var pageMargins: ({ left: 2.0, right: 2.0, top: 2.0, bottom: 2.0 })

    // The currently-visible ChemCanvas instance and its bound document. Resolved once the
    // per-document Repeater below has instantiated at least one activeCanvas.
    property var activeCanvas: null
    readonly property var activeSketch: DocumentManager.documentFor(DocumentManager.activeDocId)
    property var dirtyDocs: ({})
    property int _pendingCloseDocId: -1

    onActiveCanvasChanged: {
        if (activeCanvas && activeCanvas.sketch) {
            AppController.setV8Process(activeCanvas.sketch)
            // Properties/stereo/check badges only refresh for the active document —
            // kick the debounce so a newly focused tab isn't stale until its next edit.
            propUpdateTimer.restart()
        }
    }

    // Per-document file paths + display titles (UI-only state). titleRev bumps
    // force TabButton text bindings to re-evaluate after open/save-as/rename.
    property var docFilePaths: ({})
    property int titleRev: 0
    property url pendingSaveUrl
    property url pendingRenderUrl
    function setDocFile(docId, fileUrl) {
        docFilePaths[docId] = fileUrl
        const s = fileUrl.toString()
        docTitles[docId] = decodeURIComponent(s.substring(s.lastIndexOf("/") + 1))
        titleRev++
    }
    function saveActive(forceDialog) {
        if (!activeSketch) return
        const path = docFilePaths[DocumentManager.activeDocId]
        if (!forceDialog && path) {
            const fileStr = path.toString().toLowerCase()
            let fmt = "mol"
            if (fileStr.endsWith(".sdf")) fmt = "sdf"
            else if (fileStr.endsWith(".ket")) fmt = "ket"
            pendingSaveUrl = path
            activeSketch.requestStructure(fmt, "save")
            if (activeCanvas) activeCanvas.setClean()
        } else {
            saveDialog.open()
        }
    }

    property var docTitles: ({})
    function titleFor(docId) {
        if (docTitles[docId] === undefined) {
            var n = Object.keys(docTitles).length + 1
            docTitles[docId] = "Untitled " + n
        }
        return docTitles[docId]
    }

    Settings {
        id: uiSettings
        category: "ui"
        property bool darkMode: false
    }
    Component.onCompleted: Theme.darkMode = uiSettings.darkMode

    visible: true
    width: 1400
    height: 900
    title: "sketch" + (activeCanvas && activeCanvas.isDirty ? " *" : "")
    color: Theme.background

    property bool _forceQuit: false

    onClosing: function(close_event) {
        if (window._forceQuit) return
        // Check ALL tabs for unsaved changes, not just the active one
        var anyDirty = false
        for (var i = 0; i < canvasRepeater.count; i++) {
            var item = canvasRepeater.itemAt(i)
            if (item && item.isDirty) { anyDirty = true; break }
        }
        if (anyDirty) {
            close_event.accepted = false
            unsavedChangesDialog.open()
        }
    }

    // Change cursor globally when processing
    ProgressBar {
        id: processingBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 2
        indeterminate: true
        visible: window.isProcessing
        z: 9999
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Indigo Backend
        IndigoService {
            id: indigoSvc
        }

        // Imago OCR Backend — chemical-structure image recognition for Insert Image
        ImagoService {
            id: imagoSvc
            onImageRecognized: function(molfile, warningsCount, error) {
                window.isProcessing = false
                if (!error && molfile && warningsCount <= imageFileDialog.maxAcceptableWarnings && activeSketch) {
                    activeSketch.sendCommand("insertRecognizedStructure", [molfile, imageFileDialog.chemX, imageFileDialog.chemY])
                } else {
                    // Recognition failed or low-confidence — fall back to a plain image embed.
                    const dataUri = fileIO.readImageAsDataUri(imageFileDialog.pendingFileUrl)
                    if (dataUri === "") {
                        workerErrorDialog.errorText = "Failed to load image. Ensure it is a supported format (png, jpg, gif, bmp) and under 5 MB."
                        workerErrorDialog.open()
                    } else if (activeSketch) {
                        activeSketch.addImage(dataUri, imageFileDialog.chemX, imageFileDialog.chemY, 1.5, 1.5)
                    }
                }
            }
        }

        // Global Shortcuts
        Shortcut { sequence: "S"; onActivated: if (activeCanvas) activeCanvas.currentTool = "SELECT" }
        Shortcut { sequence: "E"; onActivated: if (activeCanvas) activeCanvas.currentTool = "ERASE" }
        Shortcut { sequence: "B"; onActivated: if (activeCanvas) activeCanvas.currentTool = "BOND_1" }
        Shortcut { sequence: "R"; onActivated: if (activeCanvas) activeCanvas.currentTool = "TEMPLATE_BENZENE" }
        Shortcut { sequence: "Ctrl+R"; onActivated: rgroupPanel.open() }

        Connections {
            target: indigoSvc
            function onLayoutFinished(newMol) {
                window.isProcessing = false
                if (newMol && activeCanvas) activeCanvas.loadMolfile(newMol)
            }
            function onCommonScaffoldFinished(result, error) {
                window.isProcessing = false
                if (result && activeCanvas) {
                    activeCanvas.loadMolfile(result)
                } else {
                    workerErrorDialog.errorText = "Find Common Scaffold: " + (error || "unknown error")
                    workerErrorDialog.open()
                }
            }
            function onRgroupDecompositionFinished(result, error) {
                window.isProcessing = false
                if (result && activeCanvas) {
                    activeCanvas.loadMolfile(result)
                } else {
                    workerErrorDialog.errorText = "Decompose to R-Groups: " + (error || "unknown error")
                    workerErrorDialog.open()
                }
            }
            function onSimilarityRankFinished(resultJson, error) {
                window.isProcessing = false
                if (resultJson) {
                    try {
                        const arr = JSON.parse(resultJson)
                        arr.sort((a, b) => b.score - a.score)
                        const labels = window._pendingBatchLabels || []
                        const lines = arr.map((r, i) => {
                            const name = labels[r.index] || ("Record " + (r.index + 1))
                            return (i + 1) + ". " + name + " — " + (r.score * 100).toFixed(1) + "%"
                        })
                        similarityRankResultDialog.text = lines.join("\n")
                        similarityRankResultDialog.open()
                    } catch (e) {
                        workerErrorDialog.errorText = "Rank by Similarity: failed to parse results."
                        workerErrorDialog.open()
                    }
                } else {
                    workerErrorDialog.errorText = "Rank by Similarity: " + (error || "unknown error")
                    workerErrorDialog.open()
                }
            }
            function onBatchAlignFinished(resultJson, error) {
                window.isProcessing = false
                if (resultJson && activeSketch) {
                    activeSketch.sendCommand("realignSdfBatch", [resultJson])
                } else {
                    workerErrorDialog.errorText = "Align Batch to Common Scaffold: " + (error || "unknown error")
                    workerErrorDialog.open()
                }
            }
            function onRdfBatchParsed(recordsJson, error) {
                if (recordsJson && activeSketch) {
                    activeSketch.sendCommand("deserializeRdfBatch", [recordsJson])
                } else {
                    window.isProcessing = false
                    workerErrorDialog.errorText = "Open RDF: " + (error || "unknown error")
                    workerErrorDialog.open()
                }
            }
            function onIndigoBatchParsed(recordsJson, error) {
                if (recordsJson && activeSketch) {
                    activeSketch.sendCommand("deserializeIndigoBatch", [recordsJson])
                } else {
                    window.isProcessing = false
                    workerErrorDialog.errorText = "Open batch file: " + (error || "unknown error")
                    workerErrorDialog.open()
                }
            }
            function onClean2dFinished(newMol) {
                window.isProcessing = false
                if (newMol && activeCanvas) activeCanvas.loadMolfile(newMol)
            }
            function onAromatizeFinished(newMol) {
                window.isProcessing = false
                if (newMol && activeCanvas) activeCanvas.loadMolfile(newMol)
            }
            function onDearomatizeFinished(newMol) {
                window.isProcessing = false
                if (newMol && activeCanvas) activeCanvas.loadMolfile(newMol)
            }
            function onUnfoldHydrogensFinished(newMol) {
                window.isProcessing = false
                if (newMol && activeCanvas) activeCanvas.loadMolfile(newMol)
            }
            function onFoldHydrogensFinished(newMol) {
                window.isProcessing = false
                if (newMol && activeCanvas) activeCanvas.loadMolfile(newMol)
            }
            function onNormalizeFinished(newMol) {
                window.isProcessing = false
                if (newMol && activeCanvas) activeCanvas.loadMolfile(newMol)
            }
            function onStandardizeFinished(newMol) {
                window.isProcessing = false
                if (newMol && activeCanvas) activeCanvas.loadMolfile(newMol)
            }
            function onSmilesFinished(smiles) {
                if (smiles) indigoSvc.copyToClipboard(smiles)
            }
            function onCanonicalSmilesFinished(smiles) {
                if (smiles) indigoSvc.copyToClipboard(smiles)
            }
            function onInchiFinished(inchi) {
                if (inchi) indigoSvc.copyToClipboard(inchi)
            }
            function onInchiKeyFinished(inchiKey) {
                if (inchiKey) indigoSvc.copyToClipboard(inchiKey)
            }
            function onHashFinished(hash) {
                if (hash) indigoSvc.copyToClipboard(hash)
            }
            function onSimilarityFinished(result) {
                if (result) {
                    similarityResultDialog.text = "Tanimoto similarity: " + result
                    similarityResultDialog.open()
                }
            }
            function onMassCompositionFinished(result) {
                if (result) indigoSvc.copyToClipboard(result)
            }
            function onPkaValuesFinished(result) {
                if (result) indigoSvc.copyToClipboard(result)
            }
            function onRenderFinished(success, error) {
                if (!success) {
                    const isPdf = window.pendingRenderUrl.toString().toLowerCase().endsWith(".pdf")
                    workerErrorDialog.errorText = (isPdf ? "PDF export failed: " : "SVG export failed: ") + error
                    workerErrorDialog.open()
                }
            }
            function onReactionMappingFinished(result, error) {
                window.isProcessing = false
                if (result && activeCanvas) {
                    activeCanvas.loadMolfile(result)
                } else {
                    workerErrorDialog.errorText = "Atom Mapping: " + (error || "unknown error")
                    workerErrorDialog.open()
                }
            }
            function onIonizeFinished(result, error) {
                window.isProcessing = false
                if (result && activeCanvas) {
                    activeCanvas.loadMolfile(result)
                } else {
                    workerErrorDialog.errorText = "Ionize at pH: " + (error || "unknown error")
                    workerErrorDialog.open()
                }
            }
            function onPropertiesReady(mw, mono, mf, atoms, bonds, tpsa, logp, hba, hbd, rotBonds, molarRefractivity, pka, heavyAtoms, isChiral, mostAbundantMass, fragmentCount, ringCount) {
                propPanel.molMW       = mw
                propPanel.molMono     = mono
                propPanel.molFormula  = mf
                propPanel.molAtoms    = atoms
                propPanel.molBonds    = bonds
                propPanel.molTPSA     = tpsa
                propPanel.molLogP     = logp
                propPanel.molHBA      = hba
                propPanel.molHBD      = hbd
                propPanel.molRotBonds = rotBonds
                propPanel.molMolarRefractivity = molarRefractivity
                propPanel.molPka      = pka
                propPanel.molHeavyAtoms = heavyAtoms
                propPanel.molIsChiral   = isChiral
                propPanel.molMostAbundantMass = mostAbundantMass
                propPanel.molFragmentCount = fragmentCount
                propPanel.molRingCount = ringCount
            }
            function onStereoDescriptorsReady(jsonMap) {
                if (activeSketch) activeSketch.setStereoDescriptors(jsonMap)
            }
            function onCheckIssuesReady(structuredJson) {
                if (activeSketch) activeSketch.setCheckIssues(structuredJson)
            }
            function onSubstructureSearchFinished(resultJson) {
                const parsed = JSON.parse(resultJson)
                if (parsed.error) {
                    smartsSearchPopup.statusIsError = true
                    smartsSearchPopup.statusText = parsed.error
                    return
                }
                smartsSearchPopup.statusIsError = false
                smartsSearchPopup.statusText = parsed.matchCount + " match" + (parsed.matchCount === 1 ? "" : "es") + (parsed.truncated ? " (showing first 200)" : "") + " found"
                if (activeSketch) activeSketch.sendCommand("selectSubstructureMatches", [JSON.stringify(parsed)])
            }
            function onCheckFinished(report) {
                // Inline check warnings (badges, red bonds) are driven by checkIssuesReady,
                // which fires silently on every calc_props cycle. The modal dialog is
                // explicit-only: opened solely when the user clicks the Validate button.
                if (window.explicitCheckPending) {
                    window.explicitCheckPending = false
                    checkResultDialog.reportText = report
                    checkResultDialog.open()
                }
            }

            // Biopolymer load
            function onBiopolymerLoaded(molfile) {
                window.isProcessing = false
                if (activeCanvas) activeCanvas.loadMolfile(molfile)
                biopolymerDialog.onLoadSuccess()
            }
            function onBiopolymerLoadError(error) {
                window.isProcessing = false
                biopolymerDialog.showLoadError(error)
            }

            // Biopolymer export results → shown in dialog textarea
            function onBioSequenceReady(text) { biopolymerDialog.showExportResult("Sequence", text) }
            function onBioFastaReady(text)    { biopolymerDialog.showExportResult("FASTA",    text) }
            function onBioHelmReady(text)     { biopolymerDialog.showExportResult("HELM",     text) }
            function onBioIdtReady(text)      { biopolymerDialog.showExportResult("IDT",      text) }
            function onBioAxoLabsReady(text)  { biopolymerDialog.showExportResult("AxoLabs",  text) }
        }

        Timer {
            id: propUpdateTimer
            interval: 600
            repeat: false
            onTriggered: if (activeSketch) activeSketch.requestSerialize("calc_props")
        }

        Connections {
            target: window.activeSketch
            function onStructureReady(reqId, data) {
                if (reqId === "save") {
                    fileIO.write(window.pendingSaveUrl, data)
                } else if (reqId === "render_svg") {
                    indigoSvc.renderToFile(data, window.pendingRenderUrl, "svg")
                } else if (reqId === "render_pdf") {
                    indigoSvc.renderToFile(data, window.pendingRenderUrl, "pdf")
                } else if (reqId === "render_grid") {
                    indigoSvc.renderReactionGridToFile(data, window.pendingRenderUrl, "pdf")
                } else if (reqId === "automap") {
                    indigoSvc.autoMapReaction(data)
                } else if (reqId === "ionize") {
                    indigoSvc.ionizeAtPh(data, window.pendingIonizePh)
                } else if (reqId === "clear_mapping") {
                    indigoSvc.clearReactionMapping(data)
                } else if (reqId === "correct_reacting_centers") {
                    indigoSvc.correctReactingCenters(data)
                } else if (reqId === "smiles") {
                    indigoSvc.smiles(data)
                } else if (reqId === "canonical_smiles") {
                    indigoSvc.canonicalSmiles(data)
                } else if (reqId === "inchi") {
                    indigoSvc.inchi(data)
                } else if (reqId === "inchikey") {
                    indigoSvc.inchiKey(data)
                } else if (reqId === "hash") {
                    indigoSvc.hash(data)
                } else if (reqId === "similarity") {
                    indigoSvc.similarity(data, window.pendingSimilarityRef)
                } else if (reqId === "mass_composition") {
                    indigoSvc.massComposition(data)
                } else if (reqId === "pka_values") {
                    indigoSvc.pkaValues(data)
                } else if (reqId === "layout") {
                    window.isProcessing = true
                    indigoSvc.layout(data)
                } else if (reqId === "clean2d") {
                    window.isProcessing = true
                    indigoSvc.clean2d(data)
                } else if (reqId === "aromatize") {
                    window.isProcessing = true
                    indigoSvc.aromatize(data)
                } else if (reqId === "dearomatize") {
                    window.isProcessing = true
                    indigoSvc.dearomatize(data)
                } else if (reqId === "unfoldH") {
                    window.isProcessing = true
                    indigoSvc.unfoldHydrogens(data)
                } else if (reqId === "foldH") {
                    window.isProcessing = true
                    indigoSvc.foldHydrogens(data)
                } else if (reqId === "normalize") {
                    window.isProcessing = true
                    indigoSvc.normalize(data)
                } else if (reqId === "standardize") {
                    window.isProcessing = true
                    indigoSvc.standardize(data)
                } else if (reqId === "check") {
                    indigoSvc.checkStructure(data)
                } else if (reqId === "calc_props") {
                    indigoSvc.calcProperties(data)
                    indigoSvc.calcStereoDescriptors(data)
                    indigoSvc.checkStructure(data)
                    if (activeSketch) {
                        activeSketch.sendCommand("getMoleculeName", [])
                        activeSketch.sendCommand("getSdfProps", [])
                    }
                } else if (reqId === "bio_seq") {
                    indigoSvc.exportBioSequence(data)
                } else if (reqId === "bio_fasta") {
                    indigoSvc.exportBioFasta(data)
                } else if (reqId === "bio_helm") {
                    indigoSvc.exportBioHelm(data)
                } else if (reqId === "bio_idt") {
                    indigoSvc.exportBioIdt(data)
                } else if (reqId === "bio_axolabs") {
                    indigoSvc.exportBioAxoLabs(data)
                } else if (reqId === "atom_props") {
                    const props = JSON.parse(data)
                    if (props && props.id !== undefined)
                        atomPropsDialog.openForAtom(props.id, props)
                } else if (reqId === "clipboard_ket") {
                    if (data && data.length > 0 && activeSketch)
                        activeSketch.setOsClipboardText(data)
                } else if (reqId === "sdf_batch_list") {
                    window.isProcessing = false
                    const parsed = JSON.parse(data)
                    if (parsed.count <= 1) {
                        if (activeSketch) activeSketch.sendCommand("loadSdfBatchRecord", [0])
                    } else {
                        sdfRecordPicker.recordCount = parsed.count
                        sdfRecordPicker.model = parsed.records
                        sdfRecordPicker.open()
                    }
                } else if (reqId === "sdf_batch_realigned") {
                    const parsed = JSON.parse(data)
                    sdfRecordPicker.recordCount = parsed.count
                    sdfRecordPicker.model = parsed.records
                } else if (reqId === "sdf_batch_load") {
                    window.isProcessing = false
                    if (activeCanvas) {
                        activeCanvas._needsCentering = true
                        activeCanvas.refresh()
                        setDocFile(DocumentManager.activeDocId, pendingSdfBatchUrl)
                    }
                } else if (reqId === "sdf_batch_molfiles") {
                    const parsed = JSON.parse(data)
                    if (parsed.molfiles && parsed.molfiles.length >= 2) {
                        if (window._pendingBatchAction === "export_grid") {
                            window._pendingBatchGridMolfiles = parsed.molfiles
                            batchGridSaveDialog.open()
                        } else if (window._pendingBatchAction === "export_file") {
                            window._pendingBatchGridMolfiles = parsed.molfiles
                            batchFileSaveDialog.open()
                        } else if (window._pendingBatchAction === "align") {
                            indigoSvc.alignBatchToScaffold(parsed.molfiles)
                        } else if (window._pendingBatchAction === "decompose") {
                            indigoSvc.decomposeToRGroups(parsed.molfiles)
                        } else if (window._pendingBatchAction === "similarity") {
                            window._pendingBatchLabels = parsed.labels || []
                            if (!window.pendingSimilarityRefMolfile) {
                                window.isProcessing = false
                                workerErrorDialog.errorText = "Rank by Similarity: no active structure to compare against."
                                workerErrorDialog.open()
                            } else {
                                indigoSvc.rankBySimilarity(window.pendingSimilarityRefMolfile, parsed.molfiles)
                            }
                        } else {
                            indigoSvc.findCommonScaffold(parsed.molfiles)
                        }
                    } else {
                        window.isProcessing = false
                        workerErrorDialog.errorText = "Batch structure analysis: not enough valid structures in this batch."
                        workerErrorDialog.open()
                    }
                } else if (reqId === "smarts_search") {
                    indigoSvc.substructureSearch(data, window.pendingSmartsQuery)
                } else if (reqId === "mol_name") {
                    propPanel.molName = data
                } else if (reqId === "sdf_props") {
                    propPanel.sdfProps = JSON.parse(data)
                }
            }
            function onStateUpdated(state, selection, dirty, undoState, redoState, result) {
                if (result !== "stereoUpdated" && result !== "checkUpdated")
                    propUpdateTimer.restart()
            }
            function onErrorOccurred(error) {
                window.isProcessing = false
                workerErrorDialog.errorText = error
                workerErrorDialog.open()
            }
        }

        function executeStructureOp(op) {
            if (op === "smartsSearch") { smartsSearchPopup.open(); return }
            if (op === "rgroups") { rgroupPanel.open(); return }
            if (!activeSketch) return
            if (op === "check") {
                window.explicitCheckPending = true
                activeSketch.requestSerialize("check")
                return
            }
            if (op === "layoutSelected") {
                activeSketch.sendCommand("layoutSelectedChain", [])
                return
            }
            if (op !== "layout" && op !== "aromatize") window.isProcessing = true
            activeSketch.requestSerialize(op)
        }

        AppMenuBar {
            Layout.fillWidth: true
            AppMenuBarItem {
                text: "File"
                MenuItemRow { text: "New Document"; onTriggered: DocumentManager.addDocument() }
                MenuItemRow { text: "Open…"; iconSource: "open.svg"; onTriggered: openDialog.open() }
                MenuItemRow { text: "Save"; iconSource: "save.svg"; shortcutHint: "Ctrl+S"; onTriggered: window.saveActive(false) }
                MenuItemRow { text: "Save As…"; iconSource: "save.svg"; onTriggered: window.saveActive(true) }
                Rectangle { width: parent.width; height: 1; color: Theme.outline; opacity: 0.6 }
                MenuItemRow { text: "Load from SMILES…"; iconSource: "smiles_in.svg"; onTriggered: smilesDialog.open() }
                MenuItemRow { text: "Load from InChI…"; iconSource: "smiles_in.svg"; onTriggered: inchiLoadDialog.open() }
                MenuItemRow { text: "Biopolymer…"; iconSource: "biopolymer.svg"; onTriggered: biopolymerDialog.open() }
                Rectangle { width: parent.width; height: 1; color: Theme.outline; opacity: 0.6 }
                MenuItemRow { text: "Export as PDF…"; iconSource: "file-thumbnail.svg"; onTriggered: window.printToPdf() }
                MenuItemRow { text: "Export Reaction Scheme (Grid)…"; iconSource: "file-thumbnail.svg"; onTriggered: gridSaveDialog.open() }
                Rectangle { width: parent.width; height: 1; color: Theme.outline; opacity: 0.6 }
                MenuItemRow { text: "Clear Canvas"; iconSource: "clear.svg"; onTriggered: activeCanvas.clearCanvas() }
            }
            AppMenuBarItem {
                text: "Edit"
                MenuItemRow { text: "Undo"; iconSource: "undo.svg"; shortcutHint: "Ctrl+Z"; enabled: !!(activeCanvas && activeCanvas.canUndo); onTriggered: activeCanvas.undo() }
                MenuItemRow { text: "Redo"; iconSource: "redo.svg"; shortcutHint: "Ctrl+Y"; enabled: !!(activeCanvas && activeCanvas.canRedo); onTriggered: activeCanvas.redo() }
                Rectangle { width: parent.width; height: 1; color: Theme.outline; opacity: 0.6 }
                MenuItemRow { text: "Cut"; iconSource: "cut.svg"; shortcutHint: "Ctrl+X"; onTriggered: activeCanvas.cutSelection() }
                MenuItemRow { text: "Copy"; iconSource: "copy.svg"; shortcutHint: "Ctrl+C"; onTriggered: activeCanvas.copySelection() }
                MenuItemRow { text: "Paste"; iconSource: "paste.svg"; shortcutHint: "Ctrl+V"; onTriggered: activeCanvas.pasteSelection() }
                Rectangle { width: parent.width; height: 1; color: Theme.outline; opacity: 0.6 }
                MenuItemRow { text: "Copy as Image"; iconSource: "copy_image.svg"; shortcutHint: "Ctrl+Shift+C"; onTriggered: activeCanvas.copyAsImage() }
            }
            AppMenuBarItem {
                text: "Structure"
                MenuItemRow { text: "Layout"; iconSource: "layout.svg"; onTriggered: executeStructureOp("layout") }
                MenuItemRow { text: "Layout Selected"; iconSource: "layout.svg"; enabled: Selection.hasAtoms(activeSketch, 1) && !window.isProcessing; onTriggered: executeStructureOp("layoutSelected") }
                MenuItemRow { text: "Clean 2D"; iconSource: "layout.svg"; onTriggered: executeStructureOp("clean2d") }
                Rectangle { width: parent.width; height: 1; color: Theme.outline; opacity: 0.6 }
                MenuItemRow { text: "Aromatize"; iconSource: "arom.svg"; onTriggered: executeStructureOp("aromatize") }
                MenuItemRow { text: "Dearomatize"; iconSource: "dearom.svg"; onTriggered: executeStructureOp("dearomatize") }
                MenuItemRow { text: "Add Explicit H"; iconSource: "explicit-hydrogens.svg"; onTriggered: executeStructureOp("unfoldH") }
                MenuItemRow { text: "Remove Explicit H"; iconSource: "explicit-hydrogens.svg"; onTriggered: executeStructureOp("foldH") }
                MenuItemRow { text: "Normalize"; iconSource: "clean.svg"; onTriggered: executeStructureOp("normalize") }
                MenuItemRow { text: "Standardize"; iconSource: "analyse.svg"; onTriggered: executeStructureOp("standardize") }
                MenuItemRow { text: "Ionize at pH…"; iconSource: "analyse.svg"; onTriggered: ionizeDialog.open() }
                Rectangle { width: parent.width; height: 1; color: Theme.outline; opacity: 0.6 }
                MenuItemRow { text: "Validate"; iconSource: "check.svg"; onTriggered: executeStructureOp("check") }
                MenuItemRow { text: "Search Substructure (SMARTS)…"; iconSource: "search.svg"; onTriggered: executeStructureOp("smartsSearch") }
                MenuItemRow { text: "R-Groups…"; iconSource: "rgroup-label.svg"; shortcutHint: "Ctrl+R"; onTriggered: executeStructureOp("rgroups") }
            }
            AppMenuBarItem {
                id: reactionsMenuTrigger
                text: "Reactions"
                ColumnLayout {
                    width: 210
                    spacing: 6

                    GridLayout {
                        columns: 4
                        columnSpacing: Theme.spacingSmall
                        Layout.alignment: Qt.AlignHCenter
                        Repeater {
                            model: [
                                { id: "RXN_ARROW", icon: "reaction-arrow-open-angle.svg", tip: "Reaction Arrow" },
                                { id: "MULTITAIL_ARROW", icon: "reaction-arrow-multitail.svg", tip: "Multi-tail Arrow" },
                                { id: "RXN_PLUS", icon: "reaction-plus.svg", tip: "Reaction Plus" },
                                { id: "AAM", icon: "reaction-map.svg", tip: "Atom-Atom Mapping" }
                            ]
                            delegate: IconCell {
                                id: rxnToolCell
                                required property var modelData
                                cellWidth: 44
                                cellHeight: Theme.toolCellSize
                                iconSource: "icons/" + rxnToolCell.modelData.icon
                                tip: rxnToolCell.modelData.tip
                                selected: !!(activeCanvas && activeCanvas.currentTool === rxnToolCell.modelData.id)
                                onClicked: {
                                    if (!activeCanvas) return
                                    activeCanvas.currentTool = activeCanvas.currentTool === rxnToolCell.modelData.id ? "SELECT" : rxnToolCell.modelData.id
                                    reactionsMenuTrigger.close()
                                }
                            }
                        }
                    }

                    Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: Theme.outline }

                    Text {
                        text: "Arrow style"
                        color: Theme.textSecondary
                        font { pixelSize: Theme.fontSizeCaption; bold: true; letterSpacing: 1; family: Theme.fontDisplay }
                    }
                    Repeater {
                        model: [
                            { mode: "filled-triangle", label: "Filled Triangle", icon: "reaction-arrow-filled-triangle.svg" },
                            { mode: "open-angle", label: "Open Angle", icon: "reaction-arrow-open-angle.svg" },
                            { mode: "retrosynthetic", label: "Retrosynthetic", icon: "reaction-arrow-retrosynthetic-arrow.svg" },
                            { mode: "equilibrium-FF", label: "Equilibrium (↔)", icon: "reaction-arrow-equilibrium-filled-triangle.svg" },
                            { mode: "equilibrium-FH", label: "Equilibrium (⇌)", icon: "reaction-arrow-equilibrium-filled-half-bow.svg" },
                            { mode: "curved-mechanism", label: "Curved Mechanism", icon: "reaction-arrow-elliptical-arc-arrow-filled-triangle.svg" }
                        ]
                        delegate: Rectangle {
                            id: amRow
                            required property var modelData
                            Layout.fillWidth: true
                            implicitHeight: 24
                            radius: 3
                            color: !!(activeCanvas && activeCanvas.currentArrowMode === amRow.modelData.mode) ? Theme.selected : (amRowMouse.containsMouse ? Theme.hover : "transparent")
                            Behavior on color { ColorAnimation { duration: 150 } }
                            Image {
                                id: amRowIcon
                                anchors { left: parent ? parent.left : undefined; leftMargin: 6; verticalCenter: parent ? parent.verticalCenter : undefined }
                                source: "icons/" + amRow.modelData.icon
                                width: 18
                                height: 18
                                sourceSize: Qt.size(18, 18)
                                onStatusChanged: {
                                    if (status === Image.Error) {
                                        console.warn("Failed to load icon:", amRow.modelData.icon)
                                    }
                                }
                            }
                            Text {
                                anchors { left: amRowIcon.right; leftMargin: 6; verticalCenter: parent ? parent.verticalCenter : undefined }
                                text: amRow.modelData.label
                                color: !!(activeCanvas && activeCanvas.currentArrowMode === amRow.modelData.mode) ? Theme.accent : Theme.textPrimary
                                font { pixelSize: Theme.fontSizeLabel; family: Theme.fontDisplay }
                            }
                            MouseArea {
                                id: amRowMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (activeCanvas) activeCanvas.currentArrowMode = amRow.modelData.mode
                                    reactionsMenuTrigger.close()
                                }
                            }
                        }
                    }

                    Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: Theme.outline }
                    Text {
                        text: "Atom mapping"
                        color: Theme.textSecondary
                        font { pixelSize: Theme.fontSizeCaption; bold: true; letterSpacing: 1; family: Theme.fontDisplay }
                    }
                    MenuItemRow {
                        Layout.fillWidth: true
                        text: "Auto-map Reaction"
                        iconSource: "reaction-map.svg"
                        onTriggered: {
                            window.isProcessing = true
                            if (activeSketch) activeSketch.requestSerialize("automap")
                            reactionsMenuTrigger.close()
                        }
                    }
                    MenuItemRow {
                        Layout.fillWidth: true
                        text: "Clear Mapping"
                        iconSource: "reaction-map.svg"
                        onTriggered: {
                            window.isProcessing = true
                            if (activeSketch) activeSketch.requestSerialize("clear_mapping")
                            reactionsMenuTrigger.close()
                        }
                    }
                    MenuItemRow {
                        Layout.fillWidth: true
                        text: "Correct Reacting Centers"
                        iconSource: "reaction-map.svg"
                        onTriggered: {
                            window.isProcessing = true
                            if (activeSketch) activeSketch.requestSerialize("correct_reacting_centers")
                            reactionsMenuTrigger.close()
                        }
                    }
                }
            }
            AppMenuBarItem {
                id: queryMenuTrigger
                text: "Query"
                ColumnLayout {
                    width: 210
                    spacing: 6

                    Text {
                        text: "Query atoms"
                        color: Theme.textSecondary
                        font { pixelSize: Theme.fontSizeCaption; bold: true; letterSpacing: 1; family: Theme.fontDisplay }
                    }
                    GridLayout {
                        columns: 4
                        columnSpacing: Theme.spacingSmall
                        rowSpacing: Theme.spacingSmall
                        Layout.alignment: Qt.AlignHCenter
                        Repeater {
                            model: [
                                { id: "ATOM_A",  icon: "A",  tip: "Any atom" },
                                { id: "ATOM_AH", icon: "AH", tip: "Any atom including H" },
                                { id: "ATOM_Q",  icon: "Q",  tip: "Any heteroatom" },
                                { id: "ATOM_QH", icon: "QH", tip: "Heteroatom or H" },
                                { id: "ATOM_M",  icon: "M",  tip: "Any metal" },
                                { id: "ATOM_MH", icon: "MH", tip: "Metal or H" },
                                { id: "ATOM_X",  icon: "X",  tip: "Any halogen" },
                                { id: "ATOM_XH", icon: "XH", tip: "Halogen or H" }
                            ]
                            delegate: IconCell {
                                id: qaCell
                                required property var modelData
                                cellWidth: 42
                                cellHeight: 32
                                glyph: qaCell.modelData.icon
                                tip: qaCell.modelData.tip
                                selected: !!(activeCanvas && activeCanvas.currentTool === qaCell.modelData.id)
                                onClicked: {
                                    if (!activeCanvas) return
                                    activeCanvas.currentTool = activeCanvas.currentTool === qaCell.modelData.id ? "SELECT" : qaCell.modelData.id
                                    queryMenuTrigger.close()
                                }
                            }
                        }
                    }

                    Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: Theme.outline }

                    Text {
                        text: "R-group labels"
                        color: Theme.textSecondary
                        font { pixelSize: Theme.fontSizeCaption; bold: true; letterSpacing: 1; family: Theme.fontDisplay }
                    }
                    GridLayout {
                        columns: 4
                        columnSpacing: Theme.spacingSmall
                        rowSpacing: Theme.spacingSmall
                        Layout.alignment: Qt.AlignHCenter
                        Repeater {
                            model: ["R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8"]
                            delegate: IconCell {
                                id: rgCell
                                required property string modelData
                                cellWidth: 42
                                cellHeight: 28
                                glyph: rgCell.modelData
                                hasGlyphColorOverride: true
                                glyphColor: Theme.rgroupColor
                                accentColor: Theme.rgroupColor
                                borderAlwaysVisible: true
                                onClicked: {
                                    if (activeCanvas) activeCanvas.currentTool = "ATOM_" + rgCell.modelData
                                    queryMenuTrigger.close()
                                }
                            }
                        }
                    }
                }
            }
            AppMenuBarItem {
                text: "Copy"
                MenuItemRow { text: "Copy SMILES"; onTriggered: if (activeSketch) activeSketch.requestSerialize("smiles") }
                MenuItemRow { text: "Copy Canonical SMILES"; onTriggered: if (activeSketch) activeSketch.requestSerialize("canonical_smiles") }
                Rectangle { width: parent.width; height: 1; color: Theme.outline; opacity: 0.6 }
                MenuItemRow { text: "Copy InChI"; onTriggered: if (activeSketch) activeSketch.requestSerialize("inchi") }
                MenuItemRow { text: "Copy InChIKey"; onTriggered: if (activeSketch) activeSketch.requestSerialize("inchikey") }
                Rectangle { width: parent.width; height: 1; color: Theme.outline; opacity: 0.6 }
                MenuItemRow { text: "Copy Hash"; onTriggered: if (activeSketch) activeSketch.requestSerialize("hash") }
                MenuItemRow { text: "Copy Mass Composition"; onTriggered: if (activeSketch) activeSketch.requestSerialize("mass_composition") }
                MenuItemRow { text: "Copy pKa Values"; onTriggered: if (activeSketch) activeSketch.requestSerialize("pka_values") }
                Rectangle { width: parent.width; height: 1; color: Theme.outline; opacity: 0.6 }
                MenuItemRow { text: "Compare Similarity…"; onTriggered: similarityDialog.open() }
            }
            AppMenuBarItem {
                text: "View"
                MenuItemRow { text: "Fit to Screen"; iconSource: "fit.svg"; shortcutHint: "Ctrl+0"; onTriggered: activeCanvas.fitToMolecule() }
                Rectangle { width: parent.width; height: 1; color: Theme.outline; opacity: 0.6 }
                MenuItemRow { text: "Zoom In"; iconSource: "zoom-in.svg"; onTriggered: window.zoomLevel = Math.min(3.0, window.zoomLevel + 0.1) }
                MenuItemRow { text: "Zoom Out"; iconSource: "zoom-out.svg"; onTriggered: window.zoomLevel = Math.max(0.1, window.zoomLevel - 0.1) }
                MenuItemRow { text: "Reset (100%)"; onTriggered: window.zoomLevel = 1.0 }
            }
        }

        // Top Toolbar
        Rectangle {
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
                                if (fileToolbarDelegate.modelData.id === "SAVE") window.saveActive(false)
                                else if (fileToolbarDelegate.modelData.id === "OPEN") openDialog.open()
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
                                if (editToolbarDelegate.modelData.id === "UNDO") return !!(activeCanvas && activeCanvas.canUndo)
                                if (editToolbarDelegate.modelData.id === "REDO") return !!(activeCanvas && activeCanvas.canRedo)
                                return true
                            }
                            onClicked: {
                                if (editToolbarDelegate.modelData.id === "UNDO") activeCanvas.undo()
                                else if (editToolbarDelegate.modelData.id === "REDO") activeCanvas.redo()
                                else if (editToolbarDelegate.modelData.id === "COPY") activeCanvas.copySelection()
                                else if (editToolbarDelegate.modelData.id === "CUT") activeCanvas.cutSelection()
                                else if (editToolbarDelegate.modelData.id === "PASTE") activeCanvas.pasteSelection()
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
                            return structOpDelegate.modelData.id === "rgroups" || !window.isProcessing
                        }
                        onClicked: {
                            executeStructureOp(structOpDelegate.modelData.id)
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
                    onActivated: if (activeSketch) activeSketch.setStereoFlags(currentIndex === 0 ? "abs" : "rel", 0)
                    Binding on currentIndex {
                        value: (activeSketch && activeSketch.primitives && activeSketch.primitives.stereoFlags && activeSketch.primitives.stereoFlags.type === "rel") ? 1 : 0
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
                        enabled: Selection.hasAtoms(activeSketch, alignDelegate.modelData.minAtoms)
                        onClicked: {
                            if (!activeSketch) return
                            if (alignDelegate.modelData.fn === "align") activeSketch.alignAtoms(alignDelegate.modelData.mode)
                            else activeSketch.distributeAtoms(alignDelegate.modelData.mode)
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
                        enabled: Selection.hasAtoms(activeSketch, 2)
                        onClicked: if (activeSketch) activeSketch.transformSelection(xformDelegate.modelData.mode)
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

        // Horizontal separator
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Theme.outline
        }

        // Document tabs
        RowLayout {
            Layout.fillWidth: true
            spacing: 4

            TabBar {
                id: docTabBar
                Layout.fillWidth: true
                currentIndex: DocumentManager.docIds.indexOf(DocumentManager.activeDocId)
                onCurrentIndexChanged: {
                    var ids = DocumentManager.docIds
                    if (currentIndex >= 0 && currentIndex < ids.length) DocumentManager.activeDocId = ids[currentIndex]
                }

                Repeater {
                    model: DocumentManager.docIds
                    TabButton {
                        id: tabBtn
                        required property int modelData
                        // titleRev forces re-evaluation after open / save-as / rename
                        text: (window.titleRev, (window.dirtyDocs[modelData] ? "● " : "") + window.titleFor(modelData))

                        onDoubleClicked: {
                            renameDialog.docId = modelData
                            renameDialog.open()
                        }

                        Text {
                            text: "✕"
                            anchors.right: parent ? parent.right : undefined
                            anchors.verticalCenter: parent ? parent.verticalCenter : undefined
                            anchors.rightMargin: 8
                            font.pixelSize: Theme.fontSizeCaption
                            color: tabMouseArea.containsMouse ? Theme.error : Theme.textSecondary
                            MouseArea {
                                id: tabMouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: {
                                    if (DocumentManager.docIds.length <= 1) return
                                    if (window.dirtyDocs[tabBtn.modelData]) {
                                        // Switch to the tab being closed first: saveDialog/the
                                        // structureReady Connections below only ever operate on
                                        // activeSketch/activeCanvas, so saving a *different*,
                                        // still-background tab here would silently save the
                                        // wrong document's content instead.
                                        DocumentManager.activeDocId = tabBtn.modelData
                                        window._pendingCloseDocId = tabBtn.modelData
                                        unsavedChangesDialog.open()
                                    } else {
                                        DocumentManager.closeDocument(tabBtn.modelData)
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Button {
                text: "+"
                onClicked: DocumentManager.addDocument()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            ToolPanel {
                Layout.preferredWidth: Theme.toolPanelWidth
                Layout.fillHeight: true
                sketch: window.activeSketch
                currentTool: activeCanvas ? activeCanvas.currentTool : "SELECT"
                onToolSelected: (toolId) => {
                    if (!activeCanvas) return
                    if (toolId === "ATOM_ANY") {
                        periodicTablePopup.open()
                    } else {
                        if (activeCanvas.currentTool === toolId) {
                            activeCanvas.currentTool = "SELECT"
                        } else {
                            activeCanvas.currentTool = toolId
                        }
                    }
                }
            }

            // Thin separator
            Rectangle {
                Layout.preferredWidth: 1
                Layout.fillHeight: true
                color: Theme.outline
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0

                // Top Ruler
                Canvas {
                    id: topRuler
                    antialiasing: true
                    Layout.fillWidth: true
                    Layout.preferredHeight: 24
                    Layout.leftMargin: 24 // Align with the left ruler width
                    
                    property real scrollX: scrollView.contentItem.contentX
                    property real zoom: window.zoomLevel
                    property real docX: docRect.x
                    property var margins: window.pageMargins
                    property string pageSize: pageSizeCombo.currentText
                    
                    readonly property real cmPixels: window.pixelsPerCm * zoom
                    readonly property real startX: docX - scrollX
                    
                    onScrollXChanged: requestPaint()
                    onZoomChanged: requestPaint()
                    onDocXChanged: requestPaint()
                    onWidthChanged: requestPaint()
                    onMarginsChanged: requestPaint()
                    onPageSizeChanged: requestPaint()
                    
                    onPaint: {
                        const ctx = getContext("2d");
                        ctx.clearRect(0, 0, width, height);
                        ctx.fillStyle = Theme.surface.toString();
                        ctx.fillRect(0, 0, width, height);
                        ctx.strokeStyle = Theme.rulerColor;
                        ctx.fillStyle = Theme.rulerColor;
                        ctx.font = Theme.fontSizeCaption + "px " + Theme.fontMonoCss;
                        ctx.beginPath();
                        
                        for (let i = -50; i < 150; i += 0.1) {
                            const x = startX + (i * cmPixels);
                            if (x < 0) continue;
                            if (x > width) break;
                            
                            if (Math.abs(i - Math.round(i)) < 0.01) {
                                ctx.moveTo(x, 0); ctx.lineTo(x, height);
                                if (i !== 0) ctx.fillText(Math.round(i).toString(), x + 2, 10);
                            } else if (Math.abs(i - Math.round(i*2)/2) < 0.01) {
                                ctx.moveTo(x, height/2); ctx.lineTo(x, height);
                            } else {
                                ctx.moveTo(x, height - 5); ctx.lineTo(x, height);
                            }
                        }
                        ctx.stroke();
                        
                        // Shade left and right margins
                        ctx.save();
                        ctx.globalAlpha = 0.15;
                        ctx.fillStyle = Theme.rulerColor;
                        
                        const pWidthMm = window.pageSizeMm(pageSizeCombo.currentText).w;
                        const marginLeftPx = startX + (window.pageMargins.left * cmPixels);
                        const marginRightPx = startX + ((pWidthMm/10 - window.pageMargins.right) * cmPixels);
                        const pageWidthPx = startX + ((pWidthMm/10) * cmPixels);
                        
                        const leftRectW = Math.max(0, Math.min(marginLeftPx, width));
                        ctx.fillRect(0, 0, leftRectW, height);
                        
                        const rightRectX = Math.max(0, Math.min(marginRightPx, width));
                        const rightRectW = Math.max(0, Math.min(pageWidthPx, width) - rightRectX);
                        ctx.fillRect(rightRectX, 0, rightRectW, height);
                        
                        ctx.restore();
                        
                        ctx.beginPath();
                        ctx.moveTo(0, height);
                        ctx.lineTo(width, height);
                        ctx.stroke();
                    }

                    MouseArea {
                        id: leftMarginDrag
                        width: 6
                        height: parent.height
                        x: parent.startX + (window.pageMargins.left * parent.cmPixels) - 3
                        y: 0
                        cursorShape: Qt.SizeHorCursor
                        
                        property real dragStartX: 0
                        property real marginStartX: 0
                        
                        onPressed: (mouse) => {
                            let parentPos = mapToItem(parent, mouse.x, mouse.y)
                            dragStartX = parentPos.x
                            marginStartX = window.pageMargins.left
                        }
                        
                        onPositionChanged: (mouse) => {
                            if (pressed) {
                                let parentPos = mapToItem(parent, mouse.x, mouse.y)
                                let deltaX = parentPos.x - dragStartX
                                let deltaCm = deltaX / parent.cmPixels
                                let newMargin = marginStartX + deltaCm
                                
                                let pWidthCm = window.pageSizeMm(pageSizeCombo.currentText).w / 10
                                let maxMargin = pWidthCm - window.pageMargins.right - 1.0
                                newMargin = Math.max(0.0, Math.min(newMargin, maxMargin))
                                
                                let margins = {
                                    left: newMargin,
                                    right: window.pageMargins.right,
                                    top: window.pageMargins.top,
                                    bottom: window.pageMargins.bottom
                                }
                                window.pageMargins = margins
                            }
                        }
                    }

                    MouseArea {
                        id: rightMarginDrag
                        width: 6
                        height: parent.height
                        x: {
                            let pWidthCm = window.pageSizeMm(pageSizeCombo.currentText).w / 10
                            return parent.startX + ((pWidthCm - window.pageMargins.right) * parent.cmPixels) - 3
                        }
                        y: 0
                        cursorShape: Qt.SizeHorCursor
                        
                        property real dragStartX: 0
                        property real marginStartX: 0
                        
                        onPressed: (mouse) => {
                            let parentPos = mapToItem(parent, mouse.x, mouse.y)
                            dragStartX = parentPos.x
                            marginStartX = window.pageMargins.right
                        }
                        
                        onPositionChanged: (mouse) => {
                            if (pressed) {
                                let parentPos = mapToItem(parent, mouse.x, mouse.y)
                                let deltaX = parentPos.x - dragStartX
                                let deltaCm = deltaX / parent.cmPixels
                                let newMargin = marginStartX - deltaCm
                                
                                let pWidthCm = window.pageSizeMm(pageSizeCombo.currentText).w / 10
                                let maxMargin = pWidthCm - window.pageMargins.left - 1.0
                                newMargin = Math.max(0.0, Math.min(newMargin, maxMargin))
                                
                                let margins = {
                                    left: window.pageMargins.left,
                                    right: newMargin,
                                    top: window.pageMargins.top,
                                    bottom: window.pageMargins.bottom
                                }
                                window.pageMargins = margins
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 0

                    // Left Ruler
                    Canvas {
                        id: leftRuler
                        antialiasing: true
                        Layout.preferredWidth: 24
                        Layout.fillHeight: true
                        
                        property real scrollY: scrollView.contentItem.contentY
                        property real zoom: window.zoomLevel
                        property real docY: docRect.y
                        property var margins: window.pageMargins
                        property string pageSize: pageSizeCombo.currentText
                        
                        readonly property real cmPixels: window.pixelsPerCm * zoom
                        readonly property real startY: docY - scrollY
                        
                        onScrollYChanged: requestPaint()
                        onZoomChanged: requestPaint()
                        onDocYChanged: requestPaint()
                        onHeightChanged: requestPaint()
                        onMarginsChanged: requestPaint()
                        onPageSizeChanged: requestPaint()
                        
                        onPaint: {
                            const ctx = getContext("2d");
                            ctx.clearRect(0, 0, width, height);
                            ctx.fillStyle = Theme.surface.toString();
                            ctx.fillRect(0, 0, width, height);
                            ctx.strokeStyle = Theme.rulerColor;
                            ctx.fillStyle = Theme.rulerColor;
                            ctx.font = Theme.fontSizeCaption + "px " + Theme.fontMonoCss;
                            ctx.beginPath();
                            
                            for (let i = -50; i < 150; i += 0.1) {
                                const y = startY + (i * cmPixels);
                                if (y < 0) continue;
                                if (y > height) break;
                                
                                if (Math.abs(i - Math.round(i)) < 0.01) {
                                    ctx.moveTo(0, y); ctx.lineTo(width, y);
                                    if (i !== 0) ctx.fillText(Math.round(i).toString(), 2, y + 10);
                                } else if (Math.abs(i - Math.round(i*2)/2) < 0.01) {
                                    ctx.moveTo(width/2, y); ctx.lineTo(width, y);
                                } else {
                                    ctx.moveTo(width - 5, y); ctx.lineTo(width, y);
                                }
                            }
                            ctx.stroke();
                            
                            // Shade top and bottom margins
                            ctx.save();
                            ctx.globalAlpha = 0.15;
                            ctx.fillStyle = Theme.rulerColor;
                            
                            const pHeightMm = window.pageSizeMm(pageSizeCombo.currentText).h;
                            const marginTopPx = startY + (window.pageMargins.top * cmPixels);
                            const marginBottomPx = startY + ((pHeightMm/10 - window.pageMargins.bottom) * cmPixels);
                            const pageHeightPx = startY + ((pHeightMm/10) * cmPixels);
                            
                            const topRectH = Math.max(0, Math.min(marginTopPx, height));
                            ctx.fillRect(0, 0, width, topRectH);
                            
                            const bottomRectY = Math.max(0, Math.min(marginBottomPx, height));
                            const bottomRectH = Math.max(0, Math.min(pageHeightPx, height) - bottomRectY);
                            ctx.fillRect(0, bottomRectY, width, bottomRectH);
                            
                            ctx.restore();
                            
                            ctx.beginPath();
                            ctx.moveTo(width, 0);
                            ctx.lineTo(width, height);
                            ctx.stroke();
                        }

                        MouseArea {
                            id: topMarginDrag
                            width: parent.width
                            height: 6
                            x: 0
                            y: parent.startY + (window.pageMargins.top * parent.cmPixels) - 3
                            cursorShape: Qt.SizeVerCursor
                            
                            property real dragStartY: 0
                            property real marginStartY: 0
                            
                            onPressed: (mouse) => {
                                let parentPos = mapToItem(parent, mouse.x, mouse.y)
                                dragStartY = parentPos.y
                                marginStartY = window.pageMargins.top
                            }
                            
                            onPositionChanged: (mouse) => {
                                if (pressed) {
                                    let parentPos = mapToItem(parent, mouse.x, mouse.y)
                                    let deltaY = parentPos.y - dragStartY
                                    let deltaCm = deltaY / parent.cmPixels
                                    let newMargin = marginStartY + deltaCm
                                    
                                    let pHeightCm = window.pageSizeMm(pageSizeCombo.currentText).h / 10
                                    let maxMargin = pHeightCm - window.pageMargins.bottom - 1.0
                                    newMargin = Math.max(0.0, Math.min(newMargin, maxMargin))
                                    
                                    let margins = {
                                        left: window.pageMargins.left,
                                        right: window.pageMargins.right,
                                        top: newMargin,
                                        bottom: window.pageMargins.bottom
                                    }
                                    window.pageMargins = margins
                                }
                            }
                        }

                        MouseArea {
                            id: bottomMarginDrag
                            width: parent.width
                            height: 6
                            x: 0
                            y: {
                                const pHeightCm = window.pageSizeMm(pageSizeCombo.currentText).h / 10
                                return parent.startY + ((pHeightCm - window.pageMargins.bottom) * parent.cmPixels) - 3
                            }
                            cursorShape: Qt.SizeVerCursor
                            
                            property real dragStartY: 0
                            property real marginStartY: 0
                            
                            onPressed: (mouse) => {
                                let parentPos = mapToItem(parent, mouse.x, mouse.y)
                                dragStartY = parentPos.y
                                marginStartY = window.pageMargins.bottom
                            }
                            
                            onPositionChanged: (mouse) => {
                                if (pressed) {
                                    let parentPos = mapToItem(parent, mouse.x, mouse.y)
                                    let deltaY = parentPos.y - dragStartY
                                    let deltaCm = deltaY / parent.cmPixels
                                    let newMargin = marginStartY - deltaCm
                                    
                                    let pHeightCm = window.pageSizeMm(pageSizeCombo.currentText).h / 10
                                    let maxMargin = pHeightCm - window.pageMargins.top - 1.0
                                    newMargin = Math.max(0.0, Math.min(newMargin, maxMargin))
                                    
                                    let margins = {
                                        left: window.pageMargins.left,
                                        right: window.pageMargins.right,
                                        top: window.pageMargins.top,
                                        bottom: newMargin
                                    }
                                    window.pageMargins = margins
                                }
                            }
                        }
                    }

                    ScrollView {
                        id: scrollView
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        ScrollBar.horizontal.policy: ScrollBar.AlwaysOn
                        ScrollBar.vertical.policy: ScrollBar.AlwaysOn

                        Item {
                            width: Math.max(scrollView.availableWidth, (docRect.width * window.zoomLevel) + 100)
                            height: Math.max(scrollView.availableHeight, (docRect.height * window.zoomLevel) + 100)

                            Rectangle {
                                anchors.fill: parent
                                color: Theme.workspaceBackground
                            }

                            Rectangle {
                                id: docRect
                                width: {
                                    switch(pageSizeCombo.currentText) {
                                        case "A4": return 794;
                                        case "A3": return 1123;
                                        case "A5": return 559;
                                        case "Letter": return 816;
                                        default: return 794;
                                    }
                                }
                                height: {
                                    switch(pageSizeCombo.currentText) {
                                        case "A4": return 1123;
                                        case "A3": return 1587;
                                        case "A5": return 794;
                                        case "Letter": return 1056;
                                        default: return 1123;
                                    }
                                }
                                color: Theme.surface
                                clip: true
                                border {
                                    color: Theme.outline
                                    width: 1
                                }
                                scale: window.zoomLevel
                                transformOrigin: Item.TopLeft
                                
                                x: Math.max(50, (parent.width - (width * window.zoomLevel)) / 2)
                                y: 50

                                Repeater {
                                    id: canvasRepeater
                                    model: DocumentManager.docIds
                                    anchors.fill: parent

                                    ChemCanvas {
                                        id: canvasInstance
                                        required property int modelData
                                        anchors.fill: parent
                                        visible: modelData === DocumentManager.activeDocId
                                        sketch: DocumentManager.documentFor(modelData)

                                        onIsDirtyChanged: {
                                            var d = window.dirtyDocs
                                            d[modelData] = isDirty
                                            window.dirtyDocs = d
                                        }
                                        Component.onCompleted: {
                                            if (modelData === DocumentManager.activeDocId) window.activeCanvas = canvasInstance
                                        }

                                        onAtomPropertiesRequested: (atomId) => {
                                            if (canvasInstance.sketch) canvasInstance.sketch.requestAtomProperties(atomId)
                                        }

                                        onTextEditRequested: (textId, content, chemX, chemY) => {
                                            textDialog.textId = textId
                                            textDialog.chemX = chemX
                                            textDialog.chemY = chemY
                                            textDialog.inputText = content
                                            textDialog.open()
                                        }
                                        onImageInsertRequested: (cx, cy) => {
                                            imageFileDialog.chemX = cx
                                            imageFileDialog.chemY = cy
                                            imageFileDialog.open()
                                        }
                                    }
                                }

                                Connections {
                                    target: DocumentManager
                                    function onActiveDocIdChanged() {
                                        for (var i = 0; i < canvasRepeater.count; i++) {
                                            var item = canvasRepeater.itemAt(i)
                                            if (item && item.modelData === DocumentManager.activeDocId) {
                                                window.activeCanvas = item
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Thin separator
            Rectangle {
                Layout.preferredWidth: 1
                Layout.fillHeight: true
                color: Theme.outline
            }

            PropertyPanel {
                id: propPanel
                Layout.preferredWidth: Theme.propertyPanelWidth
                Layout.fillHeight: true
                canvas: activeCanvas
            }
        }

        // Bottom atom bar: the most-used atoms one click away, Ketcher-style
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: Theme.toolCellSize + 8
            color: Theme.surface

            Rectangle {
                anchors.top: parent.top
                width: parent.width
                height: 1
                color: Theme.outline
            }

            RowLayout {
                anchors.centerIn: parent
                spacing: Theme.spacingSmall

                Repeater {
                    model: [
                        { id: "ATOM_C",  icon: "C",  tip: "Carbon" },
                        { id: "ATOM_N",  icon: "N",  tip: "Nitrogen" },
                        { id: "ATOM_O",  icon: "O",  tip: "Oxygen" },
                        { id: "ATOM_S",  icon: "S",  tip: "Sulfur" },
                        { id: "ATOM_F",  icon: "F",  tip: "Fluorine" },
                        { id: "ATOM_Cl", icon: "Cl", tip: "Chlorine" },
                        { id: "ATOM_ANY", icon: "period-table.svg", tip: "Periodic Table" },
                        { id: "CHARGE_PLUS",  icon: "charge-plus.svg", tip: "Charge Plus" },
                        { id: "CHARGE_MINUS", icon: "charge-minus.svg", tip: "Charge Minus" }
                    ]
                    delegate: IconCell {
                        id: atomBarCell
                        required property var modelData
                        iconSource: atomBarCell.modelData.icon.indexOf(".svg") !== -1 ? "icons/" + atomBarCell.modelData.icon : ""
                        glyph: atomBarCell.modelData.icon.indexOf(".svg") === -1 ? atomBarCell.modelData.icon : ""
                        hasGlyphColorOverride: atomBarCell.modelData.icon.indexOf(".svg") === -1
                        glyphColor: atomBarCell.modelData.icon === "C" ? Theme.textPrimary : Theme.getElementColor(atomBarCell.modelData.icon)
                        tip: atomBarCell.modelData.tip
                        selected: !!(activeCanvas && activeCanvas.currentTool === atomBarCell.modelData.id)
                        onClicked: {
                            if (!activeCanvas) return
                            if (atomBarCell.modelData.id === "ATOM_ANY") {
                                periodicTablePopup.open()
                            } else if (activeCanvas.currentTool === atomBarCell.modelData.id) {
                                activeCanvas.currentTool = "SELECT"
                            } else {
                                activeCanvas.currentTool = atomBarCell.modelData.id
                            }
                        }
                    }
                }
            }
        }

        // Status bar with Zoom Control
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            color: Theme.background
            
            Rectangle {
                anchors.top: parent.top
                width: parent.width
                height: 1
                color: Theme.outline
            }

            RowLayout {
                anchors {
                    left: parent ? parent.left : undefined
                    leftMargin: 16
                    verticalCenter: parent ? parent.verticalCenter : undefined
                }
                spacing: 12

                Text {
                    text: activeCanvas ? "Tool: " + activeCanvas.currentTool : "No document"
                    color: Theme.textSecondary
                    font { pixelSize: Theme.fontSizeBody; family: Theme.fontMono }
                }
                // text: bindings evaluate even while visible is false, so every
                // dereference below stays guarded down to the leaf array.
                Text {
                    visible: !!(activeSketch && activeSketch.primitives && activeSketch.primitives.atoms)
                    text: "Atoms: " + ((activeSketch && activeSketch.primitives && activeSketch.primitives.atoms) ? activeSketch.primitives.atoms.length : 0)
                    color: Theme.textSecondary
                    font { pixelSize: Theme.fontSizeBody; family: Theme.fontMono }
                }
                Text {
                    visible: !!(activeSketch && activeSketch.primitives && activeSketch.primitives.bonds)
                    text: "Bonds: " + ((activeSketch && activeSketch.primitives && activeSketch.primitives.bonds) ? activeSketch.primitives.bonds.length : 0)
                    color: Theme.textSecondary
                    font { pixelSize: Theme.fontSizeBody; family: Theme.fontMono }
                }
                Text {
                    visible: !!(activeSketch && activeSketch.selection && activeSketch.selection.atom_ids)
                    text: "Selected: " + Selection.totalCount(activeSketch)
                    color: Theme.textSecondary
                    font { pixelSize: Theme.fontSizeBody; family: Theme.fontMono }
                }
            }

            RowLayout {
                anchors {
                    right: parent ? parent.right : undefined
                    rightMargin: 16
                    verticalCenter: parent ? parent.verticalCenter : undefined
                }
                spacing: 8

                CheckBox {
                    text: "Show Hydrogens"
                    checked: activeCanvas ? activeCanvas.showExplicitH : false
                    onCheckedChanged: if (activeCanvas) activeCanvas.showExplicitH = checked
                }

                ToolButton {
                    text: Theme.darkMode ? "☀" : "🌙"
                    font.pixelSize: Theme.fontSizeHeadline
                    onClicked: {
                        Theme.darkMode = !Theme.darkMode
                        uiSettings.darkMode = Theme.darkMode
                    }
                    ToolTip.text: Theme.darkMode ? "Switch to light theme" : "Switch to dark theme"
                    ToolTip.visible: hovered
                    ToolTip.delay: 500
                }

                Text {
                    text: Math.round(window.zoomLevel * 100) + "%"
                    color: Theme.textSecondary
                    font {
                        pixelSize: Theme.fontSizeBody
                        bold: true
                        family: Theme.fontMono
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: window.zoomLevel = 1.0
                    }
                }

                ToolButton {
                    icon.source: "icons/zoom-out.svg"
                    icon.width: 16
                    icon.height: 16
                    onClicked: window.zoomLevel = Math.max(0.1, window.zoomLevel - 0.1)
                }

                Slider {
                    id: zoomSlider
                    from: 0.1
                    to: 3.0
                    Layout.preferredWidth: 150
                    onMoved: window.zoomLevel = value
                    Binding on value {
                        value: window.zoomLevel
                        restoreMode: Binding.RestoreBinding
                    }
                }

                ToolButton {
                    icon.source: "icons/zoom-in.svg"
                    icon.width: 16
                    icon.height: 16
                    onClicked: window.zoomLevel = Math.min(3.0, window.zoomLevel + 0.1)
                }
            }
        }
    }

    property url pendingSdfBatchUrl: ""
    property string _pendingBatchAction: "scaffold"
    property var _pendingBatchLabels: []
    property var _pendingBatchGridMolfiles: []
    property string pendingSimilarityRefMolfile: ""

    FileIO {
        id: fileIO
    }

    SdfRecordPicker {
        id: sdfRecordPicker
        onRecordChosen: function(index) {
            if (activeSketch) activeSketch.sendCommand("loadSdfBatchRecord", [index])
        }
        onFindScaffoldRequested: {
            window.isProcessing = true
            _pendingBatchAction = "scaffold"
            if (activeSketch) activeSketch.sendCommand("getSdfBatchMolfiles")
            sdfRecordPicker.close()
        }
        onDecomposeRequested: {
            window.isProcessing = true
            _pendingBatchAction = "decompose"
            if (activeSketch) activeSketch.sendCommand("getSdfBatchMolfiles")
            sdfRecordPicker.close()
        }
        onRankBySimilarityRequested: {
            window.isProcessing = true
            _pendingBatchAction = "similarity"
            window.pendingSimilarityRefMolfile = activeCanvas ? activeCanvas.getMolfile() : ""
            if (activeSketch) activeSketch.sendCommand("getSdfBatchMolfiles")
            sdfRecordPicker.close()
        }
        onAlignToScaffoldRequested: {
            window.isProcessing = true
            _pendingBatchAction = "align"
            if (activeSketch) activeSketch.sendCommand("getSdfBatchMolfiles")
        }
        onExportGridRequested: {
            _pendingBatchAction = "export_grid"
            if (activeSketch) activeSketch.sendCommand("getSdfBatchMolfiles")
            sdfRecordPicker.close()
        }
        onExportBatchFileRequested: {
            _pendingBatchAction = "export_file"
            if (activeSketch) activeSketch.sendCommand("getSdfBatchMolfiles")
        }
    }

    SubstructureSearchPopup {
        id: smartsSearchPopup
        onSearchRequested: function(smarts) {
            if (!activeSketch) return
            window.pendingSmartsQuery = smarts
            activeSketch.requestSerialize("smarts_search")
        }
        onClearRequested: {
            if (activeSketch) activeSketch.sendCommand("selectSubstructureMatches", ["{\"matches\":[]}"])
        }
    }
    property string pendingSmartsQuery: ""
    property string pendingSimilarityRef: ""
    property double pendingIonizePh: 7.4

    FileDialog {
        id: openDialog
        title: "Open Molecule"
        fileMode: FileDialog.OpenFile
        nameFilters: [
            "Molfile (*.mol)", "SDF (*.sdf)", "RDF (*.rdf)", "SMILES (*.smi *.smiles)",
            "CML (*.cml)", "CDX (*.cdx)", "Ketcher JSON (*.ket)",
            "FASTA (*.fasta *.fa)", "HELM (*.helm)", "IDT Oligo (*.idt)",
            "All files (*)"
        ]
        onAccepted: loadFromFile(selectedFile)
    }

    function pageSizeMm(name) {
        switch (name) {
            case "A4": return { w: 210, h: 297 }
            case "A3": return { w: 297, h: 420 }
            case "A5": return { w: 148, h: 210 }
            case "Letter": return { w: 215.9, h: 279.4 }
            default: return { w: 210, h: 297 }
        }
    }

    function printToPdf() {
        if (!activeCanvas) return
        pdfSaveDialog.open()
    }

    function loadFromFile(fileUrl) {
        const fileStrEarly = fileUrl.toString().toLowerCase()
        if (fileStrEarly.endsWith(".rdf")) {
            pendingSdfBatchUrl = fileUrl
            window.isProcessing = true
            indigoSvc.parseRdfBatch(fileUrl)
            return
        } else if (fileStrEarly.endsWith(".smi") || fileStrEarly.endsWith(".smiles")) {
            pendingSdfBatchUrl = fileUrl
            window.isProcessing = true
            indigoSvc.parseIndigoBatchFile(fileUrl, "smiles")
            return
        } else if (fileStrEarly.endsWith(".cml")) {
            pendingSdfBatchUrl = fileUrl
            window.isProcessing = true
            indigoSvc.parseIndigoBatchFile(fileUrl, "cml")
            return
        } else if (fileStrEarly.endsWith(".cdx")) {
            pendingSdfBatchUrl = fileUrl
            window.isProcessing = true
            indigoSvc.parseIndigoBatchFile(fileUrl, "cdx")
            return
        }
        const data = fileIO.read(fileUrl)
        if (data === "") return
        const fileStr = fileUrl.toString().toLowerCase()
        if (fileStr.endsWith(".fasta") || fileStr.endsWith(".fa")) {
            biopolymerDialog.open()
            window.isProcessing = true
            indigoSvc.loadBioFasta(data, "DNA")
        } else if (fileStr.endsWith(".helm")) {
            window.isProcessing = true
            indigoSvc.loadBioHelm(data)
        } else if (fileStr.endsWith(".idt")) {
            window.isProcessing = true
            indigoSvc.loadBioIdt(data)
        } else {
            let fmt = "mol"
            if (fileStr.endsWith(".sdf")) fmt = "sdf"
            else if (fileStr.endsWith(".ket")) fmt = "ket"
            
            if (fmt === "sdf" && activeSketch) {
                pendingSdfBatchUrl = fileUrl
                activeSketch.sendCommand("deserializeSdfBatch", [data])
            } else if (activeCanvas) {
                activeCanvas.loadStructure(fmt, data)
                setDocFile(DocumentManager.activeDocId, fileUrl)
            }
        }
    }

    DropArea {
        anchors.fill: parent
        enabled: true
        onDropped: (drop) => {
            if (drop.hasUrls && drop.urls.length > 0) {
                loadFromFile(drop.urls[0])
                drop.accept()
            }
        }
    }

    FileDialog {
        id: saveDialog
        title: "Save Molecule"
        fileMode: FileDialog.SaveFile
        nameFilters: ["Molfile (*.mol)", "SDF (*.sdf)", "Ketcher JSON (*.ket)", "PNG image (*.png)", "SVG image (*.svg)", "All files (*)"]
        onAccepted: {
            const fileStr = selectedFile.toString().toLowerCase()
            if (fileStr.endsWith(".png")) {
                activeCanvas.exportPNG(selectedFile)
                return
            }
            if (fileStr.endsWith(".svg")) {
                window.pendingRenderUrl = selectedFile
                if (activeSketch) activeSketch.requestStructure("mol", "render_svg")
                return
            }
            let fmt = "mol"
            if (fileStr.endsWith(".sdf")) fmt = "sdf"
            else if (fileStr.endsWith(".ket")) fmt = "ket"
            window.pendingSaveUrl = selectedFile
            window.setDocFile(DocumentManager.activeDocId, selectedFile)
            if (activeSketch) activeSketch.requestStructure(fmt, "save")
            if (activeCanvas) activeCanvas.setClean()
        }
    }

    FileDialog {
        id: pdfSaveDialog
        title: "Export as PDF"
        fileMode: FileDialog.SaveFile
        nameFilters: ["PDF document (*.pdf)"]
        onAccepted: {
            window.pendingRenderUrl = selectedFile
            if (activeSketch) activeSketch.requestStructure("mol", "render_pdf")
        }
    }

    FileDialog {
        id: gridSaveDialog
        title: "Export Reaction Scheme"
        fileMode: FileDialog.SaveFile
        nameFilters: ["PDF document (*.pdf)"]
        onAccepted: {
            window.pendingRenderUrl = selectedFile
            if (activeSketch) activeSketch.requestStructure("mol", "render_grid")
        }
    }

    FileDialog {
        id: batchGridSaveDialog
        title: "Export Batch as Image Grid"
        fileMode: FileDialog.SaveFile
        nameFilters: ["PDF document (*.pdf)", "PNG image (*.png)"]
        onAccepted: {
            window.pendingRenderUrl = selectedFile
            const fileStr = selectedFile.toString().toLowerCase()
            const format = fileStr.endsWith(".png") ? "png" : "pdf"
            indigoSvc.exportBatchGridToFile(window._pendingBatchGridMolfiles, selectedFile, format)
        }
    }

    FileDialog {
        id: batchFileSaveDialog
        title: "Export Batch to File"
        fileMode: FileDialog.SaveFile
        nameFilters: ["SDF (*.sdf)", "RDF (*.rdf)", "SMILES (*.smi)", "CML (*.cml)"]
        onAccepted: {
            const fileStr = selectedFile.toString().toLowerCase()
            let format = "sdf"
            if (fileStr.endsWith(".rdf")) format = "rdf"
            else if (fileStr.endsWith(".smi")) format = "smiles"
            else if (fileStr.endsWith(".cml")) format = "cml"
            indigoSvc.exportBatchToFile(window._pendingBatchGridMolfiles, selectedFile, format)
        }
    }

    MessageDialog {
        id: unsavedChangesDialog
        title: "Unsaved Changes"
        buttons: MessageDialog.Save | MessageDialog.Discard | MessageDialog.Cancel
        text: window._pendingCloseDocId >= 0 ? "This document has unsaved changes. Save before closing?" : "You have unsaved changes. Do you want to save them before exiting?"

        onButtonClicked: function(button) {
            if (button === MessageDialog.Save) {
                saveDialog.open()
            } else if (button === MessageDialog.Discard) {
                if (window._pendingCloseDocId >= 0) {
                    var docId = window._pendingCloseDocId
                    window._pendingCloseDocId = -1
                    DocumentManager.closeDocument(docId)
                } else {
                    window._forceQuit = true
                    Qt.quit()
                }
            }
            // Cancel: reset pending state
            if (button === MessageDialog.Cancel) {
                window._pendingCloseDocId = -1
            }
        }
    }

    PeriodicTablePopup {
        id: periodicTablePopup
        onElementSelected: (label) => {
            activeCanvas.currentTool = "ATOM_" + label
        }
    }

    Shortcut {
        sequence: "Ctrl+I"
        onActivated: smilesDialog.open()
    }

    Shortcut {
        sequence: "Ctrl+B"
        onActivated: biopolymerDialog.open()
    }

    Shortcut {
        sequence: "Ctrl+R"
        onActivated: rgroupPanel.open()
    }

    Shortcut {
        sequence: "Ctrl+N"
        onActivated: DocumentManager.addDocument()
    }

    Shortcut {
        sequence: "Ctrl+S"
        onActivated: window.saveActive(false)
    }

    Shortcut {
        sequence: "Ctrl+Shift+S"
        onActivated: window.saveActive(true)  // Save As: always prompt
    }

    Shortcut {
        sequence: "Ctrl+O"
        onActivated: openDialog.open()
    }

    Shortcut {
        sequence: "Ctrl+Shift+Z"
        onActivated: if (activeCanvas) activeCanvas.redo()
    }

    Shortcut {
        sequence: "Ctrl+W"
        onActivated: {
            if (DocumentManager.docIds.length > 1) {
                if (window.dirtyDocs[DocumentManager.activeDocId]) {
                    window._pendingCloseDocId = DocumentManager.activeDocId
                    unsavedChangesDialog.open()
                } else {
                    DocumentManager.closeDocument(DocumentManager.activeDocId)
                }
            }
        }
    }

    RGroupPanel {
        id: rgroupPanel
        sketch: window.activeSketch
        canvas: window.activeCanvas
    }

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
            renameInput.text = window.titleFor(renameDialog.docId)
            renameInput.selectAll()
            renameInput.forceActiveFocus()
        }
        onAccepted: {
            const name = renameInput.text.trim()
            if (name.length > 0 && docId >= 0) {
                window.docTitles[docId] = name
                window.titleRev++
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
                    if (activeSketch) activeSketch.deleteText(textDialog.textId)
                    textDialog.close()
                }
            }
        }

        onOpened: {
            textDialogInput.text = inputText
            textDialogInput.forceActiveFocus()
        }
        onAccepted: {
            if (!activeSketch) return
            const content = textDialogInput.text.trim()
            if (textId >= 0) {
                if (content.length > 0) activeSketch.updateText(textId, content)
                else activeSketch.deleteText(textId)
            } else if (content.length > 0) {
                activeSketch.addText(content, chemX, chemY)
            }
        }
    }

    FileDialog {
        id: imageFileDialog
        nameFilters: ["Images (*.png *.jpg *.jpeg *.gif *.bmp)"]

        property real chemX: 0
        property real chemY: 0
        property url pendingFileUrl
        // Tunable confidence threshold on Imago's reported recognition-warning count.
        // Verified empirically: a real chemical-structure image reported 0 warnings, an
        // unrelated screenshot reported 34 while still producing a (garbage) molfile — Imago
        // never refuses outright, so this count is the real signal to gate on. Not pinned at
        // exactly 0 since an imperfect scan may legitimately produce a few warnings and still
        // recognize correctly; revisit this number against real user images.
        property int maxAcceptableWarnings: 5

        onAccepted: {
            if (!activeSketch) return
            pendingFileUrl = selectedFile
            window.isProcessing = true
            imagoSvc.recognizeImage(selectedFile)
        }
    }

    BiopolymerDialog {
        id: biopolymerDialog

        onLoadRequested: function(format, seqType, text) {
            window.isProcessing = true
            if      (format === "Sequence") indigoSvc.loadBioSequence(text, seqType)
            else if (format === "FASTA")    indigoSvc.loadBioFasta(text, seqType)
            else if (format === "HELM")     indigoSvc.loadBioHelm(text)
            else if (format === "IDT")      indigoSvc.loadBioIdt(text)
            else if (format === "AxoLabs")  indigoSvc.loadBioAxoLabs(text)
        }

        onExportRequested: function(format) {
            if      (format === "Sequence") activeSketch.requestSerialize("bio_seq")
            else if (format === "FASTA")    activeSketch.requestSerialize("bio_fasta")
            else if (format === "HELM")     activeSketch.requestSerialize("bio_helm")
            else if (format === "IDT")      activeSketch.requestSerialize("bio_idt")
            else if (format === "AxoLabs")  activeSketch.requestSerialize("bio_axolabs")
        }
    }

    AtomPropertiesDialog {
        id: atomPropsDialog
        onPropertiesChanged: function(atomId, isotope, radical, valence) {
            if (activeCanvas) activeCanvas.applyPropertyChange("atomProps", atomId, { isotope: isotope, radical: radical, valence: valence })
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
                window.isProcessing = true
                indigoSvc.layout(smi)
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
            if (ref && activeSketch) {
                window.pendingSimilarityRef = ref
                activeSketch.requestSerialize("similarity")
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
            if (!isNaN(pH) && activeSketch) {
                window.isProcessing = true
                window.pendingIonizePh = pH
                activeSketch.requestSerialize("ionize")
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
                window.isProcessing = true
                indigoSvc.layout(txt)
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
