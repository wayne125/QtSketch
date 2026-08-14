import QtQuick
Item {
    id: root

    property Item canvas: null
    property int renderVersion: 0
    property int overlayVersion: 0
    property real scale: 1.0

    anchors.fill: parent

    Canvas {
        id: overlayCanvas
        anchors.fill: parent
        antialiasing: true
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        onPaint: {
            const ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            if (!canvas.sketch.overlayState) return

            if (!canvas.sketch.primitives || !canvas.sketch.primitives.atomsById) return

            if (canvas.sketch.overlayState.hoverAtomId !== null && canvas.sketch.overlayState.hoverAtomId !== undefined && canvas.sketch.primitives.atomsById[canvas.sketch.overlayState.hoverAtomId.toString()] && !(canvas.sketch.selection.atom_ids && canvas.sketch.selection.atom_ids.indexOf(canvas.sketch.overlayState.hoverAtomId) >= 0)) {
                const atom = canvas.sketch.primitives.atomsById[canvas.sketch.overlayState.hoverAtomId.toString()]
                const pCanvas = canvas.chemToCanvas(atom.x, atom.y)

                if (atom.isSgroup) {
                    const fontSize = Math.max(8, Theme.baseFontSize * root.scale)
                    ctx.font = "bold " + fontSize + "px " + Theme.fontFamilyCss
                    const tw = ctx.measureText(atom.label).width
                    const padX = 5 * root.scale
                    const padY = 3 * root.scale
                    const pillW = tw + padX * 2
                    const pillH = fontSize + padY * 2
                    var rx = pCanvas.x - pillW / 2
                    var ry = pCanvas.y - pillH / 2
                    var rr = pillH / 2
                    ctx.beginPath()
                    ctx.moveTo(rx + rr, ry)
                    ctx.lineTo(rx + pillW - rr, ry)
                    ctx.arcTo(rx + pillW, ry, rx + pillW, ry + rr, rr)
                    ctx.lineTo(rx + pillW, ry + pillH - rr)
                    ctx.arcTo(rx + pillW, ry + pillH, rx + pillW - rr, ry + pillH, rr)
                    ctx.lineTo(rx + rr, ry + pillH)
                    ctx.arcTo(rx, ry + pillH, rx, ry + pillH - rr, rr)
                    ctx.lineTo(rx, ry + rr)
                    ctx.arcTo(rx, ry, rx + rr, ry, rr)
                    ctx.closePath()
                    ctx.fillStyle = Theme.selectionOverlay
                    ctx.fill()
                    ctx.strokeStyle = Theme.accent
                    ctx.lineWidth = Theme.clampStrokeWidth(Theme.hoverWidth, root.scale)
                    ctx.stroke()
                } else {
                    ctx.beginPath()
                    ctx.arc(pCanvas.x, pCanvas.y, Theme.clampDim(Theme.selectionWidth, root.scale), 0, Math.PI * 2)
                    ctx.fillStyle = Theme.selectionOverlay
                    ctx.fill()
                    ctx.strokeStyle = Theme.accent
                    ctx.lineWidth = Theme.clampStrokeWidth(Theme.hoverWidth, root.scale)
                    ctx.stroke()
                }
            }

            if (canvas.sketch.overlayState.hoverBondId !== null && canvas.sketch.overlayState.hoverBondId !== undefined && canvas.sketch.primitives.bonds && !(canvas.sketch.selection.bond_ids && canvas.sketch.selection.bond_ids.indexOf(canvas.sketch.overlayState.hoverBondId) >= 0)) {
                for (let j = 0; j < canvas.sketch.primitives.bonds.length; ++j) {
                    const b = canvas.sketch.primitives.bonds[j]
                    const beginStr = b.begin.toString()
                    const endStr = b.end.toString()
                    if (b.id === canvas.sketch.overlayState.hoverBondId && canvas.sketch.primitives.atomsById[beginStr] && canvas.sketch.primitives.atomsById[endStr]) {
                        const a1 = canvas.sketch.primitives.atomsById[beginStr]
                        const a2 = canvas.sketch.primitives.atomsById[endStr]
                        const p1 = canvas.chemToCanvas(a1.x, a1.y)
                        const p2 = canvas.chemToCanvas(a2.x, a2.y)
                        ctx.beginPath()
                        ctx.moveTo(p1.x, p1.y)
                        ctx.lineTo(p2.x, p2.y)
                        ctx.strokeStyle = Theme.selectionOverlayStrong
                        ctx.lineWidth = Theme.clampStrokeWidth(Theme.hoverWidth, root.scale)
                        ctx.lineCap = "round"
                        ctx.stroke()
                        break
                    }
                }
            }

            if (canvas.sketch.overlayState.dragRect && canvas.sketch.overlayState.dragRect.width > 0) {
                const r = canvas.sketch.overlayState.dragRect
                ctx.strokeStyle = Theme.dragRect
                ctx.lineWidth = 1
                ctx.setLineDash([4, 3])
                ctx.strokeRect(r.x, r.y, r.width, r.height)
                ctx.fillStyle = Theme.dragFill
                ctx.fillRect(r.x, r.y, r.width, r.height)
                ctx.setLineDash([])
            }

            // Lasso select: freehand path, visually closed back to its start point.
            const lassoPath = canvas.sketch.overlayState.lassoPath
            if (lassoPath && lassoPath.length > 1) {
                ctx.beginPath()
                ctx.moveTo(lassoPath[0].x, lassoPath[0].y)
                for (let i = 1; i < lassoPath.length; i++) ctx.lineTo(lassoPath[i].x, lassoPath[i].y)
                ctx.closePath()
                ctx.strokeStyle = Theme.dragRect
                ctx.lineWidth = 1
                ctx.setLineDash([4, 3])
                ctx.stroke()
                ctx.fillStyle = Theme.dragFill
                ctx.fill()
                ctx.setLineDash([])
            }

            // Rotate tool: bbox outline + drag handle at the top-right corner,
            // shown only once a qualifying (>=2 atom) selection exists.
            // PowerPoint-style selection handles: appear automatically whenever
            // SELECT/SELECT_FRAGMENT has a qualifying (>=2 point) selection --
            // no separate tool needed (see selectionBBoxCanvas/selectionHandles
            // in ChemCanvas.qml).
            if (canvas.currentTool === "SELECT" || canvas.currentTool === "SELECT_FRAGMENT") {
                const bbox = canvas.selectionBBoxCanvas()
                if (bbox) {
                    ctx.strokeStyle = Theme.selectionOverlayStrong
                    ctx.lineWidth = 1
                    ctx.setLineDash([4, 3])
                    ctx.strokeRect(bbox.minX, bbox.minY, bbox.maxX - bbox.minX, bbox.maxY - bbox.minY)
                    ctx.setLineDash([])

                    const handles = canvas.selectionHandles(bbox)

                    ctx.fillStyle = Theme.accent
                    ctx.strokeStyle = Theme.surface
                    ctx.lineWidth = 1
                    const corner = 4  // 4 corner handles are the first 4 entries
                    for (let hi = 0; hi < handles.resize.length; ++hi) {
                        const h = handles.resize[hi]
                        const sz = 5
                        if (hi < corner) {
                            // Filled square corner handle
                            ctx.fillRect(h.x - sz, h.y - sz, sz * 2, sz * 2)
                            ctx.strokeRect(h.x - sz, h.y - sz, sz * 2, sz * 2)
                        } else {
                            // Hollow square side/middle handle
                            ctx.fillStyle = Theme.surface
                            ctx.fillRect(h.x - sz, h.y - sz, sz * 2, sz * 2)
                            ctx.strokeRect(h.x - sz, h.y - sz, sz * 2, sz * 2)
                            ctx.fillStyle = Theme.accent
                        }
                    }

                    if (canvas.selectedImageId < 0) {
                        // Rotate handle: line connecting it to the top edge, plus a
                        // circular handle.
                        ctx.strokeStyle = Theme.selectionOverlayStrong
                        ctx.lineWidth = 1
                        ctx.beginPath()
                        ctx.moveTo(handles.rotate.x, bbox.minY)
                        ctx.lineTo(handles.rotate.x, handles.rotate.y)
                        ctx.stroke()

                        ctx.beginPath()
                        ctx.arc(handles.rotate.x, handles.rotate.y, 6, 0, Math.PI * 2)
                        ctx.fillStyle = Theme.accent
                        ctx.fill()
                        ctx.strokeStyle = Theme.surface
                        ctx.lineWidth = 1.5
                        ctx.stroke()
                    }
                }
            }

            if (canvas.sketch.overlayState.bondPreview) {
                let p1 = null
                if (canvas.sketch.overlayState.bondPreview.startAtomId !== null && canvas.sketch.overlayState.bondPreview.startAtomId !== undefined && canvas.sketch.primitives.atomsById[canvas.sketch.overlayState.bondPreview.startAtomId.toString()]) {
                    const startAtom = canvas.sketch.primitives.atomsById[canvas.sketch.overlayState.bondPreview.startAtomId.toString()]
                    p1 = canvas.chemToCanvas(startAtom.x, startAtom.y)
                } else if (canvas.sketch.overlayState.bondPreview.startX !== undefined) {
                    p1 = canvas.chemToCanvas(canvas.sketch.overlayState.bondPreview.startX, canvas.sketch.overlayState.bondPreview.startY)
                }
                
                if (p1 !== null) {
                    ctx.beginPath()
                    ctx.moveTo(p1.x, p1.y)
                    ctx.lineTo(canvas.sketch.overlayState.bondPreview.endX, canvas.sketch.overlayState.bondPreview.endY)
                    ctx.strokeStyle = Theme.accent
                    ctx.lineWidth = Theme.clampStrokeWidth(Theme.bondWidth, root.scale)
                    ctx.stroke()
                }
            }

            // Ring Template Ghost Preview
            if (canvas.sketch.overlayState.hoverRingSize) {
                const ringSize = canvas.sketch.overlayState.hoverRingSize;
                const cx = canvas.sketch.overlayState.mouseX || 0;
                const cy = canvas.sketch.overlayState.mouseY || 0;
                
                const chemPos = canvas.canvasToChem(cx, cy);
                const chemCx = chemPos.x;
                const chemCy = chemPos.y;
                
                const coords = canvas.sketch.getRingPreviewCoords(
                    ringSize, chemCx, chemCy, 
                    canvas.sketch.overlayState.hoverAtomId, 
                    canvas.sketch.overlayState.hoverBondId
                );
                
                if (coords && coords.length > 0) {
                    ctx.beginPath();
                    const firstPt = canvas.chemToCanvas(coords[0].x, coords[0].y);
                    ctx.moveTo(firstPt.x, firstPt.y);
                    for (let i = 1; i < coords.length; i++) {
                        const pt = canvas.chemToCanvas(coords[i].x, coords[i].y);
                        ctx.lineTo(pt.x, pt.y);
                    }
                    ctx.closePath();
                    
                    ctx.strokeStyle = Theme.accent;
                    ctx.lineWidth = Theme.clampStrokeWidth(Theme.bondWidth, root.scale);
                    ctx.setLineDash([4, 4]);
                    ctx.stroke();
                    ctx.setLineDash([]);
                }
            } else if (canvas.currentTool && canvas.currentTool.indexOf("TEMPLATE_") === 0 && !canvas.mouseArea.isDragging && !canvas.mouseArea.movingImage && !canvas.mouseArea.rotatingSelection && !canvas.mouseArea.resizingSelection && !canvas.sketch.overlayState.hoverAtomId && !canvas.sketch.overlayState.hoverBondId) {
                if (canvas.mouseArea.containsMouse) {
                    const cx = canvas.mouseArea.mouseX;
                    const cy = canvas.mouseArea.mouseY;
                    ctx.beginPath();
                    const r = Theme.clampDim(30, root.scale);
                    for (let i = 0; i < 6; i++) {
                        const angle = i * Math.PI / 3 + Math.PI / 6;
                        const px = cx + r * Math.cos(angle);
                        const py = cy + r * Math.sin(angle);
                        if (i === 0) ctx.moveTo(px, py);
                        else ctx.lineTo(px, py);
                    }
                    ctx.closePath();
                    
                    ctx.strokeStyle = Theme.accent;
                    ctx.lineWidth = Theme.clampStrokeWidth(Theme.bondWidth, root.scale);
                    ctx.setLineDash([4, 4]);
                    ctx.globalAlpha = 0.4;
                    ctx.stroke();
                    ctx.setLineDash([]);
                    ctx.globalAlpha = 1.0;
                }
            } else if (canvas.clipboardPreview && canvas.currentTool === "SELECT" && canvas.mouseArea.containsMouse && !canvas.mouseArea.isDragging && !canvas.mouseArea.movingImage && !canvas.mouseArea.rotatingSelection && !canvas.mouseArea.resizingSelection) {
                const mouseChem = canvas.canvasToChem(canvas.mouseArea.mouseX, canvas.mouseArea.mouseY);
                const clampedX = Math.max(-30, Math.min(30, mouseChem.x));
                const clampedY = Math.max(-21, Math.min(21, mouseChem.y));
                const dx = clampedX - canvas.clipboardPreview.cx;
                const dy = clampedY - canvas.clipboardPreview.cy;

                const preview = canvas.clipboardPreview;
                ctx.save();
                ctx.globalAlpha = 0.45;
                ctx.strokeStyle = Theme.accent;
                ctx.lineWidth = Theme.clampStrokeWidth(Theme.bondWidth, root.scale);
                ctx.setLineDash([4, 4]);

                if (preview.bonds) {
                    for (let i = 0; i < preview.bonds.length; i++) {
                        const b = preview.bonds[i];
                        const p1 = canvas.chemToCanvas(b.x1 + dx, b.y1 + dy);
                        const p2 = canvas.chemToCanvas(b.x2 + dx, b.y2 + dy);
                        ctx.beginPath();
                        ctx.moveTo(p1.x, p1.y);
                        ctx.lineTo(p2.x, p2.y);
                        ctx.stroke();
                    }
                }
                ctx.setLineDash([]);

                if (preview.atoms) {
                    ctx.fillStyle = Theme.accent;
                    const r = Theme.clampDim(3, root.scale);
                    for (let i = 0; i < preview.atoms.length; i++) {
                        const a = preview.atoms[i];
                        if (a.label && a.label !== "C") {
                            const p = canvas.chemToCanvas(a.x + dx, a.y + dy);
                            ctx.beginPath();
                            ctx.arc(p.x, p.y, r, 0, 2 * Math.PI);
                            ctx.fill();
                        }
                    }
                }
                ctx.restore();
            } else if (canvas.currentTool && (canvas.currentTool.indexOf("FG_") === 0 || canvas.currentTool.indexOf("SS_") === 0 || canvas.currentTool.indexOf("LIB_") === 0) &&
                       canvas.mouseArea.containsMouse && !canvas.mouseArea.isDragging && !canvas.mouseArea.movingImage &&
                       !canvas.mouseArea.rotatingSelection && !canvas.mouseArea.resizingSelection) {
                const cx = canvas.mouseArea.mouseX;
                const cy = canvas.mouseArea.mouseY;
                const chemPos = canvas.canvasToChem(cx, cy);
                const preview = canvas.sketch.getFunctionalGroupPreview(
                    canvas.currentTool, chemPos.x, chemPos.y, canvas.sketch.overlayState.hoverAtomId);

                ctx.strokeStyle = Theme.accent;
                ctx.lineWidth = Theme.clampStrokeWidth(Theme.bondWidth, root.scale);
                ctx.setLineDash([4, 4]);
                for (let i = 0; i < preview.length; i++) {
                    const entry = preview[i];
                    if (entry.isBond) {
                        const p1 = canvas.chemToCanvas(entry.startX, entry.startY);
                        const p2 = canvas.chemToCanvas(entry.endX, entry.endY);
                        ctx.beginPath();
                        ctx.moveTo(p1.x, p1.y);
                        ctx.lineTo(p2.x, p2.y);
                        ctx.stroke();
                    }
                }
                ctx.setLineDash([]);
            }

            if (canvas.sketch.overlayState.previewBonds) {
                for (let i = 0; i < canvas.sketch.overlayState.previewBonds.length; i++) {
                    const pb = canvas.sketch.overlayState.previewBonds[i]
                    const p1 = canvas.chemToCanvas(pb.startX, pb.startY)
                    const p2 = canvas.chemToCanvas(pb.endX, pb.endY)
                    ctx.beginPath()
                    ctx.moveTo(p1.x, p1.y)
                    ctx.lineTo(p2.x, p2.y)
                    ctx.strokeStyle = Theme.accent
                    ctx.lineWidth = Theme.clampStrokeWidth(Theme.bondWidth, root.scale)
                    ctx.stroke()
                }
            }

            if (canvas.sketch.overlayState.previewAtoms) {
                for (let i = 0; i < canvas.sketch.overlayState.previewAtoms.length; i++) {
                    const pa = canvas.sketch.overlayState.previewAtoms[i]
                    const p = canvas.chemToCanvas(pa.x, pa.y)
                    ctx.fillStyle = Theme.accent
                    ctx.font = "bold " + Theme.clampFontSize(Theme.baseFontSize, root.scale) + "px " + Theme.fontFamilyCss
                    ctx.textAlign = "center"
                    ctx.textBaseline = "middle"
                    if (pa.label) ctx.fillText(pa.label, p.x, p.y)
                }

                // Chain tool: floating bond-count label next to the drag endpoint, matching
                // the live carbon-count readout other chemistry editors show during a chain
                // drag. previewAtoms.length - 1 == the bond count (ChainPlacementEngine emits
                // nBonds+1 atoms).
                if (canvas.currentTool === "CHAIN" && canvas.sketch.overlayState.previewAtoms.length > 1) {
                    const lastAtom = canvas.sketch.overlayState.previewAtoms[canvas.sketch.overlayState.previewAtoms.length - 1]
                    const lp = canvas.chemToCanvas(lastAtom.x, lastAtom.y)
                    ctx.fillStyle = Theme.textSecondary
                    ctx.font = Theme.clampFontSize(Theme.fontSizeCaption, root.scale) + "px " + Theme.fontFamilyCss
                    ctx.textAlign = "left"
                    ctx.textBaseline = "middle"
                    ctx.fillText(String(canvas.sketch.overlayState.previewAtoms.length - 1), lp.x + 8, lp.y - 8)
                }
            }
        }
    }

    // Atom info tooltip
    Item {
        id: atomTooltip

        property var hovAtom: {
            const ov = canvas.sketch.overlayState
            if (!ov || ov.hoverAtomId === null || ov.hoverAtomId === undefined) return null
            const prims = canvas.sketch.primitives
            if (!prims || !prims.atomsById) return null
            return prims.atomsById[ov.hoverAtomId.toString()] || null
        }

        visible: hovAtom !== null && hovAtom !== undefined

        property point _pos: {
            if (!hovAtom || !canvas) return Qt.point(0, 0)
            const p = canvas.chemToCanvas(hovAtom.x, hovAtom.y)
            const xOff = 18
            const yOff = -44
            const tx = Math.min(p.x + xOff, root.width  - tooltipBox.width  - 6)
            const ty = Math.max(p.y + yOff, 6)
            return Qt.point(tx, ty)
        }

        x: _pos.x
        y: _pos.y

        Rectangle {
            id: tooltipBox
            width: tooltipCol.implicitWidth + 14
            height: tooltipCol.implicitHeight + 10
            radius: 5
            color: Theme.surface
            border.color: Theme.outline
            border.width: 1

            layer.enabled: true
            layer.effect: null

            Column {
                id: tooltipCol
                anchors { left: parent.left; top: parent.top; margins: 7 }
                spacing: 1

                Row {
                    spacing: 6
                    Text {
                        text: {
                            if (!atomTooltip.hovAtom) return ""
                            return atomTooltip.hovAtom.isSgroup ? atomTooltip.hovAtom.label : atomTooltip.hovAtom.element
                        }
                        font { pixelSize: 18; bold: true; family: Theme.fontDisplay }
                        color: atomTooltip.hovAtom ? (atomTooltip.hovAtom.color || Theme.textPrimary) : Theme.textPrimary
                    }
                    Text {
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 3
                        visible: atomTooltip.hovAtom && !atomTooltip.hovAtom.isSgroup && atomTooltip.hovAtom.atomicNum > 0
                        text: atomTooltip.hovAtom && atomTooltip.hovAtom.atomicNum > 0
                              ? "#" + atomTooltip.hovAtom.atomicNum : ""
                        font { pixelSize: Theme.fontSizeLabel; family: Theme.fontMono }
                        color: Theme.textSecondary
                    }
                }

                Text {
                    visible: atomTooltip.hovAtom && !atomTooltip.hovAtom.isSgroup && atomTooltip.hovAtom.atomicTitle !== ""
                    // isSgroup pill primitives (src/v8_worker.js) have no atomicTitle field at
                    // all — bindings are evaluated even while invisible, so this needs the same
                    // isSgroup guard as `visible` above, not just a hovAtom truthiness check.
                    text: (atomTooltip.hovAtom && !atomTooltip.hovAtom.isSgroup) ? atomTooltip.hovAtom.atomicTitle : ""
                    font { pixelSize: Theme.fontSizeLabel; family: Theme.fontDisplay }
                    color: Theme.textSecondary
                }

                Text {
                    visible: atomTooltip.hovAtom && !atomTooltip.hovAtom.isSgroup && atomTooltip.hovAtom.atomicMass > 0
                    text: atomTooltip.hovAtom && atomTooltip.hovAtom.atomicMass > 0
                          ? atomTooltip.hovAtom.atomicMass + " u" : ""
                    font { pixelSize: Theme.fontSizeLabel; family: Theme.fontMono }
                    color: Theme.textSecondary
                }

                Text {
                    visible: atomTooltip.hovAtom && !atomTooltip.hovAtom.isSgroup && atomTooltip.hovAtom.charge !== 0
                    text: {
                        if (!atomTooltip.hovAtom || atomTooltip.hovAtom.charge === 0) return ""
                        const c = atomTooltip.hovAtom.charge
                        if (c === 1) return "charge: +"
                        if (c === -1) return "charge: −"
                        return "charge: " + (c > 0 ? "+" + c : c)
                    }
                    font { pixelSize: Theme.fontSizeLabel; family: Theme.fontMono }
                    color: Theme.textSecondary
                }
            }
        }
    }

    Connections {
        target: root
        function onOverlayVersionChanged() { overlayCanvas.requestPaint() }
        function onRenderVersionChanged() { overlayCanvas.requestPaint() }
        function onScaleChanged() { overlayCanvas.requestPaint() }
    }

    Connections {
        target: canvas ? canvas.sketch : null
        function onOverlayStateChanged() { overlayCanvas.requestPaint() }
        function onSelectionChanged() { overlayCanvas.requestPaint() }
    }

    Connections {
        target: canvas
        function onCurrentToolChanged() { overlayCanvas.requestPaint() }
        function onClipboardPreviewChanged() { overlayCanvas.requestPaint() }
    }

    Connections {
        target: canvas ? canvas.mouseArea : null
        function onMouseXChanged() {
            if (canvas && ((canvas.currentTool && (canvas.currentTool.indexOf("TEMPLATE_") === 0 || canvas.currentTool.indexOf("FG_") === 0 || canvas.currentTool.indexOf("SS_") === 0 || canvas.currentTool.indexOf("LIB_") === 0)) || (canvas.clipboardPreview && canvas.currentTool === "SELECT"))) {
                overlayCanvas.requestPaint()
            }
        }
        function onMouseYChanged() {
            if (canvas && ((canvas.currentTool && (canvas.currentTool.indexOf("TEMPLATE_") === 0 || canvas.currentTool.indexOf("FG_") === 0 || canvas.currentTool.indexOf("SS_") === 0 || canvas.currentTool.indexOf("LIB_") === 0)) || (canvas.clipboardPreview && canvas.currentTool === "SELECT"))) {
                overlayCanvas.requestPaint()
            }
        }
    }
}
