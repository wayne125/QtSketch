.pragma library

// Ported from ToolPanel.qml's fgDelegate.onPaint (the superset of the three
// forked thumbnail-paint bodies: ssDelegate/libDelegate drew a simplified
// subset, fgDelegate additionally handled wedge stereo and double/triple
// bonds). thumb is {atoms:[{x,y,label}], bonds:[{x1,y1,x2,y2,type,stereo}]}
// with coordinates normalized to [0,1] over the thumbnail's own bounding box.
function paint(ctx, width, height, thumb, strokeColor) {
    ctx.clearRect(0, 0, width, height)
    if (!thumb || !thumb.bonds) return

    ctx.strokeStyle = strokeColor
    ctx.fillStyle = strokeColor
    ctx.lineWidth = 1.2
    ctx.lineCap = "round"

    for (var i = 0; i < thumb.bonds.length; i++) {
        var b = thumb.bonds[i]
        var px1 = b.x1 * width, py1 = b.y1 * height
        var px2 = b.x2 * width, py2 = b.y2 * height
        var dx = px2 - px1, dy = py2 - py1
        var len = Math.sqrt(dx * dx + dy * dy)
        if (len < 0.5) continue
        var nx = -dy / len, ny = dx / len
        var btype = b.type || 1

        if (btype === 1) {
            if (b.stereo === 1) {
                ctx.beginPath()
                ctx.moveTo(px1, py1)
                ctx.lineTo(px2 + nx * 2, py2 + ny * 2)
                ctx.lineTo(px2 - nx * 2, py2 - ny * 2)
                ctx.closePath()
                ctx.fill()
            } else {
                ctx.beginPath()
                ctx.moveTo(px1, py1)
                ctx.lineTo(px2, py2)
                ctx.stroke()
            }
        } else if (btype === 2) {
            var off = 1.5
            ctx.beginPath()
            ctx.moveTo(px1, py1)
            ctx.lineTo(px2, py2)
            ctx.stroke()
            ctx.beginPath()
            ctx.moveTo(px1 + nx * off, py1 + ny * off)
            ctx.lineTo(px2 + nx * off, py2 + ny * off)
            ctx.stroke()
        } else if (btype === 3) {
            var off3 = 2
            ctx.beginPath()
            ctx.moveTo(px1, py1)
            ctx.lineTo(px2, py2)
            ctx.stroke()
            ctx.beginPath()
            ctx.moveTo(px1 + nx * off3, py1 + ny * off3)
            ctx.lineTo(px2 + nx * off3, py2 + ny * off3)
            ctx.stroke()
            ctx.beginPath()
            ctx.moveTo(px1 - nx * off3, py1 - ny * off3)
            ctx.lineTo(px2 - nx * off3, py2 - ny * off3)
            ctx.stroke()
        } else {
            ctx.beginPath()
            ctx.moveTo(px1, py1)
            ctx.lineTo(px2, py2)
            ctx.stroke()
        }
    }

    if (thumb.atoms) {
        for (var j = 0; j < thumb.atoms.length; j++) {
            var a = thumb.atoms[j]
            if (a.label && a.label !== "C") {
                if (a.color) {
                    ctx.fillStyle = a.color
                    ctx.font = "7px sans-serif"
                    ctx.textAlign = "center"
                    ctx.textBaseline = "middle"
                    ctx.fillText(a.label, a.x * width, a.y * height)
                } else {
                    // JS-worker-engine thumbnails carry no color field -- keep the
                    // exact original monochrome-dot behavior, unchanged.
                    ctx.beginPath()
                    ctx.arc(a.x * width, a.y * height, 1.5, 0, Math.PI * 2)
                    ctx.fill()
                }
            }
        }
    }
}
