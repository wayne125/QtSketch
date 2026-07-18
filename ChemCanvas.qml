import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes


Item {
    id: root
    anchors.fill: parent

    property real scale: 1.0
    property real offsetX: 0
    property real offsetY: 0
    property real bondLength: Theme.baseBondLength
    // Single source of truth for the canvas<->chem scale factor (mirrors chem-core.js's
    // Scale.canvasToModel/modelToCanvas, which isn't reachable here since chem-core.js only
    // runs inside the Node.js worker process, not the QML engine).
    readonly property real chemScale: scale * bondLength

    // Page/canvas boundary, in chemical-coordinate space. Must match
    // src/v8_worker.js's PAGE_MIN_X/MAX_X/MIN_Y/MAX_Y (kept in sync manually) --
    // this is only the visual outline; the actual hard clamp lives in the
    // worker functions that place/move/rotate content.
    readonly property rect pageBounds: Qt.rect(-30, -21, 60, 42)

    // Bound by MainWindow's per-document Repeater to this canvas's V8Process instance.
    property var sketch: null
    property string currentTool: "SELECT" // "SELECT", "BOND", "ATOM", "BENZENE"
    property string currentArrowMode: "filled-triangle" // default reaction arrow mode
    property bool showExplicitH: false
    // These must remain 'var' as they store plain JS objects from CanvasState.
    // In a future refactor, these should be moved to a C++ backed type or QtObject subclasses
    // to enable granular property change notifications.
    property var selectedAtom: null
    property var selectedBond: null
    property var selectedRxnArrow: null
    property bool isDirty: false
    property bool canUndo: false
    property bool canRedo: false

    property int _renderVersion: 0
    property int _overlayVersion: 0
    property bool _refreshing: false
    // Armed by loadStructure() only: fit-to-view should follow loading a file,
    // never the user's first interactive draw on a blank canvas.
    property bool _needsCentering: false
    property bool _panningActive: false

    focus: true

    signal atomPropertiesRequested(int atomId)
    // Emitted by the TEXT tool: textId is -1 for "create new at (chemX, chemY)"
    signal textEditRequested(int textId, string content, real chemX, real chemY)
    signal imageInsertRequested(real cx, real cy)

    // Finds a text annotation near a canvas point (px tolerance), or null.
    function hitTestText(cx, cy) {
        if (!sketch || !sketch.primitives || !sketch.primitives.texts) return null
        for (let i = 0; i < sketch.primitives.texts.length; i++) {
            const t = sketch.primitives.texts[i]
            const p = chemToCanvas(t.x, t.y)
            if (Math.abs(p.x - cx) < 60 && Math.abs(p.y - cy) < 20) return t
        }
        return null
    }

    onShowExplicitHChanged: {
        sketch.sendCommand("setShowExplicitH", [showExplicitH])
        refresh()
    }

    // Render data stored in pure JS file (CanvasState.js) so V4 does NOT build
    // InternalClass chains when the top-level component's properties change.

    // Incrementing this triggers all Canvas layers to repaint.

    function setOverlayState(s) {
        sketch.overlayState = s;
        _overlayVersion++;
    }

    // Public accessors for layers and mouse handlers
    
    
    

    
    

        

    

    

    

    

    Keys.onPressed: (event) => {
        if (event.modifiers & Qt.ControlModifier) {
            if (event.key === Qt.Key_Z) {
                sketch.undo()
                refresh()
                event.accepted = true
            }
            if (event.key === Qt.Key_Y) {
                sketch.redo()
                refresh()
                event.accepted = true
            }
            if (event.key === Qt.Key_C) {
                if (event.modifiers & Qt.ShiftModifier) {
                    copyAsImage()
                } else {
                    sketch.copySelection()
                    sketch.requestClipboardKet()  // async → clipboard_ket handler writes to OS clipboard
                }
                event.accepted = true
            }
            if (event.key === Qt.Key_X) {
                sketch.cutSelection()
                sketch.requestClipboardKet()
                refresh()
                event.accepted = true
            }
            if (event.key === Qt.Key_V) {
                const center = canvasToChem(root.width / 2, root.height / 2)
                const osText = sketch.getOsClipboardText()
                if (osText && osText.length > 0) {
                    // Prefer OS clipboard: try KET, then MOL/SDF
                    if (osText.indexOf('"root"') >= 0 || osText.indexOf('"atoms"') >= 0) {
                        sketch.importKetAtPosition(osText, center.x, center.y)
                    } else {
                        // Not KET — fall back to internal clipboard
                        sketch.pasteSelection(center.x, center.y)
                    }
                } else {
                    sketch.pasteSelection(center.x, center.y)
                }
                refresh()
                event.accepted = true
            }
        }
        if (event.key === Qt.Key_A && (event.modifiers & Qt.ControlModifier)) {
            sketch.selectAll()
            event.accepted = true
        }
        if (event.key === Qt.Key_0 && (event.modifiers & Qt.ControlModifier)) {
            fitToMolecule()
            event.accepted = true
        }
        if (event.key === Qt.Key_Delete || event.key === Qt.Key_Backspace) {
            sketch.deleteSelection()
            refresh()
            event.accepted = true
        }
        if (event.key === Qt.Key_Escape) {
            currentTool = "SELECT"
            sketch.setOverlayState({ hoverAtomId: null, hoverBondId: null, dragRect: null, bondPreview: null })
            event.accepted = true
        }
        // Single-key tool shortcuts (no modifier)
        if (event.modifiers === Qt.NoModifier) {
            if (event.key === Qt.Key_S) { currentTool = "SELECT"; event.accepted = true }
            else if (event.key === Qt.Key_B) { currentTool = "BOND_1"; event.accepted = true }
            else if (event.key === Qt.Key_E) { currentTool = "ERASE"; event.accepted = true }
            else if (event.key === Qt.Key_R) { currentTool = "TEMPLATE_BENZENE"; event.accepted = true }
            else if (event.key === Qt.Key_H) { currentTool = "HAND"; event.accepted = true }
        }
        if (event.key === Qt.Key_Left || event.key === Qt.Key_Right ||
                event.key === Qt.Key_Up || event.key === Qt.Key_Down) {
            const sel = sketch.selection
            if ((sel.atom_ids && sel.atom_ids.length > 0) ||
                    (sel.rxnArrow_ids && sel.rxnArrow_ids.length > 0) ||
                    (sel.rxnPlus_ids && sel.rxnPlus_ids.length > 0) ||
                    (sel.multitailArrow_ids && sel.multitailArrow_ids.length > 0)) {
                const nudgePx = (event.modifiers & Qt.ShiftModifier) ? 1 : 10
                const f = root.chemScale
                let cdx = 0, cdy = 0
                if (event.key === Qt.Key_Left)  cdx = -nudgePx / f
                if (event.key === Qt.Key_Right) cdx =  nudgePx / f
                if (event.key === Qt.Key_Up)    cdy =  nudgePx / f
                if (event.key === Qt.Key_Down)  cdy = -nudgePx / f
                sketch.moveSelection(cdx, cdy)
                sketch.commitMove()
                refresh()
                event.accepted = true
            }
        }
    }

    Connections {
        target: sketch
        function onStateUpdated(state, selection, dirty, undoState, redoState, result) {
            if (sketch.selection.atom_ids && sketch.selection.atom_ids.length === 1 && (!sketch.selection.bond_ids || sketch.selection.bond_ids.length === 0)) {
                const atom = sketch.primitives.atomsById[sketch.selection.atom_ids[0].toString()]
                if (atom) {
                    selectedAtom = { id: sketch.selection.atom_ids[0], label: atom.label, charge: atom.charge, isSgroup: atom.isSgroup || false,
                        isAtomList: atom.isAtomList || false, atomListElements: atom.atomListElements || "", atomListNot: atom.atomListNot || false }
                } else {
                    selectedAtom = null
                }
                selectedBond = null
            } else if (sketch.selection.bond_ids && sketch.selection.bond_ids.length === 1 && (!sketch.selection.atom_ids || sketch.selection.atom_ids.length === 0)) {
                const bId = sketch.selection.bond_ids[0]
                let bObj = null
                for (let i=0; i<sketch.primitives.bonds.length; i++) { if(sketch.primitives.bonds[i].id === bId) bObj = sketch.primitives.bonds[i] }
                selectedAtom = null
                selectedBond = { id: bId, type: bObj ? bObj.type : 1 }
                selectedRxnArrow = null
            } else if (sketch.selection.rxnArrow_ids && sketch.selection.rxnArrow_ids.length === 1
                       && (!sketch.selection.atom_ids || sketch.selection.atom_ids.length === 0)
                       && (!sketch.selection.bond_ids || sketch.selection.bond_ids.length === 0)) {
                const arId = sketch.selection.rxnArrow_ids[0]
                let arObj = null
                if (sketch.primitives.rxnArrows) {
                    for (let i = 0; i < sketch.primitives.rxnArrows.length; i++) {
                        if (sketch.primitives.rxnArrows[i].id === arId) arObj = sketch.primitives.rxnArrows[i]
                    }
                }
                selectedAtom = null
                selectedBond = null
                selectedRxnArrow = arObj ? {
                    id: arId,
                    mode: arObj.mode || "filled-triangle",
                    conditionsAbove: (arObj.conditionsText && arObj.conditionsText.above) || "",
                    conditionsBelow: (arObj.conditionsText && arObj.conditionsText.below) || ""
                } : null
            } else {
                selectedAtom = null
                selectedBond = null
                selectedRxnArrow = null
            }
            
            isDirty = dirty
            canUndo = undoState
            canRedo = redoState
            
            _renderVersion++

            if (_needsCentering && sketch.primitives && sketch.primitives.bbox) {
                _needsCentering = false
                centerOnMolecule()
            }
        }
    }

    function refresh() {
        // Obsolete now that V8 updates reactively, but kept for compatibility 
        // with other parts of the code. We can optionally send an 'init' to trigger state.
    }

    function _refreshImpl() {
        // Deprecated
    }

    function setClean() {
        isDirty = false
    }

    function normalizeStructure() {
        sketch.normalizeStructure()
        refresh()
    }

    function centerStructure() {
        sketch.centerStructure()
        refresh()
    }

    // The single QML-side entry point for every structure mutation — the
    // async request/response half of the same convention is requestSerialize(reqId)
    // paired with the catch-all onStructureReady(reqId, data) handler (MainWindow.qml,
    // ToolPanel.qml); no third mechanism should be introduced for either half.
    // Parameter shape: one value → pass it directly as val; two or more related
    // values → one object literal (e.g. {above, below}, {elements, notList}).
    function applyPropertyChange(type, id, val) {
        if (type === "atomLabel") {
            sketch.changeAtomLabel(id, val)
        } else if (type === "atomCharge") {
            sketch.changeAtomCharge(id, parseInt(val) || 0)
        } else if (type === "bondType") {
            sketch.changeBondType(id, parseInt(val) || 1)
        } else if (type === "rxnArrowMode") {
            sketch.setRxnArrowMode(id, val)
        } else if (type === "rxnArrowConditions") {
            sketch.setRxnArrowConditions(id, val.above || "", val.below || "")
        } else if (type === "atomQueryList") {
            if (val.elements && val.elements.trim().length > 0) sketch.setAtomQueryList(id, val.elements, !!val.notList)
            else sketch.clearAtomQueryList(id, "C")
        } else if (type === "atomProps") {
            sketch.changeAtomIsotope(id, val.isotope || 0)
            sketch.changeAtomRadical(id, val.radical || 0)
            sketch.changeAtomValence(id, val.valence !== undefined ? val.valence : -1)
        } else if (type === "rgroupDefine") {
            sketch.addRGroup(id)
        } else if (type === "rgroupDelete") {
            sketch.deleteRGroup(id)
        } else if (type === "rgroupLogic") {
            sketch.setRGroupLogic(id, val.range || "", !!val.resth, val.ifthen || 0)
        } else if (type === "rgroupAddMember") {
            sketch.addRGroupMember(id)
        } else if (type === "rgroupRemoveMember") {
            sketch.removeRGroupMember(id, val)
        }
        refresh()
    }

    function clearCanvas() {
        sketch.clearCanvas()
        scale = 1.0
        offsetX = root.width  / 2
        offsetY = root.height / 2
        refresh()
    }

    function undo() {
        sketch.undo()
        refresh()
    }

    function redo() {
        sketch.redo()
        refresh()
    }

    function copySelection() {
        sketch.copySelection()
    }

    function cutSelection() {
        sketch.cutSelection()
        refresh()
    }

    function pasteSelection() {
        const center = canvasToChem(root.width / 2, root.height / 2)
        sketch.pasteSelection(center.x, center.y)
        refresh()
    }

    function getStructure(fmt) {
        return sketch.getStructure(fmt)
    }

    function getMolfile() {
        return getStructure('mol')
    }

    function loadMolfile(data) {
        loadStructure('mol', data)
    }

    function loadStructure(fmt, data) {
        _needsCentering = true
        sketch.loadStructure(fmt, data)
        refresh()
    }

    function serializeMol() {
        return sketch.serializeMol()
    }

    function deserializeMol(data) {
        sketch.deserializeMol(data)
        refresh()
    }

    // Resolve a hit atom ID: if it's a contracted sgroup prim, return its attachment atom ID instead
    function resolveHitAtom(id) {
        if (id === null || id === undefined || !sketch.primitives || !sketch.primitives.atomsById) return id
        const prim = sketch.primitives.atomsById[id.toString()]
        if (prim && prim.isSgroup && prim.attachAtomId !== null && prim.attachAtomId !== undefined)
            return prim.attachAtomId
        return id
    }

    function chemToCanvas(chemX, chemY) {
        if (typeof chemX === 'object' && chemX !== null) {
            chemY = chemX.y;
            chemX = chemX.x;
        }
        const f = root.chemScale
        return Qt.point(chemX * f + offsetX, chemY * f + offsetY)
    }

    function canvasToChem(cx, cy) {
        if (typeof cx === 'object' && cx !== null) {
            cy = cx.y;
            cx = cx.x;
        }
        const f = root.chemScale
        return Qt.point((cx - offsetX) / f, (cy - offsetY) / f)
    }

    // Canvas-space bounding box of the current selection, across every
    // selectable type (atoms, reaction arrows, rxn-plus signs, multitail
    // arrows -- mirrors moveSelection's own multi-type handling in
    // src/v8_worker.js), or null if fewer than 2 distinct points are present.
    // Drives the PowerPoint-style resize/rotate handles drawn on the selection.
    function selectionBBoxCanvas() {
        if (!sketch.selection || !sketch.primitives) return null
        let minX = Infinity, minY = Infinity, maxX = -Infinity, maxY = -Infinity
        let count = 0
        function feed(cx, cy) {
            const p = chemToCanvas(cx, cy)
            if (p.x < minX) minX = p.x
            if (p.x > maxX) maxX = p.x
            if (p.y < minY) minY = p.y
            if (p.y > maxY) maxY = p.y
            count++
        }
        const atomIds = sketch.selection.atom_ids || []
        for (let i = 0; i < atomIds.length; ++i) {
            const a = sketch.primitives.atomsById ? sketch.primitives.atomsById[atomIds[i].toString()] : null
            if (a) feed(a.x, a.y)
        }
        const arrowIds = sketch.selection.rxnArrow_ids || []
        if (arrowIds.length > 0 && sketch.primitives.rxnArrows) {
            for (let i = 0; i < sketch.primitives.rxnArrows.length; ++i) {
                const ar = sketch.primitives.rxnArrows[i]
                if (arrowIds.indexOf(ar.id) < 0) continue
                if (ar.p1) feed(ar.p1.x, ar.p1.y)
                if (ar.p2) feed(ar.p2.x, ar.p2.y)
            }
        }
        const plusIds = sketch.selection.rxnPlus_ids || []
        if (plusIds.length > 0 && sketch.primitives.rxnPluses) {
            for (let i = 0; i < sketch.primitives.rxnPluses.length; ++i) {
                const pl = sketch.primitives.rxnPluses[i]
                if (plusIds.indexOf(pl.id) >= 0) feed(pl.x, pl.y)
            }
        }
        const mtaIds = sketch.selection.multitailArrow_ids || []
        if (mtaIds.length > 0 && sketch.primitives.multitailArrows) {
            for (let i = 0; i < sketch.primitives.multitailArrows.length; ++i) {
                const mta = sketch.primitives.multitailArrows[i]
                if (mtaIds.indexOf(mta.id) < 0) continue
                feed(mta.spineTopX, mta.spineTopY)
                feed(mta.spineTopX, mta.spineTopY + mta.height)
            }
        }
        if (count < 2) return null
        return { minX: minX, minY: minY, maxX: maxX, maxY: maxY }
    }

    // PowerPoint-style handle geometry for a selection bbox: 4 corner handles,
    // 4 edge-midpoint handles, and 1 rotate handle above the top edge. Each
    // resize handle carries its geometric opposite point ("anchor") -- the
    // point that stays fixed while dragging that handle -- since resize scales
    // about the opposite corner/edge, not the centroid (unlike rotate).
    function selectionHandles(bbox) {
        const cxm = (bbox.minX + bbox.maxX) / 2
        const cym = (bbox.minY + bbox.maxY) / 2
        return {
            rotate: { x: cxm, y: bbox.minY - 24 },
            resize: [
                { x: bbox.minX, y: bbox.minY, anchorX: bbox.maxX, anchorY: bbox.maxY, cursor: Qt.SizeFDiagCursor },
                { x: bbox.maxX, y: bbox.minY, anchorX: bbox.minX, anchorY: bbox.maxY, cursor: Qt.SizeBDiagCursor },
                { x: bbox.maxX, y: bbox.maxY, anchorX: bbox.minX, anchorY: bbox.minY, cursor: Qt.SizeFDiagCursor },
                { x: bbox.minX, y: bbox.maxY, anchorX: bbox.maxX, anchorY: bbox.minY, cursor: Qt.SizeBDiagCursor },
                { x: cxm, y: bbox.minY, anchorX: cxm, anchorY: bbox.maxY, cursor: Qt.SizeVerCursor },
                { x: cxm, y: bbox.maxY, anchorX: cxm, anchorY: bbox.minY, cursor: Qt.SizeVerCursor },
                { x: bbox.minX, y: cym, anchorX: bbox.maxX, anchorY: cym, cursor: Qt.SizeHorCursor },
                { x: bbox.maxX, y: cym, anchorX: bbox.minX, anchorY: cym, cursor: Qt.SizeHorCursor }
            ]
        }
    }

    function fitToMolecule() {
        const prims = sketch.primitives
        if (!prims || !prims.bbox) return
        const bb = prims.bbox
        const w = bb.maxX - bb.minX
        const h = bb.maxY - bb.minY
        let newScale
        if (w < 0.01 && h < 0.01) {
            newScale = 1.0
        } else {
            const pad = 0.70
            const sx = w > 0.01 ? (root.width  * pad) / (w * bondLength) : 999
            const sy = h > 0.01 ? (root.height * pad) / (h * bondLength) : 999
            // Ceiling 1.5: "fit" may shrink a large molecule to view, but must not
            // blow a small one up to fill the page (a lone ring is not a poster).
            newScale = Math.max(0.1, Math.min(1.5, Math.min(sx, sy)))
        }
        scale = newScale
        const chemCenterX = (bb.minX + bb.maxX) / 2
        const chemCenterY = (bb.minY + bb.maxY) / 2
        offsetX = root.width  / 2 - chemCenterX * root.chemScale
        offsetY = root.height / 2 - chemCenterY * root.chemScale
    }

    function centerOnMolecule() { fitToMolecule() }

    function _grabClean(fileUrl, afterSave) {
        gridCanvas.visible = false
        selectionLayer.visible = false
        toolOverlayItem.visible = false
        exportBg.visible = true
        root.grabToImage(function(result) {
            result.saveToFile(fileUrl)
            if (afterSave) afterSave(fileUrl)
            gridCanvas.visible = true
            selectionLayer.visible = true
            toolOverlayItem.visible = true
            exportBg.visible = false
        })
    }

    function exportPNG(fileUrl) { _grabClean(fileUrl, null) }

    // Grabs a clean (no grid/selection/overlay) PNG, then invokes afterSave(url) —
    // used to feed AppController.exportPdf() the same clean render Copy-as-Image uses.
    function exportForPrint(fileUrl, afterSave) { _grabClean(fileUrl, afterSave) }

    function copyAsImage() {
        const tmp = AppController.tempPngPath()
        const tmpUrl = Qt.url("file:///" + tmp.replace(/\\/g, "/"))
        _grabClean(tmpUrl, function(url) { AppController.copyImageToClipboard(url) })
    }

    function zoomAt(cx, cy, factor) {
        const chem = canvasToChem(cx, cy)
        scale = Math.max(0.1, Math.min(20.0, scale * factor))
        const fNew = root.chemScale
        offsetX = cx - chem.x * fNew
        offsetY = cy - chem.y * fNew
    }

    Rectangle {
        id: exportBg
        anchors.fill: parent
        color: "white"
        z: -1
        visible: false
    }

    // Subtle dot grid overlay
    Canvas {
        id: gridCanvas
        anchors.fill: parent
        antialiasing: true
        onPaint: {
            const ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            let gridSize = 40 * root.scale
            while (gridSize < 8) gridSize *= 2
            const dotRadius = Math.max(0.8, 1.2 * root.scale)
            ctx.fillStyle = Theme.rulerColor
            ctx.globalAlpha = 0.18
            const startX = root.offsetX % gridSize
            const startY = root.offsetY % gridSize
            ctx.beginPath()
            for (let x = startX; x < width; x += gridSize) {
                for (let y = startY; y < height; y += gridSize) {
                    // moveTo before each arc starts a fresh subpath at the circle's own
                    // starting point -- without it, arc() draws a straight connecting line
                    // from the previous circle's endpoint to this one's start, turning the
                    // whole column into one continuous filled wedge instead of separate dots.
                    ctx.moveTo(x + dotRadius, y)
                    ctx.arc(x, y, dotRadius, 0, Math.PI * 2)
                }
            }
            ctx.fill()
            ctx.globalAlpha = 1.0
        }
    }

    // Theme flips (dark mode) must repaint every Canvas layer — they read Theme
    // colors imperatively at paint time, not through bindings.
    Connections {
        target: Theme
        function onDarkModeChanged() {
            root._renderVersion++
            gridCanvas.requestPaint()
        }
    }

    Connections {
        target: root
        function onScaleChanged() { gridCanvas.requestPaint() }
        function onOffsetXChanged() { gridCanvas.requestPaint() }
        function onOffsetYChanged() { gridCanvas.requestPaint() }
        function onWidthChanged() { gridCanvas.requestPaint() }
        function onHeightChanged() { gridCanvas.requestPaint() }
    }

    MoleculeLayer {
        id: moleculeLayer
        anchors.fill: parent
        scale: root.scale
        offsetX: root.offsetX
        offsetY: root.offsetY
        bondLength: root.bondLength
        canvas: root
        renderVersion: root._renderVersion
    }

    SelectionLayer {
        id: selectionLayer
        anchors.fill: parent
        scale: root.scale
        offsetX: root.offsetX
        offsetY: root.offsetY
        bondLength: root.bondLength
        canvas: root
        renderVersion: root._renderVersion
        overlayVersion: root._overlayVersion
    }

    LabelLayer {
        id: labelLayer
        anchors.fill: parent
        scale: root.scale
        offsetX: root.offsetX
        offsetY: root.offsetY
        bondLength: root.bondLength
        canvas: root
        renderVersion: root._renderVersion
    }

    ToolOverlay {
        id: toolOverlayItem
        anchors.fill: parent
        overlayVersion: root._overlayVersion
        canvas: root
        renderVersion: root._renderVersion
        scale: root.scale
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.AllButtons
        cursorShape: {
            if (mouse.panning) return Qt.ClosedHandCursor
            if (rotatingSelection) return Qt.CrossCursor
            if (resizingSelection) return Qt.SizeAllCursor
            if (currentTool === "HAND") return Qt.OpenHandCursor
            if (currentTool === "SELECT" || currentTool === "SELECT_FRAGMENT") {
                const hb = selectionBBoxCanvas()
                if (hb) {
                    const handles = selectionHandles(hb)
                    const rdx = mouse.mouseX - handles.rotate.x, rdy = mouse.mouseY - handles.rotate.y
                    if (rdx * rdx + rdy * rdy <= 100) return Qt.CrossCursor
                    for (let hi = 0; hi < handles.resize.length; ++hi) {
                        const h = handles.resize[hi]
                        const hdx = mouse.mouseX - h.x, hdy = mouse.mouseY - h.y
                        if (hdx * hdx + hdy * hdy <= 64) return h.cursor
                    }
                }
                return Qt.ArrowCursor
            }
            if (currentTool === "SELECT_LASSO") return Qt.ArrowCursor
            if (currentTool === "ERASE") return Qt.PointingHandCursor
            return Qt.CrossCursor
        }

        property real pressX: 0
        property real pressY: 0
        property real startPressX: 0
        property real startPressY: 0
        property bool panning: false
        property bool isDragging: false
        property bool isAppControllerDragging: false
        property bool movingSelection: false
        property bool shiftAtPress: false

        // Selection-handle rotate drag state (canvas-space center + unwrapped
        // accumulated angle, so a full-circle drag doesn't jump at the atan2
        // +-pi seam). Triggered by grabbing the bbox's rotate handle directly
        // in SELECT/SELECT_FRAGMENT mode -- no separate tool needed.
        property bool rotatingSelection: false
        property real rotateCenterX: 0
        property real rotateCenterY: 0
        property real rotateLastAngle: 0
        property real rotateRawTotal: 0
        property real rotateAppliedTotal: 0

        // Selection-handle resize drag state. The anchor (canvas-space at
        // press time, chem-space for the backend call) is the handle's
        // geometric opposite point and stays fixed for the whole drag.
        property bool resizingSelection: false
        property real resizeAnchorCanvasX: 0
        property real resizeAnchorCanvasY: 0
        property real resizeAnchorChemX: 0
        property real resizeAnchorChemY: 0
        property real resizeOrigDist: 0

        onWheel: (wheel) => {
            const factor = wheel.angleDelta.y > 0 ? 1.15 : (1.0 / 1.15)
            zoomAt(wheel.x, wheel.y, factor)
        }

        onPressed: (m) => {
            root.forceActiveFocus()
            pressX = m.x
            pressY = m.y
            startPressX = m.x
            startPressY = m.y
            shiftAtPress = (m.modifiers & Qt.ShiftModifier) !== 0
            if (m.button === Qt.MiddleButton || (m.button === Qt.LeftButton && currentTool === "HAND")) {
                panning = true
                _panningActive = true
                return
            }
            if (m.button === Qt.RightButton) {
                const hitAtom = selectionLayer.hitTestAtom(m.x, m.y)
                const hitBond = (hitAtom === null) ? selectionLayer.hitTestBond(m.x, m.y) : null
                contextMenu._hitAtom = hitAtom
                contextMenu._hitBond = hitBond
                contextMenu.popup(m.x, m.y)
                return
            }
            if (m.button === Qt.LeftButton) {
                const hitAtom = selectionLayer.hitTestAtom(m.x, m.y)
                const hitBond = hitAtom === null ? selectionLayer.hitTestBond(m.x, m.y) : null
                const hitRxnArrow = (hitAtom === null && hitBond === null) ? selectionLayer.hitTestRxnArrow(m.x, m.y) : null
                const hitRxnPlus = (hitAtom === null && hitBond === null && hitRxnArrow === null) ? selectionLayer.hitTestRxnPlus(m.x, m.y) : null
                const hitMultitailArrow = (hitAtom === null && hitBond === null && hitRxnArrow === null && hitRxnPlus === null) ? selectionLayer.hitTestMultitailArrow(m.x, m.y) : null
                
                if (currentTool === "SELECT" || currentTool === "SELECT_FRAGMENT") {
                    // PowerPoint-style selection handles take priority over the
                    // normal hit-test below since they're drawn on top and only
                    // exist when a qualifying selection is already present.
                    const handleBBox = selectionBBoxCanvas()
                    if (handleBBox) {
                        const handles = selectionHandles(handleBBox)
                        const rdx = m.x - handles.rotate.x, rdy = m.y - handles.rotate.y
                        if (rdx * rdx + rdy * rdy <= 100) {
                            const cxm = (handleBBox.minX + handleBBox.maxX) / 2
                            const cym = (handleBBox.minY + handleBBox.maxY) / 2
                            rotatingSelection = true
                            rotateCenterX = cxm
                            rotateCenterY = cym
                            rotateLastAngle = Math.atan2(m.y - cym, m.x - cxm)
                            rotateRawTotal = 0
                            rotateAppliedTotal = 0
                            return
                        }
                        for (let hi = 0; hi < handles.resize.length; ++hi) {
                            const h = handles.resize[hi]
                            const hdx = m.x - h.x, hdy = m.y - h.y
                            if (hdx * hdx + hdy * hdy <= 64) {
                                const anchorChem = canvasToChem(h.anchorX, h.anchorY)
                                resizingSelection = true
                                resizeAnchorCanvasX = h.anchorX
                                resizeAnchorCanvasY = h.anchorY
                                resizeAnchorChemX = anchorChem.x
                                resizeAnchorChemY = anchorChem.y
                                resizeOrigDist = Math.sqrt((h.x - h.anchorX) * (h.x - h.anchorX) + (h.y - h.anchorY) * (h.y - h.anchorY))
                                return
                            }
                        }
                    }

                    const shiftHeld = (m.modifiers & Qt.ShiftModifier) !== 0
                    if (shiftHeld && hitAtom !== null) {
                        // Shift+click atom: toggle membership in selection
                        const alreadySel = sketch.selection.atom_ids && sketch.selection.atom_ids.indexOf(hitAtom) >= 0
                        if (alreadySel) {
                            sketch.removeItemFromSelection(hitAtom, null)
                        } else {
                            sketch.addItemToSelection(hitAtom, null)
                        }
                        isDragging = true
                        movingSelection = true
                        refresh()
                    } else if (shiftHeld && hitBond !== null) {
                        // Shift+click bond: toggle membership in selection
                        const alreadySel = sketch.selection.bond_ids && sketch.selection.bond_ids.indexOf(hitBond) >= 0
                        if (alreadySel) {
                            sketch.removeItemFromSelection(null, hitBond)
                        } else {
                            sketch.addItemToSelection(null, hitBond)
                        }
                        isDragging = true
                        movingSelection = false
                        refresh()
                    } else if (shiftHeld) {
                        // Shift+click empty space: start additive rubber band without clearing
                        isDragging = true
                        movingSelection = false
                        setOverlayState({ hoverAtomId: null, hoverBondId: null, dragRect: Qt.rect(m.x, m.y, 0, 0), bondPreview: null })
                    } else if (hitAtom !== null && sketch.selection.atom_ids && sketch.selection.atom_ids.indexOf(hitAtom) !== -1) {
                        // Clicked an ALREADY selected atom -> prepare to move selection
                        isDragging = true
                        movingSelection = true
                    } else if (hitAtom !== null) {
                        if (currentTool === "SELECT_FRAGMENT") {
                            sketch.selectFragment(hitAtom, null)
                            isDragging = true
                            movingSelection = true
                        } else {
                            const hitPrim = sketch.primitives.atomsById[hitAtom.toString()]
                            if (hitPrim && hitPrim.isSgroup) {
                                // Sgroup pill: select it and allow moving, no bond preview
                                sketch.selectItem(hitAtom, null)
                                isDragging = true
                                movingSelection = true
                                setOverlayState({ hoverAtomId: hitAtom, hoverBondId: null, dragRect: null, bondPreview: null })
                            } else {
                                // Dragging an unselected atom moves it (ChemDraw-style: drag always
                                // moves). Extending a structure by dragging out a bond is still
                                // available via the dedicated BOND_* tools.
                                sketch.selectItem(hitAtom, null)
                                isDragging = true
                                movingSelection = true
                                setOverlayState({ hoverAtomId: hitAtom, hoverBondId: null, dragRect: null, bondPreview: null })
                            }
                        }
                    } else if (hitBond !== null) {
                        if (currentTool === "SELECT_FRAGMENT") {
                            sketch.selectFragment(null, hitBond)
                            isDragging = true
                            movingSelection = true
                        } else if (sketch.selection.bond_ids && sketch.selection.bond_ids.indexOf(hitBond) >= 0 &&
                                   sketch.selection.atom_ids && sketch.selection.atom_ids.length > 0) {
                            // Clicked an ALREADY selected bond (with atoms also selected) -> prepare to move selection
                            isDragging = true
                            movingSelection = true
                        } else {
                            // Dragging an unselected bond moves it (matches the atom case above).
                            sketch.selectItem(null, hitBond)
                            isDragging = true
                            movingSelection = true
                            setOverlayState({ hoverAtomId: null, hoverBondId: hitBond, dragRect: null, bondPreview: null })
                        }
                    } else if (hitRxnArrow !== null) {
                        sketch.selectItem(null, null, hitRxnArrow, null)
                        isDragging = true
                        movingSelection = true
                    } else if (hitRxnPlus !== null) {
                        sketch.selectItem(null, null, null, hitRxnPlus)
                        isDragging = true
                        movingSelection = true
                    } else if (hitMultitailArrow !== null) {
                        sketch.selectItem(null, null, null, null, hitMultitailArrow)
                        isDragging = true
                        movingSelection = true
                    } else {
                        sketch.selectItem(null, null)
                        isDragging = true
                        movingSelection = false
                        setOverlayState({
                            hoverAtomId: null,
                            hoverBondId: null,
                            dragRect: Qt.rect(m.x, m.y, 0, 0),
                            bondPreview: null
                        })
                    }
                } else if (currentTool === "ERASE") {
                    if (hitAtom !== null) {
                        sketch.deleteAtomById(hitAtom)
                        refresh()
                    } else if (hitBond !== null) {
                        sketch.deleteBondById(hitBond)
                        refresh()
                    } else if (hitRxnArrow !== null) {
                        sketch.sendCommand("deleteRxnArrow", [hitRxnArrow])
                        refresh()
                    } else if (hitRxnPlus !== null) {
                        sketch.sendCommand("deleteRxnPlus", [hitRxnPlus])
                        refresh()
                    } else if (hitMultitailArrow !== null) {
                        sketch.deleteMultitailArrow(hitMultitailArrow)
                        refresh()
                    }
                } else if (currentTool.startsWith("BOND_")) {
                    if (hitBond !== null && hitAtom === null) {
                        isDragging = true
                        setOverlayState({
                            hoverAtomId: null,
                            hoverBondId: hitBond,
                            dragRect: null,
                            bondPreview: null
                        })
                        return
                    }
                    let startAtomId = resolveHitAtom(hitAtom)
                    const chemP = canvasToChem(m.x, m.y)
                    isDragging = true
                    setOverlayState({
                        hoverAtomId: hitAtom,
                        hoverBondId: hitBond,
                        dragRect: null,
                        bondPreview: {
                            startAtomId: startAtomId,
                            startX: chemP.x,

                            startY: chemP.y,
                            endX: m.x,
                            endY: m.y
                        }
                    })
                } else if (currentTool.startsWith("ATOM_") || currentTool.startsWith("FG_") || currentTool.startsWith("SS_") || currentTool.startsWith("LIB_") || currentTool.startsWith("TEMPLATE_")) {
                    if (hitAtom !== null) {
                        const cP = canvasToChem(m.x, m.y)
                        // resolveHitAtom: hitAtom may be a contracted sgroup pill id here.
                        // The drag-end fallback path already resolves this (see
                        // resolveHitAtom(hid) below); without it here too, starting a drag
                        // on top of an existing pill grafts nothing and the new group lands
                        // as a disconnected fragment instead of bonding to it.
                        AppController.handleDragStart(currentTool, resolveHitAtom(hitAtom), m.x, m.y, cP.x, cP.y)
                        isAppControllerDragging = true
                    } else {
                        if (currentTool.startsWith("ATOM_")) {
                            const atomLabel = currentTool.split("_")[1]
                            const cP = canvasToChem(m.x, m.y)
                            sketch.addAtom(atomLabel, cP.x, cP.y, 0)
                            refresh()
                        } else if (currentTool.startsWith("FG_") || currentTool.startsWith("SS_")) {
                            const fgName = currentTool.substring(3)
                            const cP = canvasToChem(m.x, m.y)
                            sketch.insertFunctionalGroup(fgName, cP.x, cP.y)
                            refresh()
                        } else if (currentTool.startsWith("LIB_")) {
                            const cP = canvasToChem(m.x, m.y)
                            // Clicking directly on an existing bond fuses the template's
                            // designated ring edge onto it (library.sdf <bondid> metadata);
                            // the worker falls back to plain placement itself when the
                            // template has no fusion metadata, so no check needed here.
                            // Mirrors the TEMPLATE_ branch below, which already passes
                            // hitBond into getRingPreviewCoords the same way.
                            if (hitBond !== null)
                                sketch.insertLibraryTemplateFused(currentTool.substring(4), cP.x, cP.y, hitBond)
                            else
                                sketch.insertFunctionalGroup(currentTool.substring(4), cP.x, cP.y)
                            refresh()
                        } else if (currentTool.startsWith("TEMPLATE_")) {
                            const cP2 = canvasToChem(m.x, m.y)
                            let ringSize = 6
                            if (currentTool === "TEMPLATE_BENZENE") {
                                ringSize = 6
                            } else {
                                const parsed = parseInt(currentTool.split("_")[1])
                                if (!isNaN(parsed)) ringSize = parsed
                            }
                            var coords = sketch.getRingPreviewCoords(
                                ringSize, cP2.x, cP2.y, hitAtom, hitBond
                            );
                            if (coords && coords.length > 0) {
                                const coordList = []
                                for (let i = 0; i < coords.length; i++) {
                                    coordList.push(coords[i].x);
                                    coordList.push(coords[i].y);
                                }
                                sketch.addRing(coordList, currentTool === "TEMPLATE_BENZENE")
                            }
                            refresh()
                        }
                    }
                } else if (currentTool === "CHAIN") {
                    // Drag lays a zig-zag carbon chain; the bond-preview line doubles
                    // as the drag feedback (same overlay the BOND tools use).
                    isDragging = true
                    setOverlayState({
                        hoverAtomId: null, hoverBondId: null, dragRect: null,
                        bondPreview: { startX: canvasToChem(m.x, m.y).x, startY: canvasToChem(m.x, m.y).y, endX: m.x, endY: m.y }
                    })
                } else if (currentTool === "TEXT") {
                    const hitText = hitTestText(m.x, m.y)
                    const cP = canvasToChem(m.x, m.y)
                    if (hitText !== null) {
                        textEditRequested(hitText.id, hitText.content, hitText.x, hitText.y)
                    } else {
                        textEditRequested(-1, "", cP.x, cP.y)
                    }
                } else if (currentTool === "IMAGE") {
                    const cP = canvasToChem(m.x, m.y)
                    imageInsertRequested(cP.x, cP.y)
                } else if (currentTool === "RXN_ARROW") {
                    const cP = canvasToChem(m.x, m.y)
                    sketch.addRxnArrow(cP.x, cP.y, currentArrowMode)
                    refresh()
                } else if (currentTool === "MULTITAIL_ARROW") {
                    const cP = canvasToChem(m.x, m.y)
                    sketch.addMultitailArrow(cP.x, cP.y)
                    refresh()
                } else if (currentTool === "RXN_PLUS") {
                    const cP = canvasToChem(m.x, m.y)
                    sketch.addRxnPlus(cP.x, cP.y)
                    refresh()
                } else if (currentTool.startsWith("CHARGE_")) {
                    if (hitAtom !== null) {
                        // resolveHitAtom: hitAtom may be a contracted sgroup pill id, which
                        // isn't a real entry in the backend's atom table -- changeAtomCharge
                        // would silently do nothing without this (same fix as resolveHitAtom's
                        // other callers, e.g. bond drawing).
                        const realAtomId = resolveHitAtom(hitAtom)
                        const atom = sketch.primitives.atomsById[realAtomId.toString()]
                        if (atom) {
                            const currentCharge = atom.charge || 0
                            const newCharge = currentTool === "CHARGE_PLUS" ? currentCharge + 1 : currentCharge - 1
                            sketch.changeAtomCharge(realAtomId, newCharge)
                            refresh()
                        }
                    }
                } else if (currentTool === "AAM") {
                    if (hitAtom !== null) {
                        const realAtomId = resolveHitAtom(hitAtom)
                        const atom = sketch.primitives.atomsById[realAtomId.toString()]
                        if (atom) {
                            const currentAam = atom.aam || 0
                            const newAam = (m.modifiers & Qt.ShiftModifier) ? 0 : (currentAam >= 99 ? 0 : currentAam + 1)
                            sketch.setAtomMapping(realAtomId, newAam)
                            refresh()
                        }
                    }
                } else if (currentTool === "SELECT_LASSO") {
                    isDragging = true
                    setOverlayState({ hoverAtomId: null, hoverBondId: null, dragRect: null, bondPreview: null, lassoPath: [Qt.point(m.x, m.y)] })
                }
            }
        }

        onPositionChanged: (m) => {
            if (panning) {
                offsetX += m.x - pressX
                offsetY += m.y - pressY
                pressX = m.x
                pressY = m.y
                setOverlayState({ hoverAtomId: null, hoverBondId: null, dragRect: null, bondPreview: null })
                return
            }

            if (rotatingSelection) {
                const angleNow = Math.atan2(m.y - rotateCenterY, m.x - rotateCenterX)
                let d = angleNow - rotateLastAngle
                if (d > Math.PI) d -= 2 * Math.PI
                if (d < -Math.PI) d += 2 * Math.PI
                rotateRawTotal += d
                rotateLastAngle = angleNow
                const shiftHeld = (m.modifiers & Qt.ShiftModifier) !== 0
                const step = Math.PI / 12  // 15 degrees
                const desiredTotal = shiftHeld ? Math.round(rotateRawTotal / step) * step : rotateRawTotal
                const incremental = desiredTotal - rotateAppliedTotal
                if (incremental !== 0) {
                    sketch.rotateSelectionLive(incremental)
                    rotateAppliedTotal = desiredTotal
                    refresh()
                }
                return
            }

            if (resizingSelection) {
                const rdx = m.x - resizeAnchorCanvasX, rdy = m.y - resizeAnchorCanvasY
                const dist = Math.sqrt(rdx * rdx + rdy * rdy)
                if (resizeOrigDist > 0) {
                    const factor = dist / resizeOrigDist
                    sketch.scaleSelectionLive(factor, resizeAnchorChemX, resizeAnchorChemY)
                    refresh()
                }
                return
            }

            const hoverAtomId = selectionLayer.hitTestAtom(m.x, m.y)
            const hoverBondId = hoverAtomId === null ? selectionLayer.hitTestBond(m.x, m.y) : null

            if (isAppControllerDragging) {
                // Was hardcoded (m.x, m.y, 1.0): the pixel-to-chemical conversion inside
                // PlacementPreviewManager::updatePreview used a stale hardcoded 37.8
                // instead of the actual zoom-aware chemScale (pixels per 1 chemical unit,
                // same value canvasToChem/chemToCanvas use), and the placement distance was
                // 1.0 chemical units instead of 1.5 (chem-core.js's StandardBondLength) --
                // together, every drag-placed atom/fragment landed at the wrong distance
                // from its neighbor, worse the more zoomed in/out you were.
                AppController.handleDrag(m.x, m.y, chemScale, 1.5)
                return
            }

            if (isDragging) {
                if (!(m.buttons & Qt.LeftButton)) {
                    isDragging = false
                    AppController.clearChainPreview()
                    setOverlayState({ hoverAtomId: hoverAtomId, hoverBondId: hoverBondId, dragRect: null, bondPreview: null })
                    return
                }

                if (currentTool === "CHAIN") {
                    // Live zigzag ghost (matches what addChain will actually commit) via
                    // AppController/ChainPlacementEngine, published into overlayState.
                    // previewAtoms/previewBonds and rendered by ToolOverlay.qml. bondPreview
                    // itself is deliberately left frozen at its press-time value (a zero-length
                    // segment) rather than updated to the live mouse position, so the old plain
                    // rubber-band line this overlay used to draw for CHAIN doesn't render on
                    // top of the new zigzag ghost.
                    //
                    // Ordering matters here: setOverlayState() replaces sketch.overlayState
                    // wholesale (it's `sketch.overlayState = s`, not a merge), while
                    // AppController.updateChainPreview() does a proper C++-side read-modify-write
                    // that preserves whatever is already in overlayState. Calling
                    // updateChainPreview() AFTER setOverlayState() means its merge is the one
                    // that lands; doing it the other way around silently wiped out the
                    // previewAtoms/previewBonds it had just set, one JS tick before any repaint.
                    const bp = sketch.overlayState.bondPreview
                    if (bp) {
                        const curChem = canvasToChem(m.x, m.y)
                        setOverlayState({ hoverAtomId: null, hoverBondId: null, dragRect: null, bondPreview: bp })
                        // Chain geometry lives in chem-coordinate space (bp.startX/startY and
                        // curChem are both canvasToChem output), so the divisor here must be
                        // v8_worker.js addChain's own chem-unit bond length (chem-core.js:
                        // StandardBondLength = MonomerSize(0.75) * 2 = 1.5) - NOT root.bondLength
                        // (Theme.baseBondLength, a PIXEL value used for canvas<->chem scaling
                        // elsewhere). Passing the pixel value here was the first attempt and
                        // produced a wildly oversized single-segment ghost: a chem-space distance
                        // of ~13 units divided by ~19 (pixels, wrong unit) rounds down to 1 bond,
                        // then that 1 bond's "length" gets stepped by 19 chem-units instead of
                        // 1.5, landing the second atom ~19x too far away.
                        AppController.updateChainPreview(bp.startX, bp.startY, curChem.x, curChem.y, 1.5)
                    }
                    return
                }

                if (currentTool === "SELECT_LASSO") {
                    const path = (sketch.overlayState.lassoPath || []).concat([Qt.point(m.x, m.y)])
                    setOverlayState({ hoverAtomId: null, hoverBondId: null, dragRect: null, bondPreview: null, lassoPath: path })
                    return
                }

                if (currentTool === "SELECT" || currentTool === "SELECT_FRAGMENT") {
                    if (movingSelection) {
                        const dx = m.x - pressX
                        const dy = m.y - pressY
                        const chemDx = dx / root.chemScale
                        const chemDy = dy / root.chemScale
                        sketch.moveSelection(chemDx, chemDy)
                        refresh()
                        pressX = m.x
                        pressY = m.y
                    } else if (sketch.overlayState.bondPreview) {
                        // Smart drawing bond
                        let targetX = m.x
                        let targetY = m.y
                        if (hoverAtomId !== null && hoverAtomId !== sketch.overlayState.bondPreview.startAtomId) {
                            for (let i = 0; i < sketch.primitives.atoms.length; i++) {
                                if (sketch.primitives.atoms[i].id === hoverAtomId) {
                                    const aPos = chemToCanvas(sketch.primitives.atoms[i].x, sketch.primitives.atoms[i].y)
                                    targetX = aPos.x
                                    targetY = aPos.y
                                    break
                                }
                            }
                        }
                        setOverlayState({
                            hoverAtomId: hoverAtomId,
                            hoverBondId: hoverBondId,
                            dragRect: null,
                            bondPreview: {
                                startAtomId: sketch.overlayState.bondPreview.startAtomId,
                                startX: sketch.overlayState.bondPreview.startX,
                                startY: sketch.overlayState.bondPreview.startY,
                                endX: targetX,
                                endY: targetY
                            }
                        })
                    } else if (sketch.overlayState.dragRect !== null) {
                        const x = Math.min(pressX, m.x)
                        const y = Math.min(pressY, m.y)
                        const w = Math.abs(m.x - pressX)
                        const h = Math.abs(m.y - pressY)
                        setOverlayState({
                            hoverAtomId: hoverAtomId,
                            hoverBondId: hoverBondId,
                            dragRect: Qt.rect(x, y, w, h),
                            bondPreview: null
                        })
                    }
                    return
                } else if (currentTool.startsWith("BOND_") && sketch.overlayState.bondPreview) {
                    let targetX = m.x
                    let targetY = m.y
                    if (hoverAtomId !== null && hoverAtomId !== sketch.overlayState.bondPreview.startAtomId) {
                        // Snap to atom
                        for (let i = 0; i < sketch.primitives.atoms.length; i++) {
                            if (sketch.primitives.atoms[i].id === hoverAtomId) {
                                const aPos = chemToCanvas(sketch.primitives.atoms[i].x, sketch.primitives.atoms[i].y)
                                targetX = aPos.x
                                targetY = aPos.y
                                break
                            }
                        }
                    }
                    setOverlayState({
                        hoverAtomId: hoverAtomId,
                        hoverBondId: hoverBondId,
                        dragRect: null,
                        bondPreview: {
                            startAtomId: sketch.overlayState.bondPreview.startAtomId,
                            startX: sketch.overlayState.bondPreview.startX,
                            startY: sketch.overlayState.bondPreview.startY,
                            endX: targetX,
                            endY: targetY
                        }
                    })
                    return
                }
            }

            // Normal hover logic
            let hoverRingSize = null
            if (currentTool.startsWith("TEMPLATE_")) {
                // TEMPLATE_BENZENE has no numeric suffix; without the explicit 6 the
                // benzene tool gets no ring ghost at all.
                hoverRingSize = currentTool === "TEMPLATE_BENZENE" ? 6 : (parseInt(currentTool.split("_")[1]) || null);
            }
            if (hoverAtomId !== sketch.overlayState.hoverAtomId || hoverBondId !== sketch.overlayState.hoverBondId || hoverRingSize !== null) {
                setOverlayState({
                    hoverAtomId: hoverAtomId,
                    hoverBondId: hoverBondId,
                    dragRect: sketch.overlayState.dragRect,
                    bondPreview: sketch.overlayState.bondPreview,
                    hoverRingSize: hoverRingSize,
                    mouseX: m.x,
                    mouseY: m.y
                })
            }
        }

        onReleased: (m) => {
            if (panning) {
                panning = false
                _panningActive = false
                _renderVersion++  // force full repaint of all layers
                return
            }
            if (rotatingSelection) {
                rotatingSelection = false
                if (rotateAppliedTotal !== 0) {
                    sketch.commitRotate()
                }
                return
            }
            if (resizingSelection) {
                resizingSelection = false
                sketch.commitScale()
                return
            }
            if (isDragging && currentTool === "SELECT_LASSO") {
                isDragging = false
                const path = sketch.overlayState.lassoPath || []
                setOverlayState({ hoverAtomId: null, hoverBondId: null, dragRect: null, bondPreview: null, lassoPath: null })
                if (path.length >= 3) {
                    const flat = []
                    for (let i = 0; i < path.length; ++i) {
                        const cp = canvasToChem(path[i].x, path[i].y)
                        flat.push(cp.x); flat.push(cp.y)
                    }
                    sketch.selectByLasso(flat)
                }
                return
            }
            if (isDragging && currentTool === "CHAIN") {
                isDragging = false
                AppController.clearChainPreview()
                const bp = sketch.overlayState.bondPreview
                setOverlayState({ hoverAtomId: null, hoverBondId: null, dragRect: null, bondPreview: null })
                if (bp) {
                    const endChem = canvasToChem(m.x, m.y)
                    const dx = endChem.x - bp.startX, dy = endChem.y - bp.startY
                    if (dx * dx + dy * dy > 0.25) {  // ignore sub-half-bond accidental drags
                        sketch.addChain(bp.startX, bp.startY, endChem.x, endChem.y)
                        refresh()
                    }
                }
                return
            }
            if (isAppControllerDragging) {
                isAppControllerDragging = false
                let wasDrag = AppController.handleDragEnd()
                if (!wasDrag) {
                    // Fallback to click behavior
                    if (currentTool.startsWith("ATOM_")) {
                        const atomLabel = currentTool.split("_")[1]
                        if (sketch.overlayState.hoverAtomId !== null) {
                            sketch.changeAtomLabel(sketch.overlayState.hoverAtomId, atomLabel)
                        }
                    } else if (currentTool.startsWith("FG_") || currentTool.startsWith("SS_") || currentTool.startsWith("LIB_")) {
                        const fgName = currentTool.startsWith("LIB_") ? currentTool.substring(4) : currentTool.substring(3)
                        if (sketch.overlayState.hoverAtomId !== null) {
                            const hid = sketch.overlayState.hoverAtomId
                            const atom = sketch.primitives.atomsById[hid.toString()]
                            const realTargetId = resolveHitAtom(hid)
                            sketch.insertFunctionalGroup(fgName, atom.x, atom.y, realTargetId)
                        }
                    } else if (currentTool.startsWith("TEMPLATE_")) {
                        const cP2 = canvasToChem(m.x, m.y)
                        let ringSize = 6
                        if (currentTool === "TEMPLATE_BENZENE") { ringSize = 6 }
                        else { const parsed = parseInt(currentTool.split("_")[1]); if (!isNaN(parsed)) ringSize = parsed; }
                        
                        var coords = sketch.getRingPreviewCoords(
                            ringSize, cP2.x, cP2.y, sketch.overlayState.hoverAtomId, null
                        );
                        if (coords && coords.length > 0) {
                            const coordList = []
                            for (let i = 0; i < coords.length; i++) {
                                coordList.push(coords[i].x); coordList.push(coords[i].y);
                            }
                            sketch.addRing(coordList, currentTool === "TEMPLATE_BENZENE")
                        }
                    }
                }
                refresh()
                return
            }

            if (isDragging) {
                isDragging = false
                if (currentTool === "SELECT" || currentTool === "SELECT_FRAGMENT") {
                    if (movingSelection) {
                        movingSelection = false
                        const dx = m.x - startPressX
                        const dy = m.y - startPressY
                        if (Math.abs(dx) >= 5 || Math.abs(dy) >= 5) {
                            sketch.commitMove()
                        }
                    } else if (sketch.overlayState.bondPreview) {
                        const dx = m.x - startPressX
                        const dy = m.y - startPressY
                        if (Math.abs(dx) < 5 && Math.abs(dy) < 5) {
                            // Short click -> just select it
                            sketch.selectItem(sketch.overlayState.bondPreview.startAtomId, null)
                        } else {
                            // Drew a bond!
                            const endAtomId = resolveHitAtom(sketch.overlayState.hoverAtomId)
                            const startAtomId = sketch.overlayState.bondPreview.startAtomId
                            if (endAtomId === null) {
                                const endChemP = canvasToChem(m.x, m.y)
                                sketch.addBondAndAtom(startAtomId, "C", endChemP.x, endChemP.y, 1, 0)
                            } else if (startAtomId !== endAtomId) {
                                sketch.addBond(startAtomId, endAtomId, 1, 0)
                            }
                            refresh()
                        }
                    } else if (sketch.overlayState.dragRect !== null) {
                        const dx = m.x - startPressX
                        const dy = m.y - startPressY
                        if (Math.abs(dx) >= 5 || Math.abs(dy) >= 5) {
                            const r = sketch.overlayState.dragRect
                            const tl = canvasToChem(r.x, r.y)
                            const br = canvasToChem(r.x + r.width, r.y + r.height)
                            if (shiftAtPress) {
                                sketch.addSelectionByRect(tl.x, tl.y, br.x, br.y)
                            } else {
                                sketch.selectByRect(tl.x, tl.y, br.x, br.y)
                            }
                        } else if (sketch.overlayState.hoverBondId !== null && sketch.overlayState.hoverAtomId === null) {
                            // Short click on bond: select it (SELECT) or cycle type (if already selected)
                            const hitBond = sketch.overlayState.hoverBondId
                            const alreadySel = sketch.selection.bond_ids && sketch.selection.bond_ids.indexOf(hitBond) >= 0
                            if (alreadySel && currentTool === "SELECT") {
                                // Second click cycles bond type: 1→2→3→1
                                let bObj = null
                                for (let j = 0; j < sketch.primitives.bonds.length; j++) {
                                    if (sketch.primitives.bonds[j].id === hitBond) { bObj = sketch.primitives.bonds[j]; break; }
                                }
                                if (bObj) {
                                    sketch.changeBondType(hitBond, bObj.type === 1 ? 2 : (bObj.type === 2 ? 3 : 1), 0)
                                    refresh()
                                }
                            } else {
                                sketch.selectItem(null, hitBond)
                            }
                        }
                    }
                } else if (currentTool.startsWith("BOND_")) {
                    if (sketch.overlayState.hoverBondId !== null && sketch.overlayState.bondPreview === null) {
                        const dx = m.x - pressX
                        const dy = m.y - pressY
                        if (Math.abs(dx) < 5 && Math.abs(dy) < 5) {
                            const targetBond = sketch.overlayState.hoverBondId
                            let newType = 1, newStereo = 0
                            if (currentTool === "BOND_UP") { newType = 1; newStereo = 1 }
                            else if (currentTool === "BOND_DOWN") { newType = 1; newStereo = 6 }
                            else if (currentTool === "BOND_UPDOWN") { newType = 1; newStereo = 4 }
                            else if (currentTool === "BOND_1") {
                                let bObj = null
                                for (let k=0; k<sketch.primitives.bonds.length; k++) {
                                    if (sketch.primitives.bonds[k].id === targetBond) { bObj = sketch.primitives.bonds[k]; break; }
                                }
                                if (bObj) {
                                    newType = bObj.type === 1 ? 2 : (bObj.type === 2 ? 3 : 1)
                                }
                            } else {
                                newType = parseInt(currentTool.split("_")[1]) || 1
                            }
                            sketch.changeBondType(targetBond, newType, newStereo)
                            refresh()
                        }
                    } else if (sketch.overlayState.bondPreview) {
                        const endAtomId = resolveHitAtom(sketch.overlayState.hoverAtomId)
                        const startAtomId = sketch.overlayState.bondPreview.startAtomId  // already resolved on press
                        const startX = sketch.overlayState.bondPreview.startX
                        const startY = sketch.overlayState.bondPreview.startY
                        let newType = 1, newStereo = 0
                        if (currentTool === "BOND_UP") { newType = 1; newStereo = 1 }
                        else if (currentTool === "BOND_DOWN") { newType = 1; newStereo = 6 }
                        else if (currentTool === "BOND_UPDOWN") { newType = 1; newStereo = 4 }
                        else { newType = parseInt(currentTool.split("_")[1]) || 1 }

                        if (startAtomId === null) {
                            if (endAtomId === null) {
                                const endChemP = canvasToChem(m.x, m.y)
                                sketch.addBondBetweenCoords(startX, startY, endChemP.x, endChemP.y, newType, newStereo)
                            } else {
                                sketch.addBondAndAtom(endAtomId, "C", startX, startY, newType, newStereo)
                            }
                        } else {
                            if (endAtomId === null) {
                                const endChemP = canvasToChem(m.x, m.y)
                                sketch.addBondAndAtom(startAtomId, "C", endChemP.x, endChemP.y, newType, newStereo)
                            } else if (startAtomId !== endAtomId) {
                                sketch.addBond(startAtomId, endAtomId, newType, newStereo)
                            }
                        }
                        refresh()
                    }
                }
                setOverlayState({
                    hoverAtomId: sketch.overlayState.hoverAtomId,
                    hoverBondId: sketch.overlayState.hoverBondId,
                    dragRect: null,
                    bondPreview: null
                })
            }
        }

        onDoubleClicked: (m) => {
            if (m.button !== Qt.LeftButton) return
            if (currentTool === "SELECT" || currentTool === "SELECT_FRAGMENT") {
                const hitAtom = selectionLayer.hitTestAtom(m.x, m.y)
                if (hitAtom !== null) {
                    // Cancel drag from the second press so onReleased is a no-op
                    isDragging = false
                    movingSelection = false
                    setOverlayState({ hoverAtomId: hitAtom, hoverBondId: null, dragRect: null, bondPreview: null })
                    const prim = sketch.primitives.atomsById[hitAtom.toString()]
                    if (prim && prim.isSgroup) {
                        sketch.sendCommand("toggleSgroupExpanded", [hitAtom])
                    } else {
                        root.atomPropertiesRequested(hitAtom)
                    }
                }
            }
        }
    }

    Menu {
        id: contextMenu
        property var _hitAtom: null
        property var _hitBond: null

        MenuItem {
            text: "Properties…"
            visible: contextMenu._hitAtom !== null
            // resolveHitAtom: _hitAtom may be a contracted sgroup pill id here, which
            // requestAtomProperties/changeAtomCharge don't understand (unlike
            // deleteAtomById below, which deliberately handles both cases itself).
            onTriggered: if (contextMenu._hitAtom !== null) sketch.requestAtomProperties(resolveHitAtom(contextMenu._hitAtom))
        }
        MenuItem {
            text: "Charge +"
            visible: contextMenu._hitAtom !== null
            onTriggered: if (contextMenu._hitAtom !== null) {
                const realAtomId = resolveHitAtom(contextMenu._hitAtom)
                const atom = sketch.primitives.atomsById[realAtomId.toString()]
                if (atom) sketch.changeAtomCharge(realAtomId, (atom.charge || 0) + 1)
            }
        }
        MenuItem {
            text: "Charge −"
            visible: contextMenu._hitAtom !== null
            onTriggered: if (contextMenu._hitAtom !== null) {
                const realAtomId = resolveHitAtom(contextMenu._hitAtom)
                const atom = sketch.primitives.atomsById[realAtomId.toString()]
                if (atom) sketch.changeAtomCharge(realAtomId, (atom.charge || 0) - 1)
            }
        }
        MenuItem {
            text: "Attachment Point 1"
            visible: contextMenu._hitAtom !== null
            onTriggered: if (contextMenu._hitAtom !== null) sketch.setAttachmentPoint(resolveHitAtom(contextMenu._hitAtom), 1)
        }
        MenuItem {
            text: "Attachment Point 2"
            visible: contextMenu._hitAtom !== null
            onTriggered: if (contextMenu._hitAtom !== null) sketch.setAttachmentPoint(resolveHitAtom(contextMenu._hitAtom), 2)
        }
        MenuItem {
            text: "Clear Attachment Point"
            visible: contextMenu._hitAtom !== null
            onTriggered: if (contextMenu._hitAtom !== null) sketch.setAttachmentPoint(resolveHitAtom(contextMenu._hitAtom), 0)
        }
        MenuItem {
            text: "Delete Atom"
            visible: contextMenu._hitAtom !== null
            // deleteAtomById already handles a contracted sgroup pill id itself
            // (deletes the whole group) -- do NOT resolve here, unlike the items above.
            onTriggered: if (contextMenu._hitAtom !== null) { sketch.deleteAtomById(contextMenu._hitAtom); refresh() }
        }

        MenuSeparator { visible: contextMenu._hitAtom !== null || contextMenu._hitBond !== null }

        MenuItem {
            text: "Single Bond"
            visible: contextMenu._hitBond !== null
            onTriggered: if (contextMenu._hitBond !== null) { sketch.changeBondType(contextMenu._hitBond, 1, 0); refresh() }
        }
        MenuItem {
            text: "Double Bond"
            visible: contextMenu._hitBond !== null
            onTriggered: if (contextMenu._hitBond !== null) { sketch.changeBondType(contextMenu._hitBond, 2, 0); refresh() }
        }
        MenuItem {
            text: "Triple Bond"
            visible: contextMenu._hitBond !== null
            onTriggered: if (contextMenu._hitBond !== null) { sketch.changeBondType(contextMenu._hitBond, 3, 0); refresh() }
        }
        MenuItem {
            text: "Delete Bond"
            visible: contextMenu._hitBond !== null
            onTriggered: if (contextMenu._hitBond !== null) { sketch.deleteBondById(contextMenu._hitBond); refresh() }
        }

        MenuSeparator { visible: contextMenu._hitAtom === null && contextMenu._hitBond === null }

        MenuItem {
            text: "Paste"
            visible: contextMenu._hitAtom === null && contextMenu._hitBond === null
            onTriggered: {
                const center = canvasToChem(root.width / 2, root.height / 2)
                const osText = sketch.getOsClipboardText()
                if (osText && osText.length > 0 && (osText.indexOf('"root"') >= 0 || osText.indexOf('"atoms"') >= 0)) {
                    sketch.importKetAtPosition(osText, center.x, center.y)
                } else {
                    sketch.pasteSelection(center.x, center.y)
                }
                refresh()
            }
        }
        MenuItem {
            text: "Select All"
            visible: contextMenu._hitAtom === null && contextMenu._hitBond === null
            onTriggered: { sketch.selectAll(); refresh() }
        }
        MenuItem {
            text: "Clear Canvas"
            visible: contextMenu._hitAtom === null && contextMenu._hitBond === null
            onTriggered: { sketch.clearCanvas(); refresh() }
        }
    }
}










