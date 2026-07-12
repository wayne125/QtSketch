import QtQuick

Item {
    id: root
    anchors.fill: parent

    property real scale: 1.0
    property real offsetX: 0
    property real offsetY: 0
    property real bondLength: 40
    property Item canvas
    property int renderVersion: 0

    // Shared with LabelLayer's label-retraction measurements: caches ctx.measureText
    // by (font, text) since results don't change unless the font string does.
    property var _measureCache: ({})
    function measureCached(ctx, text) {
        const key = ctx.font + "|" + text
        var v = _measureCache[key]
        if (v === undefined) {
            if (Object.keys(_measureCache).length > 5000) _measureCache = ({})
            v = ctx.measureText(text).width
            _measureCache[key] = v
        }
        return v
    }

    Canvas {
        id: paintCanvas
        anchors.fill: parent
        // Item.antialiasing defaults to false; Context2D uses it as the QPainter
        // antialiasing hint, so without it all strokes render aliased.
        antialiasing: true

        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        
        onPaint: {
            const ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            if (!canvas) return

            // Page/canvas boundary: a soft visual reference, drawn beneath
            // everything else. The actual hard clamp lives in the worker
            // (src/v8_worker.js's PAGE_MIN_X/MAX_X/MIN_Y/MAX_Y).
            if (canvas.pageBounds) {
                const pb = canvas.pageBounds
                const tl = canvas.chemToCanvas(pb.x, pb.y)
                const br = canvas.chemToCanvas(pb.x + pb.width, pb.y + pb.height)
                ctx.save()
                ctx.strokeStyle = Theme.textSecondary
                ctx.globalAlpha = 0.35
                ctx.lineWidth = 1
                ctx.setLineDash([6, 4])
                ctx.strokeRect(tl.x, tl.y, br.x - tl.x, br.y - tl.y)
                ctx.restore()
            }

            if (!canvas.sketch.primitives || !canvas.sketch.primitives.bonds || !canvas.sketch.primitives.atoms) return

            // Helper: check if an atom has a visible label (needs bond retraction)
            function atomHasLabel(a) {
                if (!a) return false
                if (a.isSgroup) return true
                if (a.element !== "C") return true
                if (a.charge !== 0) return true
                if (a.stereoLabel && a.stereoLabel !== "") return true
                if (a.cipLabel && a.cipLabel !== "") return true
                if (a.aam && a.aam > 0) return true
                if (a.checkWarning && a.checkWarning !== "") return true
                if (canvas.showExplicitH) return true
                if (a.isotope && a.isotope > 0) return true
                if (a.radical && a.radical > 0) return true
                if (a.explicitValence !== undefined && a.explicitValence >= 0) return true
                if (a.attachmentPoints && a.attachmentPoints > 0) return true
                return false
            }

            // Helper: measure label width for retraction distance
            function getLabelRetraction(a, fontSize) {
                if (!atomHasLabel(a)) return 0
                ctx.font = "bold " + fontSize + "px " + Theme.fontFamilyCss
                if (a.isSgroup) {
                    return (root.measureCached(ctx, a.label) / 2) + 6 * root.scale
                }
                let textStr = a.label
                if (a.isotope && a.isotope > 0) textStr = a.isotope.toString() + textStr
                let retract = (root.measureCached(ctx, textStr) / 2) + 2 * root.scale
                // Account for stereo label and CIP label (subscript-sized)
                const subSize = Math.max(7, 10 * root.scale)
                ctx.font = "bold " + subSize + "px " + Theme.fontFamilyCss
                if (a.stereoLabel) retract += root.measureCached(ctx, " " + a.stereoLabel) / 2
                if (a.cipLabel) retract += root.measureCached(ctx, " " + a.cipLabel) / 2
                return retract
            }

            const fontSize = Theme.clampFontSize(Theme.baseFontSize, root.scale)

            // 1. Draw Bonds
            ctx.lineCap = "round"
            ctx.lineJoin = "round"
            for (let i = 0; i < canvas.sketch.primitives.bonds.length; i++) {
                const b = canvas.sketch.primitives.bonds[i]
                const a1 = canvas.sketch.primitives.atomsById[b.begin.toString()]
                const a2 = canvas.sketch.primitives.atomsById[b.end.toString()]
                if (!a1 || !a2) continue

                let p1 = canvas.chemToCanvas(a1.x, a1.y)
                let p2 = canvas.chemToCanvas(a2.x, a2.y)
                
                const isSelected = canvas.sketch.selection.bond_ids && canvas.sketch.selection.bond_ids.indexOf(b.id) >= 0
                ctx.strokeStyle = isSelected ? Theme.accent : (b.invalidStereo || b.checkWarning ? Theme.error : Theme.textPrimary)

                // --- Bond Retraction ---
                // Shorten bond endpoints if atom has a visible label
                const dx_full = p2.x - p1.x
                const dy_full = p2.y - p1.y
                const bondLen = Math.sqrt(dx_full * dx_full + dy_full * dy_full)
                
                if (bondLen > 0) {
                    const ux = dx_full / bondLen
                    const uy = dy_full / bondLen
                    
                    const retract1 = getLabelRetraction(a1, fontSize)
                    const retract2 = getLabelRetraction(a2, fontSize)
                    
                    if (retract1 > 0) {
                        p1 = { x: p1.x + ux * retract1, y: p1.y + uy * retract1 }
                    }
                    if (retract2 > 0) {
                        p2 = { x: p2.x - ux * retract2, y: p2.y - uy * retract2 }
                    }
                }

                if (b.type === 1) {
                    if (b.stereo === 1) {
                        // UP / Wedge
                        const dx = p2.x - p1.x
                        const dy = p2.y - p1.y
                        const len = Math.sqrt(dx * dx + dy * dy)
                        const w = Theme.wedgeTaperRatio * len
                        const nx = len > 0 ? (dy / len) * w : 0
                        const ny = len > 0 ? (-dx / len) * w : 0
                        ctx.fillStyle = ctx.strokeStyle
                        ctx.beginPath()
                        ctx.moveTo(p1.x, p1.y)
                        ctx.lineTo(p2.x + nx, p2.y + ny)
                        ctx.lineTo(p2.x - nx, p2.y - ny)
                        ctx.closePath()
                        ctx.fill()
                    } else if (b.stereo === 6) {
                        // DOWN / Dash (Hashed Wedge)
                        const dx = p2.x - p1.x
                        const dy = p2.y - p1.y
                        const len = Math.sqrt(dx * dx + dy * dy)
                        const maxW = Theme.wedgeTaperRatio * len
                        const numHashes = Math.max(Theme.hashMinSize, Math.floor(len / Theme.clampDim(Theme.hashSpacingFactor, root.scale)))
                        ctx.lineWidth = Theme.clampStrokeWidth(1.0, root.scale)
                        ctx.beginPath()
                        for (let j = 1; j <= numHashes; j++) {
                            let t = j / numHashes
                            let cx = p1.x + dx * t
                            let cy = p1.y + dy * t
                            let hw = t * maxW
                            let hnx = len > 0 ? (dy / len) * hw : 0
                            let hny = len > 0 ? (-dx / len) * hw : 0
                            ctx.moveTo(cx + hnx, cy + hny)
                            ctx.lineTo(cx - hnx, cy - hny)
                        }
                        ctx.stroke()
                    } else if (b.stereo === 4 || b.stereo === 3) {
                        // Wavy / UP_DOWN — smooth quadratic curves
                        ctx.lineWidth = Theme.clampStrokeWidth(Theme.bondWidth, root.scale)
                        ctx.beginPath()
                        ctx.moveTo(p1.x, p1.y)
                        const dx = p2.x - p1.x
                        const dy = p2.y - p1.y
                        const len = Math.sqrt(dx * dx + dy * dy)
                        const numWaves = Math.max(2, Math.floor(len / Theme.clampDim(Theme.wavyWaveSpacing, root.scale)))
                        const amp = Theme.clampDim(Theme.wavyAmplitude, root.scale)
                        const nx = len > 0 ? (dy / len) * amp : 0
                        const ny = len > 0 ? (-dx / len) * amp : 0
                        const seg = 1 / numWaves
                        for (let j = 0; j < numWaves; j++) {
                            const t0 = j * seg
                            const t1 = (j + 1) * seg
                            const tMid = (t0 + t1) / 2
                            const sign = (j % 2 === 0) ? 1 : -1
                            const cpx = p1.x + dx * tMid + nx * sign
                            const cpy = p1.y + dy * tMid + ny * sign
                            const ex = p1.x + dx * t1
                            const ey = p1.y + dy * t1
                            ctx.quadraticCurveTo(cpx, cpy, ex, ey)
                        }
                        ctx.lineTo(p2.x, p2.y)
                        ctx.stroke()
                    } else {
                        // Regular single bond
                        ctx.lineWidth = Theme.clampStrokeWidth(Theme.bondWidth, root.scale)
                        ctx.beginPath()
                        ctx.moveTo(p1.x, p1.y)
                        ctx.lineTo(p2.x, p2.y)
                        ctx.stroke()
                    }
                } else {
                    // --- Ring-Aware Double Bond Offset ---
                    let nx, ny
                    const dxb = p2.x - p1.x
                    const dyb = p2.y - p1.y
                    const lenb = Math.sqrt(dxb * dxb + dyb * dyb)
                    const spacing = Theme.clampDim(Theme.doubleBondSpacing, root.scale)

                    if (b.ringCenterX !== undefined && b.ringCenterY !== undefined) {
                        // Bond is in a ring: offset toward the ring center
                        const ringC = canvas.chemToCanvas(b.ringCenterX, b.ringCenterY)
                        const midX = (p1.x + p2.x) / 2
                        const midY = (p1.y + p2.y) / 2
                        // Perpendicular to bond
                        const perpX = lenb > 0 ? -(dyb / lenb) : 0
                        const perpY = lenb > 0 ? (dxb / lenb) : 0
                        // Determine which side the ring center is on
                        const toRingX = ringC.x - midX
                        const toRingY = ringC.y - midY
                        const dot = toRingX * perpX + toRingY * perpY
                        const sign = dot >= 0 ? 1 : -1
                        nx = perpX * spacing * sign
                        ny = perpY * spacing * sign
                    } else {
                        // Not in a ring: default perpendicular offset
                        const perpLen = lenb > 0 ? lenb : 1
                        nx = lenb > 0 ? (-(dyb) / perpLen) * spacing : 0
                        ny = lenb > 0 ? ((dxb) / perpLen) * spacing : 0
                    }

                    if (b.type === 2) {
                        ctx.lineWidth = Theme.clampStrokeWidth(Theme.bondWidth, root.scale)
                        // Main bond line
                        ctx.beginPath()
                        ctx.moveTo(p1.x, p1.y)
                        ctx.lineTo(p2.x, p2.y)
                        ctx.stroke()

                        // Offset second line (shortened by 15% on each end for both ring and exocyclic)
                        ctx.strokeStyle = isSelected ? Theme.accent : Theme.textPrimary
                        ctx.beginPath()
                        const shrink = 0.15
                        ctx.moveTo(p1.x + nx + dxb * shrink, p1.y + ny + dyb * shrink)
                        ctx.lineTo(p2.x + nx - dxb * shrink, p2.y + ny - dyb * shrink)
                        ctx.stroke()
                    } else if (b.type === 3) {
                        ctx.lineWidth = Theme.clampStrokeWidth(Theme.bondWidth, root.scale)
                        ctx.beginPath()
                        ctx.moveTo(p1.x + nx, p1.y + ny)
                        ctx.lineTo(p2.x + nx, p2.y + ny)
                        ctx.moveTo(p1.x - nx, p1.y - ny)
                        ctx.lineTo(p2.x - nx, p2.y - ny)
                        ctx.stroke()

                        ctx.strokeStyle = isSelected ? Theme.accent : Theme.textPrimary
                        ctx.beginPath()
                        ctx.moveTo(p1.x, p1.y)
                        ctx.lineTo(p2.x, p2.y)
                        ctx.stroke()
                    } else if (b.type === 4) {
                        // Aromatic bond order has no Kekule single/double structure to
                        // depict — a plain line here; the ring-level pass below draws
                        // one inscribed circle per aromatic ring instead of dashing
                        // every bond individually.
                        ctx.lineWidth = Theme.clampStrokeWidth(Theme.bondWidth, root.scale)
                        ctx.beginPath()
                        ctx.moveTo(p1.x, p1.y)
                        ctx.lineTo(p2.x, p2.y)
                        ctx.stroke()
                    }
                }

                // E/Z CIP descriptor, offset perpendicular from the bond midpoint
                if (b.cipLabel) {
                    const midX = (p1.x + p2.x) / 2
                    const midY = (p1.y + p2.y) / 2
                    const ddx = p2.x - p1.x
                    const ddy = p2.y - p1.y
                    const dlen = Math.sqrt(ddx * ddx + ddy * ddy)
                    const perpX = dlen > 0 ? -(ddy / dlen) : 0
                    const perpY = dlen > 0 ? (ddx / dlen) : 0
                    const cipFontSize = Math.max(9, 11 * root.scale)
                    const cipOffset = 10 * root.scale
                    ctx.font = "bold " + cipFontSize + "px " + Theme.fontFamilyCss
                    ctx.textAlign = "center"
                    ctx.textBaseline = "middle"
                    ctx.fillStyle = Theme.accent
                    ctx.fillText(b.cipLabel, midX + perpX * cipOffset, midY + perpY * cipOffset)
                }
            }

            // 1b. Aromatic ring circles — one per ring with true aromatic bond order
            // (hasBondType4), inscribed at the worker-computed ring center/radius.
            if (canvas.sketch.primitives.rings) {
                ctx.strokeStyle = Theme.textPrimary
                ctx.lineWidth = Theme.clampStrokeWidth(Theme.bondWidth * 0.7, root.scale)
                for (let ri = 0; ri < canvas.sketch.primitives.rings.length; ri++) {
                    const ring = canvas.sketch.primitives.rings[ri]
                    if (!ring.hasBondType4) continue
                    const rc = canvas.chemToCanvas(ring.x, ring.y)
                    const rPix = ring.radius * root.scale * root.bondLength
                    ctx.beginPath()
                    ctx.arc(rc.x, rc.y, rPix * 0.65, 0, Math.PI * 2)
                    ctx.stroke()
                }
            }

            // 2. Draw Reaction Arrows (mode-dispatched)
            if (canvas.sketch.primitives.rxnArrows) {
                // Helper: filled triangular arrowhead at (tipX,tipY) pointing along `angle`
                function drawFilledHead(tipX, tipY, angle, headLen, headHalfAngle) {
                    ctx.beginPath()
                    ctx.moveTo(tipX, tipY)
                    ctx.lineTo(tipX - headLen * Math.cos(angle - headHalfAngle), tipY - headLen * Math.sin(angle - headHalfAngle))
                    ctx.lineTo(tipX - headLen * Math.cos(angle + headHalfAngle), tipY - headLen * Math.sin(angle + headHalfAngle))
                    ctx.closePath()
                    ctx.fill()
                }
                // Helper: open V arrowhead at (tipX,tipY) pointing along `angle`
                function drawOpenHead(tipX, tipY, angle, headLen, headHalfAngle) {
                    ctx.beginPath()
                    ctx.moveTo(tipX - headLen * Math.cos(angle - headHalfAngle), tipY - headLen * Math.sin(angle - headHalfAngle))
                    ctx.lineTo(tipX, tipY)
                    ctx.lineTo(tipX - headLen * Math.cos(angle + headHalfAngle), tipY - headLen * Math.sin(angle + headHalfAngle))
                    ctx.stroke()
                }
                // Helper: draw centered horizontal text at (x,y) with given font size
                function drawCondText(text, x, y, fontSize) {
                    if (!text || text.length === 0) return
                    ctx.font = "italic " + fontSize + "px " + Theme.fontDisplayCss
                    ctx.textAlign = "center"
                    ctx.textBaseline = "middle"
                    ctx.fillStyle = Theme.textPrimary
                    ctx.fillText(text, x, y)
                }

                for (let i = 0; i < canvas.sketch.primitives.rxnArrows.length; ++i) {
                    const ar = canvas.sketch.primitives.rxnArrows[i]
                    const a1 = canvas.chemToCanvas(ar.p1.x, ar.p1.y)
                    const a2 = canvas.chemToCanvas(ar.p2.x, ar.p2.y)
                    const angle = Math.atan2(a2.y - a1.y, a2.x - a1.x)
                    const headLen = Theme.clampDim(Theme.arrowHeadLength, root.scale)
                    const headW = Theme.clampDim(Theme.arrowHeadWidth, root.scale)
                    const shaftW = Theme.clampStrokeWidth(Theme.bondWidth, root.scale)
                    const headHalfAngle = 0.3
                    const mode = ar.mode || "filled-triangle"

                    ctx.strokeStyle = Theme.textPrimary
                    ctx.fillStyle = Theme.textPrimary
                    ctx.lineWidth = shaftW
                    ctx.lineCap = "round"

                    if (mode === "curved-mechanism" && ar.curvature) {
                        // Curved quadratic shaft + open arrowhead at end
                        const ctrl = canvas.chemToCanvas(ar.curvature.x, ar.curvature.y)
                        ctx.beginPath()
                        ctx.moveTo(a1.x, a1.y)
                        ctx.quadraticCurveTo(ctrl.x, ctrl.y, a2.x, a2.y)
                        ctx.stroke()
                        // Tangent angle at end ≈ direction from control to end
                        const endAngle = Math.atan2(a2.y - ctrl.y, a2.x - ctrl.x)
                        ctx.lineWidth = shaftW
                        drawOpenHead(a2.x, a2.y, endAngle, headLen, headHalfAngle)
                    } else if (mode === "equilibrium-FF") {
                        // Two filled half-heads at both ends, shortened shaft
                        ctx.beginPath()
                        ctx.moveTo(a1.x + headLen * 0.6 * Math.cos(angle), a1.y + headLen * 0.6 * Math.sin(angle))
                        ctx.lineTo(a2.x - headLen * 0.6 * Math.cos(angle), a2.y - headLen * 0.6 * Math.sin(angle))
                        ctx.stroke()
                        drawFilledHead(a1.x, a1.y, angle + Math.PI, headLen * 0.8, headHalfAngle)
                        drawFilledHead(a2.x, a2.y, angle, headLen * 0.8, headHalfAngle)
                    } else if (mode === "equilibrium-FH") {
                        // Filled head at end, open half-head at start
                        ctx.beginPath()
                        ctx.moveTo(a1.x + headLen * 0.6 * Math.cos(angle), a1.y + headLen * 0.6 * Math.sin(angle))
                        ctx.lineTo(a2.x - headLen * 0.6 * Math.cos(angle), a2.y - headLen * 0.6 * Math.sin(angle))
                        ctx.stroke()
                        drawOpenHead(a1.x, a1.y, angle + Math.PI, headLen * 0.8, headHalfAngle)
                        drawFilledHead(a2.x, a2.y, angle, headLen, headHalfAngle)
                    } else if (mode === "retrosynthetic") {
                        // Filled triangle at end + crossbar near tail
                        ctx.beginPath()
                        ctx.moveTo(a1.x, a1.y)
                        ctx.lineTo(a2.x - headLen * Math.cos(angle), a2.y - headLen * Math.sin(angle))
                        ctx.stroke()
                        drawFilledHead(a2.x, a2.y, angle, headLen, headHalfAngle)
                        // Crossbar perpendicular to shaft, ~25% from tail
                        const barT = 0.25
                        const bx = a1.x + (a2.x - a1.x) * barT
                        const by = a1.y + (a2.y - a1.y) * barT
                        const barHalf = headW * 1.5
                        ctx.beginPath()
                        ctx.moveTo(bx + barHalf * Math.cos(angle + Math.PI / 2), by + barHalf * Math.sin(angle + Math.PI / 2))
                        ctx.lineTo(bx - barHalf * Math.cos(angle + Math.PI / 2), by - barHalf * Math.sin(angle + Math.PI / 2))
                        ctx.stroke()
                    } else if (mode === "open-angle") {
                        // Open V arrowhead at end (no fill)
                        ctx.beginPath()
                        ctx.moveTo(a1.x, a1.y)
                        ctx.lineTo(a2.x - headLen * 0.6 * Math.cos(angle), a2.y - headLen * 0.6 * Math.sin(angle))
                        ctx.stroke()
                        drawOpenHead(a2.x, a2.y, angle, headLen, headHalfAngle)
                    } else {
                        // "filled-triangle" (default) — filled triangular arrowhead
                        ctx.beginPath()
                        ctx.moveTo(a1.x, a1.y)
                        ctx.lineTo(a2.x - headLen * Math.cos(angle), a2.y - headLen * Math.sin(angle))
                        ctx.stroke()
                        drawFilledHead(a2.x, a2.y, angle, headLen, headHalfAngle)
                    }

                    // Conditions text above/below the shaft midpoint
                    if (ar.conditionsText && (ar.conditionsText.above || ar.conditionsText.below)) {
                        const midX = (a1.x + a2.x) / 2
                        const midY = (a1.y + a2.y) / 2
                        const perpX = Math.sin(angle)
                        const perpY = -Math.cos(angle)
                        const condFontSize = Math.max(9, 11 * root.scale)
                        const condOffset = (headW + 6) * root.scale
                        if (ar.conditionsText.above) {
                            drawCondText(ar.conditionsText.above, midX + perpX * condOffset, midY + perpY * condOffset, condFontSize)
                        }
                        if (ar.conditionsText.below) {
                            drawCondText(ar.conditionsText.below, midX - perpX * condOffset, midY - perpY * condOffset, condFontSize)
                        }
                    }
                }
            }

            // 5. Draw Multi-tail Branching Arrows
            if (canvas.sketch.primitives.multitailArrows) {
                for (let i = 0; i < canvas.sketch.primitives.multitailArrows.length; ++i) {
                    const mta = canvas.sketch.primitives.multitailArrows[i]
                    const spineTop = canvas.chemToCanvas(mta.spineTopX, mta.spineTopY)
                    const spineBot = canvas.chemToCanvas(mta.spineTopX, mta.spineTopY + mta.height)
                    const isSelected = canvas.sketch.selection.multitailArrow_ids && canvas.sketch.selection.multitailArrow_ids.indexOf(mta.id) >= 0
                    const color = isSelected ? Theme.accent : Theme.textPrimary
                    ctx.strokeStyle = color
                    ctx.fillStyle = color
                    ctx.lineWidth = Theme.clampStrokeWidth(Theme.bondWidth, root.scale)
                    ctx.lineCap = "round"

                    // Draw spine (vertical line)
                    ctx.beginPath()
                    ctx.moveTo(spineTop.x, spineTop.y)
                    ctx.lineTo(spineBot.x, spineBot.y)
                    ctx.stroke()

                    // Draw tails (horizontal lines)
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

                    // Draw arrow head from spine top to head offset
                    if (mta.headOffsetX !== 0 || mta.headOffsetY !== 0) {
                        const headTip = canvas.chemToCanvas(mta.spineTopX + mta.headOffsetX, mta.spineTopY + mta.headOffsetY)
                        const angle = Math.atan2(headTip.y - spineTop.y, headTip.x - spineTop.x)
                        const headLen = Theme.clampDim(Theme.arrowHeadLength, root.scale)
                        const headHalfAngle = 0.3

                        // Shaft from spine top to just before head tip
                        ctx.beginPath()
                        ctx.moveTo(spineTop.x, spineTop.y)
                        ctx.lineTo(headTip.x - headLen * Math.cos(angle), headTip.y - headLen * Math.sin(angle))
                        ctx.stroke()

                        // Filled triangle arrowhead
                        ctx.beginPath()
                        ctx.moveTo(headTip.x, headTip.y)
                        ctx.lineTo(headTip.x - headLen * Math.cos(angle - headHalfAngle), headTip.y - headLen * Math.sin(angle - headHalfAngle))
                        ctx.lineTo(headTip.x - headLen * Math.cos(angle + headHalfAngle), headTip.y - headLen * Math.sin(angle + headHalfAngle))
                        ctx.closePath()
                        ctx.fill()
                    }
                }
            }
        }
    }

    function repaint() {
        paintCanvas.requestPaint()
    }

    Timer {
        id: panCoalesce
        interval: 16
        repeat: false
        onTriggered: paintCanvas.requestPaint()
    }

    Connections {
        target: root
        function onScaleChanged() { paintCanvas.requestPaint() }
        function onOffsetXChanged() { panCoalesce.restart() }
        function onOffsetYChanged() { panCoalesce.restart() }
        function onRenderVersionChanged() { paintCanvas.requestPaint() }
    }
}
