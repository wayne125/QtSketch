import QtQuick

Item {
    id: root

    property real scale: 1.0
    property real offsetX: 0
    property real offsetY: 0
    property real bondLength: 40
    property Item canvas: null
    property int renderVersion: 0
    property int overlayVersion: 0

    anchors.fill: parent

    function hitTestAtom(cx, cy) {
        const atoms = canvas.sketch.primitives.atoms || []
        const radius = Math.max(Theme.selectionWidth, Theme.hoverWidth * scale)
        const r2 = radius * radius
        const fontSize = Math.max(12, 16 * scale)
        const padX = 5 * scale
        const padY = 3 * scale
        for (let i = 0; i < atoms.length; ++i) {
            const a = atoms[i]
            const p = canvas.chemToCanvas(a.x, a.y)
            const dx = p.x - cx
            const dy = p.y - cy
            if (a.isSgroup && a.label) {
                // Pill hit test: approximate text width from char count
                const approxTextW = a.label.length * fontSize * 0.65
                const halfW = approxTextW / 2 + padX + 2 * scale
                const halfH = (fontSize + padY * 2) / 2 + 2 * scale
                if (Math.abs(dx) <= halfW && Math.abs(dy) <= halfH)
                    return a.id
            } else {
                if (dx * dx + dy * dy < r2)
                    return a.id
            }
        }
        return null
    }

    function pointLineDistanceSq(px, py, ax, ay, bx, by) {
        const l2 = (ax - bx) * (ax - bx) + (ay - by) * (ay - by)
        if (l2 === 0) return (px - ax) * (px - ax) + (py - ay) * (py - ay)
        let t = ((px - ax) * (bx - ax) + (py - ay) * (by - ay)) / l2
        t = Math.max(0, Math.min(1, t))
        const projX = ax + t * (bx - ax)
        const projY = ay + t * (by - ay)
        const dx = px - projX
        const dy = py - projY
        return dx * dx + dy * dy
    }

    function hitTestBond(cx, cy) {
        const bonds = canvas.sketch.primitives.bonds || []
        const threshold = Math.max(8, Theme.atomRadius * scale)
        const t2 = threshold * threshold
        for (let i = 0; i < bonds.length; ++i) {
            const b = bonds[i]
            const beginStr = b.begin.toString()
            const endStr = b.end.toString()
            if (canvas.sketch.primitives.atomsById && canvas.sketch.primitives.atomsById[beginStr] && canvas.sketch.primitives.atomsById[endStr]) {
                const a1 = canvas.sketch.primitives.atomsById[beginStr]
                const a2 = canvas.sketch.primitives.atomsById[endStr]
                const p1 = canvas.chemToCanvas(a1.x, a1.y)
                const p2 = canvas.chemToCanvas(a2.x, a2.y)
                
                // Simple BBox check
                const minX = Math.min(p1.x, p2.x) - threshold
                const maxX = Math.max(p1.x, p2.x) + threshold
                const minY = Math.min(p1.y, p2.y) - threshold
                const maxY = Math.max(p1.y, p2.y) + threshold
                
                if (cx >= minX && cx <= maxX && cy >= minY && cy <= maxY) {
                    if (pointLineDistanceSq(cx, cy, p1.x, p1.y, p2.x, p2.y) < t2) {
                        return b.id
                    }
                }
            }
        }
        return null
    }

    function hitTestRxnArrow(cx, cy) {
        const arrows = canvas.sketch.primitives.rxnArrows || []
        const threshold = Math.max(8, Theme.atomRadius * scale)
        const t2 = threshold * threshold
        for (let i = 0; i < arrows.length; ++i) {
            const ar = arrows[i]
            const p1 = canvas.chemToCanvas(ar.p1.x, ar.p1.y)
            const p2 = canvas.chemToCanvas(ar.p2.x, ar.p2.y)
            if (pointLineDistanceSq(cx, cy, p1.x, p1.y, p2.x, p2.y) < t2) {
                return ar.id
            }
        }
        return null
    }

    function hitTestRxnPlus(cx, cy) {
        const pluses = canvas.sketch.primitives.rxnPluses || []
        const radius = Math.max(Theme.selectionWidth, Theme.hoverWidth * scale * 1.5)
        const r2 = radius * radius
        for (let i = 0; i < pluses.length; ++i) {
            const pl = pluses[i]
            const pp = canvas.chemToCanvas(pl.x, pl.y)
            const dx = pp.x - cx
            const dy = pp.y - cy
            if (dx * dx + dy * dy < r2)
                return pl.id
        }
        return null
    }

    function hitTestMultitailArrow(cx, cy) {
        const arrows = canvas.sketch.primitives.multitailArrows || []
        const threshold = Math.max(8, Theme.atomRadius * scale)
        const t2 = threshold * threshold
        for (let i = 0; i < arrows.length; ++i) {
            const mta = arrows[i]
            const spineTop = canvas.chemToCanvas(mta.spineTopX, mta.spineTopY)
            const spineBot = canvas.chemToCanvas(mta.spineTopX, mta.spineTopY + mta.height)
            // Hit test spine
            if (pointLineDistanceSq(cx, cy, spineTop.x, spineTop.y, spineBot.x, spineBot.y) < t2)
                return mta.id
            // Hit test tails
            if (mta.tails) {
                for (let j = 0; j < mta.tails.length; ++j) {
                    const tail = mta.tails[j]
                    const tailStart = canvas.chemToCanvas(tail.x, tail.y)
                    const tailEnd = canvas.chemToCanvas(mta.spineTopX, tail.y)
                    if (pointLineDistanceSq(cx, cy, tailStart.x, tailStart.y, tailEnd.x, tailEnd.y) < t2)
                        return mta.id
                }
            }
        }
        return null
    }

    Canvas {
        id: overlayCanvas
        anchors.fill: parent
        antialiasing: true

        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()

        function drawRoundedRect(ctx, x, y, w, h, r) {
            ctx.moveTo(x + r, y)
            ctx.lineTo(x + w - r, y)
            ctx.arcTo(x + w, y, x + w, y + r, r)
            ctx.lineTo(x + w, y + h - r)
            ctx.arcTo(x + w, y + h, x + w - r, y + h, r)
            ctx.lineTo(x + r, y + h)
            ctx.arcTo(x, y + h, x, y + h - r, r)
            ctx.lineTo(x, y + r)
            ctx.arcTo(x, y, x + r, y, r)
        }

        onPaint: {


            const ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            // Selection highlight circles/pills (atoms + sgroup nodes)
            const selAtoms = canvas.sketch.selection.atom_ids || []
            for (let i = 0; i < selAtoms.length; ++i) {
                const aidStr = selAtoms[i].toString()
                if (canvas.sketch.primitives.atomsById && canvas.sketch.primitives.atomsById[aidStr]) {
                    const sa = canvas.sketch.primitives.atomsById[aidStr]
                    const sp = canvas.chemToCanvas(sa.x, sa.y)
                    ctx.fillStyle = Theme.selectionOverlay
                    if (sa.isSgroup) {
                        // Pill outline matching LabelLayer pill size
                        const fontSize = Math.max(8, Theme.baseFontSize * root.scale)
                        ctx.font = "bold " + fontSize + "px " + Theme.fontFamilyCss
                        const tw = ctx.measureText(sa.label).width
                        const padX = 5 * root.scale
                        const padY = 3 * root.scale
                        const pillW = tw + padX * 2
                        const pillH = fontSize + padY * 2
                        ctx.beginPath()
                        drawRoundedRect(ctx, sp.x - pillW / 2, sp.y - pillH / 2, pillW, pillH, pillH / 2)
                        ctx.fill()
                    } else {
                        ctx.beginPath()
                        ctx.arc(sp.x, sp.y, Theme.atomRadius * root.scale, 0, 2*Math.PI)
                        ctx.fill()
                    }
                }
            }

            // Selection highlight lines (bonds)
            const selBonds = canvas.sketch.selection.bond_ids || []
            if (selBonds.length > 0 && canvas.sketch.primitives.bonds) {
                for (let k = 0; k < canvas.sketch.primitives.bonds.length; ++k) {
                    const sb = canvas.sketch.primitives.bonds[k]
                    const sbBeginStr = sb.begin.toString()
                    const sbEndStr = sb.end.toString()
                    if (selBonds.indexOf(sb.id) >= 0 && canvas.sketch.primitives.atomsById && canvas.sketch.primitives.atomsById[sbBeginStr] && canvas.sketch.primitives.atomsById[sbEndStr]) {
                        const a1 = canvas.sketch.primitives.atomsById[sbBeginStr]
                        const a2 = canvas.sketch.primitives.atomsById[sbEndStr]
                        const p1 = canvas.chemToCanvas(a1.x, a1.y)
                        const p2 = canvas.chemToCanvas(a2.x, a2.y)
                        ctx.beginPath()
                        ctx.moveTo(p1.x, p1.y)
                        ctx.lineTo(p2.x, p2.y)
                        ctx.strokeStyle = Theme.selectionOverlay
                        ctx.lineWidth = Theme.clampStrokeWidth(Theme.selectionWidth, root.scale)
                        ctx.lineCap = "round"
                        ctx.stroke()
                    }
                }
            }

            // Selection highlight for rxnArrows
            const selRxnArrows = canvas.sketch.selection.rxnArrow_ids || []
            if (selRxnArrows.length > 0 && canvas.sketch.primitives.rxnArrows) {
                for (let k = 0; k < canvas.sketch.primitives.rxnArrows.length; ++k) {
                    const ar = canvas.sketch.primitives.rxnArrows[k]
                    if (selRxnArrows.indexOf(ar.id) >= 0) {
                        const p1 = canvas.chemToCanvas(ar.p1.x, ar.p1.y)
                        const p2 = canvas.chemToCanvas(ar.p2.x, ar.p2.y)
                        ctx.strokeStyle = Theme.selectionOverlay
                        ctx.lineWidth = Theme.clampStrokeWidth(Theme.selectionWidth, root.scale)
                        ctx.beginPath()
                        ctx.moveTo(p1.x, p1.y)
                        ctx.lineTo(p2.x, p2.y)
                        ctx.stroke()
                    }
                }
            }

            // Selection highlight for rxnPluses
            const selRxnPluses = canvas.sketch.selection.rxnPlus_ids || []
            if (selRxnPluses.length > 0 && canvas.sketch.primitives.rxnPluses) {
                for (let k = 0; k < canvas.sketch.primitives.rxnPluses.length; ++k) {
                    const pl = canvas.sketch.primitives.rxnPluses[k]
                    if (selRxnPluses.indexOf(pl.id) >= 0) {
                        const pp = canvas.chemToCanvas(pl.x, pl.y)
                        const sz = Theme.clampDim(8, root.scale)
                        ctx.strokeStyle = Theme.selectionOverlay
                        ctx.lineWidth = Theme.clampStrokeWidth(Theme.selectionWidth, root.scale)
                        ctx.lineCap = "round"
                        ctx.beginPath()
                        ctx.moveTo(pp.x - sz, pp.y)
                        ctx.lineTo(pp.x + sz, pp.y)
                        ctx.moveTo(pp.x, pp.y - sz)
                        ctx.lineTo(pp.x, pp.y + sz)
                        ctx.stroke()
                    }
                }
            }

            // Selection highlight for multitailArrows
            const selMultitailArrows = canvas.sketch.selection.multitailArrow_ids || []
            if (selMultitailArrows.length > 0 && canvas.sketch.primitives.multitailArrows) {
                for (let k = 0; k < canvas.sketch.primitives.multitailArrows.length; ++k) {
                    const mta = canvas.sketch.primitives.multitailArrows[k]
                    if (selMultitailArrows.indexOf(mta.id) >= 0) {
                        const spineTop = canvas.chemToCanvas(mta.spineTopX, mta.spineTopY)
                        const spineBot = canvas.chemToCanvas(mta.spineTopX, mta.spineTopY + mta.height)
                        ctx.strokeStyle = Theme.selectionOverlay
                        ctx.lineWidth = Theme.clampStrokeWidth(Theme.selectionWidth, root.scale)
                        ctx.lineCap = "round"
                        // Spine
                        ctx.beginPath()
                        ctx.moveTo(spineTop.x, spineTop.y)
                        ctx.lineTo(spineBot.x, spineBot.y)
                        ctx.stroke()
                        // Tails
                        if (mta.tails) {
                            for (let j = 0; j < mta.tails.length; ++j) {
                                const tail = mta.tails[j]
                                const tailStart = canvas.chemToCanvas(tail.x, tail.y)
                                const tailEnd = canvas.chemToCanvas(mta.spineTopX, tail.y)
                                ctx.beginPath()
                                ctx.moveTo(tailStart.x, tailStart.y)
                                ctx.lineTo(tailEnd.x, tailEnd.y)
                                ctx.stroke()
                            }
                        }
                    }
                }
            }

        }

        Connections {
            target: root
            function onOverlayVersionChanged() { overlayCanvas.requestPaint() }
            function onRenderVersionChanged() { overlayCanvas.requestPaint() }
            function onScaleChanged() { overlayCanvas.requestPaint() }
            function onOffsetXChanged() { if (!canvas || !canvas._panningActive) overlayCanvas.requestPaint() }
            function onOffsetYChanged() { if (!canvas || !canvas._panningActive) overlayCanvas.requestPaint() }
        }
    }
}
