import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore
import "js/Selection.js" as Selection
import "js/ToolLabels.js" as ToolLabels

ApplicationWindow {
    id: window
    property alias fileDialogsGroup: fileDialogsGroup
    property alias taskDialogsGroup: taskDialogsGroup
    property alias messageDialogsGroup: messageDialogsGroup

    flags: Qt.Window | Qt.FramelessWindowHint

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
    // Declared on window (root), not a nested Item, and called everywhere as
    // window.executeStructureOp(...) — the Structure menu's MenuItemRow children get
    // reparented into AppMenuBarItem's internal Popup (default property alias), and the
    // Qt Quick Compiler (qmlsc AOT) cannot resolve a bare, unqualified function name
    // declared on an un-id'd ancestor Item across that reparenting boundary (confirmed:
    // every OTHER menu in this same reparenting setup already calls its handlers via an
    // id-qualified path - window.saveActive(...)/window.printToPdf() in the File menu,
    // activeCanvas.xxx() in Edit - and those all work; this was the one unqualified
    // exception, and every Structure-menu item threw "ReferenceError: executeStructureOp
    // is not defined" at runtime as a result).
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
            fileDialogsGroup.saveDialog.open()
        }
    }

    // Dialog ids below are declared deep in this file's own scope, which is
    // NOT automatically visible outside it (QML id-scoping is document-local —
    // it does not turn an id into a property of the containing root object).
    // AppMenus.qml holds a `win` reference to this window and reaches these
    // dialogs via e.g. `win.openDialog.open()`, so each one needed here MUST
    // be re-exposed as an alias, or that reference silently fails at runtime
    // (same failure class as the earlier executeStructureOp scoping bug).
    property alias openDialog: fileDialogsGroup.openDialog
    property alias gridSaveDialog: fileDialogsGroup.gridSaveDialog
    property alias biopolymerDialog: biopolymerDialog
    property alias smilesDialog: taskDialogsGroup.smilesDialog
    property alias similarityDialog: taskDialogsGroup.similarityDialog
    property alias ionizeDialog: taskDialogsGroup.ionizeDialog
    property alias inchiLoadDialog: taskDialogsGroup.inchiLoadDialog
    // Same reasoning: dialogs/FileDialogs.qml's batch export handlers reach
    // these two backend services via win.indigoSvc/win.imagoSvc — both need
    // the same alias treatment as the dialog ids above.
    property alias indigoSvc: indigoSvc
    property alias imagoSvc: imagoSvc

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
            messageDialogsGroup.unsavedChangesDialog.open()
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
                if (!error && molfile && warningsCount <= fileDialogsGroup.imageFileDialog.maxAcceptableWarnings && activeSketch) {
                    activeSketch.sendCommand("insertRecognizedStructure", [molfile, fileDialogsGroup.imageFileDialog.chemX, fileDialogsGroup.imageFileDialog.chemY])
                } else {
                    // Recognition failed or low-confidence — fall back to a plain image embed.
                    const dataUri = fileIO.readImageAsDataUri(fileDialogsGroup.imageFileDialog.pendingFileUrl)
                    if (dataUri === "") {
                        messageDialogsGroup.workerErrorDialog.errorText = "Failed to load image. Ensure it is a supported format (png, jpg, gif, bmp) and under 5 MB."
                        messageDialogsGroup.workerErrorDialog.open()
                    } else if (activeSketch) {
                        activeSketch.addImage(dataUri, fileDialogsGroup.imageFileDialog.chemX, fileDialogsGroup.imageFileDialog.chemY, 1.5, 1.5)
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
                    messageDialogsGroup.workerErrorDialog.errorText = "Find Common Scaffold: " + (error || "unknown error")
                    messageDialogsGroup.workerErrorDialog.open()
                }
            }
            function onRgroupDecompositionFinished(result, error) {
                window.isProcessing = false
                if (result && activeCanvas) {
                    activeCanvas.loadMolfile(result)
                } else {
                    messageDialogsGroup.workerErrorDialog.errorText = "Decompose to R-Groups: " + (error || "unknown error")
                    messageDialogsGroup.workerErrorDialog.open()
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
                        messageDialogsGroup.similarityRankResultDialog.text = lines.join("\n")
                        messageDialogsGroup.similarityRankResultDialog.open()
                    } catch (e) {
                        messageDialogsGroup.workerErrorDialog.errorText = "Rank by Similarity: failed to parse results."
                        messageDialogsGroup.workerErrorDialog.open()
                    }
                } else {
                    messageDialogsGroup.workerErrorDialog.errorText = "Rank by Similarity: " + (error || "unknown error")
                    messageDialogsGroup.workerErrorDialog.open()
                }
            }
            function onBatchAlignFinished(resultJson, error) {
                window.isProcessing = false
                if (resultJson && activeSketch) {
                    activeSketch.sendCommand("realignSdfBatch", [resultJson])
                } else {
                    messageDialogsGroup.workerErrorDialog.errorText = "Align Batch to Common Scaffold: " + (error || "unknown error")
                    messageDialogsGroup.workerErrorDialog.open()
                }
            }
            function onRdfBatchParsed(recordsJson, error) {
                if (recordsJson && activeSketch) {
                    activeSketch.sendCommand("deserializeRdfBatch", [recordsJson])
                } else {
                    window.isProcessing = false
                    messageDialogsGroup.workerErrorDialog.errorText = "Open RDF: " + (error || "unknown error")
                    messageDialogsGroup.workerErrorDialog.open()
                }
            }
            function onIndigoBatchParsed(recordsJson, error) {
                if (recordsJson && activeSketch) {
                    activeSketch.sendCommand("deserializeIndigoBatch", [recordsJson])
                } else {
                    window.isProcessing = false
                    messageDialogsGroup.workerErrorDialog.errorText = "Open batch file: " + (error || "unknown error")
                    messageDialogsGroup.workerErrorDialog.open()
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
                    messageDialogsGroup.similarityResultDialog.text = "Tanimoto similarity: " + result
                    messageDialogsGroup.similarityResultDialog.open()
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
                    messageDialogsGroup.workerErrorDialog.errorText = (isPdf ? "PDF export failed: " : "SVG export failed: ") + error
                    messageDialogsGroup.workerErrorDialog.open()
                }
            }
            function onReactionMappingFinished(result, error) {
                window.isProcessing = false
                if (result && activeCanvas) {
                    activeCanvas.loadMolfile(result)
                } else {
                    messageDialogsGroup.workerErrorDialog.errorText = "Atom Mapping: " + (error || "unknown error")
                    messageDialogsGroup.workerErrorDialog.open()
                }
            }
            function onIonizeFinished(result, error) {
                window.isProcessing = false
                if (result && activeCanvas) {
                    activeCanvas.loadMolfile(result)
                } else {
                    messageDialogsGroup.workerErrorDialog.errorText = "Ionize at pH: " + (error || "unknown error")
                    messageDialogsGroup.workerErrorDialog.open()
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
                    taskDialogsGroup.checkResultDialog.reportText = report
                    taskDialogsGroup.checkResultDialog.open()
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
            readonly property var structureResponseRoutes: ({
                save: (data) => fileIO.write(window.pendingSaveUrl, data),
                render_svg: (data) => indigoSvc.renderToFile(data, window.pendingRenderUrl, "svg"),
                render_pdf: (data) => indigoSvc.renderToFile(data, window.pendingRenderUrl, "pdf"),
                render_grid: (data) => indigoSvc.renderReactionGridToFile(data, window.pendingRenderUrl, "pdf"),
                automap: (data) => indigoSvc.autoMapReaction(data),
                ionize: (data) => indigoSvc.ionizeAtPh(data, window.pendingIonizePh),
                clear_mapping: (data) => indigoSvc.clearReactionMapping(data),
                correct_reacting_centers: (data) => indigoSvc.correctReactingCenters(data),
                smiles: (data) => indigoSvc.smiles(data),
                canonical_smiles: (data) => indigoSvc.canonicalSmiles(data),
                inchi: (data) => indigoSvc.inchi(data),
                inchikey: (data) => indigoSvc.inchiKey(data),
                hash: (data) => indigoSvc.hash(data),
                similarity: (data) => indigoSvc.similarity(data, window.pendingSimilarityRef),
                mass_composition: (data) => indigoSvc.massComposition(data),
                pka_values: (data) => indigoSvc.pkaValues(data),
                layout: (data) => { window.isProcessing = true; indigoSvc.layout(data); },
                clean2d: (data) => { window.isProcessing = true; indigoSvc.clean2d(data); },
                aromatize: (data) => { window.isProcessing = true; indigoSvc.aromatize(data); },
                dearomatize: (data) => { window.isProcessing = true; indigoSvc.dearomatize(data); },
                unfoldH: (data) => { window.isProcessing = true; indigoSvc.unfoldHydrogens(data); },
                foldH: (data) => { window.isProcessing = true; indigoSvc.foldHydrogens(data); },
                normalize: (data) => { window.isProcessing = true; indigoSvc.normalize(data); },
                standardize: (data) => { window.isProcessing = true; indigoSvc.standardize(data); },
                check: (data) => indigoSvc.checkStructure(data),
                calc_props: (data) => {
                    indigoSvc.calcProperties(data);
                    indigoSvc.calcStereoDescriptors(data);
                    indigoSvc.checkStructure(data);
                    if (activeSketch) {
                        activeSketch.sendCommand("getMoleculeName", []);
                        activeSketch.sendCommand("getSdfProps", []);
                    }
                },
                bio_seq: (data) => indigoSvc.exportBioSequence(data),
                bio_fasta: (data) => indigoSvc.exportBioFasta(data),
                bio_helm: (data) => indigoSvc.exportBioHelm(data),
                bio_idt: (data) => indigoSvc.exportBioIdt(data),
                bio_axolabs: (data) => indigoSvc.exportBioAxoLabs(data),
                atom_props: (data) => {
                    const props = JSON.parse(data);
                    if (props && props.id !== undefined)
                        atomPropsDialog.openForAtom(props.id, props);
                },
                clipboard_ket: (data) => {
                    if (data && data.length > 0 && activeSketch)
                        activeSketch.setOsClipboardText(data);
                },
                sdf_batch_list: (data) => {
                    window.isProcessing = false;
                    const parsed = JSON.parse(data);
                    if (parsed.count <= 1) {
                        if (activeSketch) activeSketch.sendCommand("loadSdfBatchRecord", [0]);
                    } else {
                        sdfRecordPicker.recordCount = parsed.count;
                        sdfRecordPicker.model = parsed.records;
                        sdfRecordPicker.open();
                    }
                },
                sdf_batch_realigned: (data) => {
                    const parsed = JSON.parse(data);
                    sdfRecordPicker.recordCount = parsed.count;
                    sdfRecordPicker.model = parsed.records;
                },
                sdf_batch_load: (data) => {
                    window.isProcessing = false;
                    if (activeCanvas) {
                        activeCanvas._needsCentering = true;
                        activeCanvas.refresh();
                        setDocFile(DocumentManager.activeDocId, pendingSdfBatchUrl);
                    }
                },
                sdf_batch_molfiles: (data) => {
                    const parsed = JSON.parse(data);
                    if (parsed.molfiles && parsed.molfiles.length >= 2) {
                        if (window._pendingBatchAction === "export_grid") {
                            window._pendingBatchGridMolfiles = parsed.molfiles;
                            fileDialogsGroup.batchGridSaveDialog.open();
                        } else if (window._pendingBatchAction === "export_file") {
                            window._pendingBatchGridMolfiles = parsed.molfiles;
                            fileDialogsGroup.batchFileSaveDialog.open();
                        } else if (window._pendingBatchAction === "align") {
                            indigoSvc.alignBatchToScaffold(parsed.molfiles);
                        } else if (window._pendingBatchAction === "decompose") {
                            indigoSvc.decomposeToRGroups(parsed.molfiles);
                        } else if (window._pendingBatchAction === "similarity") {
                            window._pendingBatchLabels = parsed.labels || [];
                            if (!window.pendingSimilarityRefMolfile) {
                                window.isProcessing = false;
                                messageDialogsGroup.workerErrorDialog.errorText = "Rank by Similarity: no active structure to compare against.";
                                messageDialogsGroup.workerErrorDialog.open();
                            } else {
                                indigoSvc.rankBySimilarity(window.pendingSimilarityRefMolfile, parsed.molfiles);
                            }
                        } else {
                            indigoSvc.findCommonScaffold(parsed.molfiles);
                        }
                    } else {
                        window.isProcessing = false;
                        messageDialogsGroup.workerErrorDialog.errorText = "Batch structure analysis: not enough valid structures in this batch.";
                        messageDialogsGroup.workerErrorDialog.open();
                    }
                },
                smarts_search: (data) => {
                    indigoSvc.substructureSearch(data, window.pendingSmartsQuery);
                },
                mol_name: (data) => {
                    propPanel.molName = data;
                },
                sdf_props: (data) => {
                    propPanel.sdfProps = JSON.parse(data);
                }
            })

            function onStructureReady(reqId, data) {
                const handler = structureResponseRoutes[reqId];
                if (handler) handler(data);
            }
            function onStateUpdated(state, selection, dirty, undoState, redoState, result) {
                if (result !== "stereoUpdated" && result !== "checkUpdated")
                    propUpdateTimer.restart()
            }
            function onErrorOccurred(error) {
                window.isProcessing = false
                messageDialogsGroup.workerErrorDialog.errorText = error
                messageDialogsGroup.workerErrorDialog.open()
            }
        }

        AppMenus {
            win: window
            Layout.fillWidth: true
        }

        // Top Toolbar
        MainToolbar {
            id: mainToolbar
            win: window
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
                            taskDialogsGroup.renameDialog.docId = modelData
                            taskDialogsGroup.renameDialog.open()
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
                                        // Switch to the tab being closed first: fileDialogsGroup.saveDialog/the
                                        // structureReady Connections below only ever operate on
                                        // activeSketch/activeCanvas, so saving a *different*,
                                        // still-background tab here would silently save the
                                        // wrong document's content instead.
                                        DocumentManager.activeDocId = tabBtn.modelData
                                        window._pendingCloseDocId = tabBtn.modelData
                                        messageDialogsGroup.unsavedChangesDialog.open()
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

                    // Ruler units label, drawn in the empty corner this leftMargin
                    // leaves above leftRuler -- topRuler doesn't clip its children,
                    // so a negative x here still renders inside that gap.
                    Text {
                        text: "cm"
                        x: -20
                        y: 6
                        color: Theme.rulerColor
                        font { pixelSize: Theme.fontSizeCaption; family: Theme.fontMono }
                    }

                    property real scrollX: scrollView.contentItem.contentX
                    property real zoom: window.zoomLevel
                    property real docX: docRect.x
                    property var margins: window.pageMargins
                    property string pageSize: mainToolbar.pageSizeCombo.currentText
                    
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
                        
                        const pWidthMm = window.pageSizeMm(mainToolbar.pageSizeCombo.currentText).w;
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
                                
                                let pWidthCm = window.pageSizeMm(mainToolbar.pageSizeCombo.currentText).w / 10
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
                            let pWidthCm = window.pageSizeMm(mainToolbar.pageSizeCombo.currentText).w / 10
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
                                
                                let pWidthCm = window.pageSizeMm(mainToolbar.pageSizeCombo.currentText).w / 10
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
                        property string pageSize: mainToolbar.pageSizeCombo.currentText
                        
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
                            
                            const pHeightMm = window.pageSizeMm(mainToolbar.pageSizeCombo.currentText).h;
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
                                    
                                    let pHeightCm = window.pageSizeMm(mainToolbar.pageSizeCombo.currentText).h / 10
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
                                const pHeightCm = window.pageSizeMm(mainToolbar.pageSizeCombo.currentText).h / 10
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
                                    
                                    let pHeightCm = window.pageSizeMm(mainToolbar.pageSizeCombo.currentText).h / 10
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
                                    switch(mainToolbar.pageSizeCombo.currentText) {
                                        case "A4": return 794;
                                        case "A3": return 1123;
                                        case "A5": return 559;
                                        case "Letter": return 816;
                                        default: return 794;
                                    }
                                }
                                height: {
                                    switch(mainToolbar.pageSizeCombo.currentText) {
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
                                            taskDialogsGroup.textDialog.textId = textId
                                            taskDialogsGroup.textDialog.chemX = chemX
                                            taskDialogsGroup.textDialog.chemY = chemY
                                            taskDialogsGroup.textDialog.inputText = content
                                            taskDialogsGroup.textDialog.open()
                                        }
                                        onImageInsertRequested: (cx, cy) => {
                                            fileDialogsGroup.imageFileDialog.chemX = cx
                                            fileDialogsGroup.imageFileDialog.chemY = cy
                                            fileDialogsGroup.imageFileDialog.open()
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
                    text: activeCanvas ? "Tool: " + ToolLabels.label(activeCanvas.currentTool) : "No document"
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
        fileDialogsGroup.pdfSaveDialog.open()
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

    PeriodicTablePopup {
        id: periodicTablePopup
        onElementSelected: (label) => {
            activeCanvas.currentTool = "ATOM_" + label
        }
    }

    Shortcut {
        sequence: "Ctrl+I"
        onActivated: taskDialogsGroup.smilesDialog.open()
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
        onActivated: fileDialogsGroup.openDialog.open()
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
                    messageDialogsGroup.unsavedChangesDialog.open()
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

    FileDialogs {
        id: fileDialogsGroup
        win: window
    }
    TaskDialogs {
        id: taskDialogsGroup
        win: window
    }
    MessageDialogs {
        id: messageDialogsGroup
        win: window
    }

}
