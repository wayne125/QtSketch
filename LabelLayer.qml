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

    // ctx.measureText is re-run for every label component on every repaint; cache by
    // (font, text) since results don't change unless the font string does (scale/style).
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
        id: labelCanvas
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

            if (!canvas) return
            if (!canvas.sketch.primitives || !canvas.sketch.primitives.atoms) return

            const fontSize = Theme.clampFontSize(Theme.baseFontSize, root.scale)
            const superSize = Math.max(7, 10 * root.scale)
            const subSize   = Math.max(7, 10 * root.scale)

            for (let i = 0; i < canvas.sketch.primitives.atoms.length; i++) {
                const a = canvas.sketch.primitives.atoms[i]

                // ── Contracted sgroup node (FG / S&S abbreviation pill) ──────
                if (a.isSgroup) {
                    const p = canvas.chemToCanvas(a.x, a.y)
                    const isSelected = canvas.sketch.selection.atom_ids && canvas.sketch.selection.atom_ids.indexOf(a.id) >= 0
                    ctx.font = "bold " + fontSize + "px " + Theme.fontFamilyCss
                    const tw = root.measureCached(ctx, a.label)
                    const padX = 5 * root.scale
                    const padY = 3 * root.scale
                    const pillW = tw + padX * 2
                    const pillH = fontSize + padY * 2
                    const rx = pillH / 2
                    // Background pill
                    ctx.fillStyle = isSelected ? Theme.selected : Theme.background
                    ctx.strokeStyle = isSelected ? Theme.accent : Theme.textPrimary
                    ctx.lineWidth = Theme.clampStrokeWidth(1.2, root.scale)
                    ctx.beginPath()
                    drawRoundedRect(ctx, p.x - pillW / 2, p.y - pillH / 2, pillW, pillH, rx)
                    ctx.fill()
                    ctx.stroke()
                    // Label text
                    ctx.fillStyle = isSelected ? Theme.accent : Theme.textPrimary
                    ctx.textAlign = "center"
                    ctx.textBaseline = "middle"
                    ctx.fillText(a.label, p.x, p.y)
                    continue
                }
                // ────────────────────────────────────────────────────────────

                const hasIsotope  = a.isotope && a.isotope > 0
                const hasRadical  = a.radical && a.radical > 0
                const hasValence  = a.explicitValence !== undefined && a.explicitValence >= 0
                const hasH        = (a.implicitHCount || 0) > 0
                const hasAttachment = a.attachmentPoints && a.attachmentPoints > 0

                // Skip unlabelled carbons
                if (a.element === "C" && a.charge === 0 && !a.stereoLabel &&
                        !canvas.showExplicitH && !hasIsotope && !hasRadical && !hasValence && !hasAttachment) {
                    if (!hasH) continue
                    // Terminal carbons with showExplicitH off still show no label if renderLabel === "C"
                    if (a.label === "C") continue
                }

                const p = canvas.chemToCanvas(a.x, a.y)
                const isSelected = canvas.sketch.selection.atom_ids && canvas.sketch.selection.atom_ids.indexOf(a.id) >= 0

                const baseElement  = a.element
                const implicitH    = a.implicitHCount || 0
                const charge       = a.charge || 0
                const stereoLabel  = a.stereoLabel || ""
                const cipLabel     = a.cipLabel || ""
                const aam          = a.aam || 0
                const checkWarning = a.checkWarning || ""
                const hOnLeft      = a.hOnLeft || false

                // ── Measure components ──────────────────────────────────────
                ctx.font = "bold " + fontSize + "px " + Theme.fontFamilyCss
                const elemW = root.measureCached(ctx, baseElement)

                let hText = "", hSubText = ""
                if (implicitH > 0) {
                    hText = "H"
                    if (implicitH > 1) hSubText = implicitH.toString()
                }
                ctx.font = "bold " + fontSize + "px " + Theme.fontFamilyCss
                const hW = hText ? root.measureCached(ctx, hText) : 0
                ctx.font = "bold " + subSize + "px " + Theme.fontFamilyCss
                const hSubW = hSubText ? root.measureCached(ctx, hSubText) : 0

                let chargeText = ""
                if (charge !== 0) {
                    if      (charge ===  1) chargeText = "+"
                    else if (charge === -1) chargeText = "–"
                    else if (charge >   1)  chargeText = charge + "+"
                    else                    chargeText = Math.abs(charge) + "–"
                }
                ctx.font = "bold " + superSize + "px " + Theme.fontFamilyCss
                const chargeW = chargeText ? root.measureCached(ctx, chargeText) : 0

                // Attachment point badge ("*1", "*2" etc.)
                let apText = ""
                if (hasAttachment) {
                    apText = "*" + a.attachmentPoints
                }
                ctx.font = "bold " + superSize + "px " + Theme.fontFamilyCss
                const apW = apText ? root.measureCached(ctx, apText) : 0

                // Isotope prefix ("13" for ¹³C)
                const isoText = hasIsotope ? a.isotope.toString() : ""
                ctx.font = "bold " + superSize + "px " + Theme.fontFamilyCss
                const isoW = isoText ? root.measureCached(ctx, isoText) : 0

                // Explicit valence suffix ("(4)" etc.)
                const valText = hasValence ? ("(" + a.explicitValence + ")") : ""
                ctx.font = "bold " + subSize + "px " + Theme.fontFamilyCss
                const valW = valText ? root.measureCached(ctx, valText) : 0

                let totalWidth = isoW + elemW + hW + hSubW + chargeW + valW + apW
                let stereoW = 0
                if (stereoLabel) {
                    ctx.font = "bold " + subSize + "px " + Theme.fontFamilyCss
                    stereoW = root.measureCached(ctx, " " + stereoLabel)
                    totalWidth += stereoW
                }
                let cipW = 0
                if (cipLabel) {
                    ctx.font = "bold " + subSize + "px " + Theme.fontFamilyCss
                    cipW = root.measureCached(ctx, " " + cipLabel)
                    totalWidth += cipW
                }

                // ── Background pill ──────────────────────────────────────────
                const padX = 4 * root.scale
                const padY = 4 * root.scale
                const pillW = totalWidth + padX * 2
                const pillH = fontSize * 1.1 + padY * 2

                ctx.fillStyle = Theme.surface
                ctx.beginPath()
                ctx.ellipse(p.x, p.y, pillW / 2, pillH / 2, 0, 0, Math.PI * 2)
                ctx.fill()

                const colorStr = a.color || Theme.getElementColor(a.element)
                const textColor = isSelected ? Theme.accent : colorStr

                // ── Draw label left-to-right ─────────────────────────────────
                let cx = p.x - totalWidth / 2

                // Isotope superscript before element
                if (isoText) {
                    ctx.font = "bold " + superSize + "px " + Theme.fontFamilyCss
                    ctx.fillStyle = textColor
                    ctx.textAlign = "left"
                    ctx.textBaseline = "middle"
                    ctx.fillText(isoText, cx, p.y - fontSize * Theme.superscriptOffset)
                    cx += isoW
                }

                if (hOnLeft && hText) {
                    // H on left side
                    ctx.font = "bold " + fontSize + "px " + Theme.fontFamilyCss
                    ctx.fillStyle = textColor
                    ctx.textAlign = "left"
                    ctx.textBaseline = "middle"
                    ctx.fillText(hText, cx, p.y)
                    cx += hW
                    if (hSubText) {
                        ctx.font = "bold " + subSize + "px " + Theme.fontFamilyCss
                        ctx.fillText(hSubText, cx, p.y + fontSize * Theme.subscriptOffset)
                        cx += hSubW
                    }
                    ctx.font = "bold " + fontSize + "px " + Theme.fontFamilyCss
                    ctx.fillText(baseElement, cx, p.y)
                    cx += elemW
                } else {
                    // Element first, H on right
                    ctx.font = "bold " + fontSize + "px " + Theme.fontFamilyCss
                    ctx.fillStyle = textColor
                    ctx.textAlign = "left"
                    ctx.textBaseline = "middle"
                    ctx.fillText(baseElement, cx, p.y)
                    cx += elemW
                    if (hText) {
                        ctx.fillText(hText, cx, p.y)
                        cx += hW
                        if (hSubText) {
                            ctx.font = "bold " + subSize + "px " + Theme.fontFamilyCss
                            ctx.fillText(hSubText, cx, p.y + fontSize * Theme.subscriptOffset)
                            cx += hSubW
                        }
                    }
                }

                // Charge superscript
                if (chargeText) {
                    ctx.font = "bold " + superSize + "px " + Theme.fontFamilyCss
                    ctx.fillStyle = textColor
                    ctx.fillText(chargeText, cx, p.y - fontSize * Theme.superscriptOffset)
                    cx += chargeW
                }

                // Attachment point superscript
                if (apText) {
                    ctx.font = "bold " + superSize + "px " + Theme.fontFamilyCss
                    ctx.fillStyle = textColor
                    ctx.fillText(apText, cx, p.y - fontSize * Theme.superscriptOffset)
                    cx += apW
                }

                // Explicit valence subscript
                if (valText) {
                    ctx.font = "bold " + subSize + "px " + Theme.fontFamilyCss
                    ctx.fillStyle = isSelected ? Theme.accent : Theme.textSecondary
                    ctx.fillText(valText, cx, p.y + fontSize * Theme.subscriptOffset)
                    cx += valW
                }

                // Stereo label
                if (stereoLabel) {
                    ctx.font = "bold " + subSize + "px " + Theme.fontFamilyCss
                    ctx.fillStyle = isSelected ? Theme.accent : Theme.textSecondary
                    ctx.fillText(" " + stereoLabel, cx, p.y)
                    cx += stereoW
                }

                // CIP descriptor (R/S/E/Z)
                if (cipLabel) {
                    ctx.font = "bold " + subSize + "px " + Theme.fontFamilyCss
                    ctx.fillStyle = Theme.accent
                    ctx.fillText(" " + cipLabel, cx, p.y)
                    cx += cipW
                }

                // ── Radical dots above pill ──────────────────────────────────
                if (hasRadical) {
                    const dotR  = Math.max(2, 2.5 * root.scale)
                    const dotY  = p.y - pillH / 2 - dotR - 1 * root.scale
                    ctx.fillStyle = textColor
                    if (a.radical === 1) {
                        // Monoradical: single dot
                        ctx.beginPath()
                        ctx.arc(p.x, dotY, dotR, 0, Math.PI * 2)
                        ctx.fill()
                    } else if (a.radical === 2) {
                        // Singlet diradical: two dots close together (paired)
                        const sep = dotR * 1.8
                        ctx.beginPath()
                        ctx.arc(p.x - sep / 2, dotY, dotR, 0, Math.PI * 2)
                        ctx.fill()
                        ctx.beginPath()
                        ctx.arc(p.x + sep / 2, dotY, dotR, 0, Math.PI * 2)
                        ctx.fill()
                        // Small line connecting them to signal pairing
                        ctx.strokeStyle = textColor
                        ctx.lineWidth = root.scale
                        ctx.beginPath()
                        ctx.moveTo(p.x - sep / 2 + dotR, dotY)
                        ctx.lineTo(p.x + sep / 2 - dotR, dotY)
                        ctx.stroke()
                    } else {
                        // Triplet diradical: two spaced dots
                        const sep = dotR * 4
                        ctx.beginPath()
                        ctx.arc(p.x - sep / 2, dotY, dotR, 0, Math.PI * 2)
                        ctx.fill()
                        ctx.beginPath()
                        ctx.arc(p.x + sep / 2, dotY, dotR, 0, Math.PI * 2)
                        ctx.fill()
                    }
                }

                // AAM (atom-atom mapping) number badge — small orange circle above pill
                if (aam > 0) {
                    const badgeR = Math.max(6, 8 * root.scale)
                    const badgeY = p.y - pillH / 2 - badgeR - 1 * root.scale
                    ctx.fillStyle = Theme.badgeAam
                    ctx.beginPath()
                    ctx.arc(p.x + pillW / 2 + badgeR, badgeY, badgeR, 0, Math.PI * 2)
                    ctx.fill()
                    ctx.fillStyle = Theme.badgeText
                    ctx.font = "bold " + Math.max(7, 9 * root.scale) + "px " + Theme.fontFamilyCss
                    ctx.textAlign = "center"
                    ctx.textBaseline = "middle"
                    ctx.fillText(aam.toString(), p.x + pillW / 2 + badgeR, badgeY)
                }

                // Check warning badge — small red circle with "!" to the upper-left of pill
                if (checkWarning) {
                    const warnR = Math.max(5, 7 * root.scale)
                    const warnY = p.y - pillH / 2 - warnR - 1 * root.scale
                    ctx.fillStyle = Theme.error
                    ctx.beginPath()
                    ctx.arc(p.x - pillW / 2 - warnR, warnY, warnR, 0, Math.PI * 2)
                    ctx.fill()
                    ctx.fillStyle = Theme.badgeText
                    ctx.font = "bold " + Math.max(8, 10 * root.scale) + "px " + Theme.fontFamilyCss
                    ctx.textAlign = "center"
                    ctx.textBaseline = "middle"
                    ctx.fillText("!", p.x - pillW / 2 - warnR, warnY)
                }
            }

            // ── Text annotations ─────────────────────────────────────────────
            if (canvas.sketch.primitives.texts) {
                const txFont = Theme.clampFontSize(Theme.baseFontSize, root.scale)
                ctx.font = txFont + "px " + Theme.fontFamilyCss
                ctx.textAlign = "left"
                ctx.textBaseline = "top"
                ctx.fillStyle = Theme.textPrimary
                for (let ti = 0; ti < canvas.sketch.primitives.texts.length; ti++) {
                    const t = canvas.sketch.primitives.texts[ti]
                    const tp = canvas.chemToCanvas(t.x, t.y)
                    const txLines = String(t.content).split("\n")
                    for (let li = 0; li < txLines.length; li++) {
                        ctx.fillText(txLines[li], tp.x, tp.y + li * txFont * 1.25)
                    }
                }
            }

            // Images
            if (canvas.sketch.primitives.images) {
                for (let ii = 0; ii < canvas.sketch.primitives.images.length; ii++) {
                    const imgNode = canvas.sketch.primitives.images[ii]
                    const tp = canvas.chemToCanvas(imgNode.x, imgNode.y)
                    const pixelW = imgNode.w * canvas.chemScale
                    const pixelH = imgNode.h * canvas.chemScale
                    const imgItem = imageRepeater.itemAt(ii)
                    if (imgItem && imgItem.status === Image.Ready) {
                        ctx.drawImage(imgItem, tp.x, tp.y, pixelW, pixelH)
                    }
                }
            }
        }

        Connections {
            target: root
            function onRenderVersionChanged() { labelCanvas.requestPaint() }
            function onScaleChanged()         { labelCanvas.requestPaint() }
            function onOffsetXChanged()        { if (!canvas || !canvas._panningActive) labelCanvas.requestPaint() }
            function onOffsetYChanged()        { if (!canvas || !canvas._panningActive) labelCanvas.requestPaint() }
        }
    }

    Repeater {
        id: imageRepeater
        model: canvas && canvas.sketch && canvas.sketch.primitives ? (canvas.sketch.primitives.images || []) : []
        Image {
            visible: false
            source: modelData.bitmap
            onStatusChanged: {
                if (status === Image.Ready)
                    labelCanvas.requestPaint()
                else if (status === Image.Error)
                    console.warn("Failed to load embedded image:", modelData.bitmap ? modelData.bitmap.substring(0, 40) : "")
            }
        }
    }
}
