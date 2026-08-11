function addAtom(label, x, y, charge) {
    if (charge !== undefined && charge !== null) charge = Math.max(-3, Math.min(3, Math.round(charge)))
    var clamped = _clampToPage(x, y)
    var p = new CoreLib.ChemCore.Vec2(clamped.x, clamped.y)
    var atomId = null
    var cmd = makeCmd(
        function() {
            if (atomId === null) {
                atomId = _struct.atoms.add(new CoreLib.ChemCore.Atom({
                    label: label || "C", charge: charge || 0, pp: p
                }))
            } else {
                _struct.atoms.set(atomId, new CoreLib.ChemCore.Atom({
                    label: label || "C", charge: charge || 0, pp: p
                }))
            }
        },
        function() { _struct.atoms.delete(atomId) }
    )
    executeCommand(cmd)
    return atomId
}

function addBond(beginId, endId, type, stereo) {
    if (_struct.checkBondExists(beginId, endId)) return
    
    var bondId = null
    var cmd = makeCmd(
        function() {
            if (bondId === null) {
                bondId = _struct.bonds.add(new CoreLib.ChemCore.Bond({
                    type: type || 1, stereo: stereo || 0, begin: beginId, end: endId
                }))
            } else {
                _struct.bonds.set(bondId, new CoreLib.ChemCore.Bond({
                    type: type || 1, stereo: stereo || 0, begin: beginId, end: endId
                }))
            }
            _struct.initHalfBonds()
            _struct.initNeighbors()
            _struct.updateHalfBonds()
            _struct.sortNeighbors()
        },
        function() {
            _struct.bonds.delete(bondId)
            _struct.initHalfBonds()
            _struct.initNeighbors()
            _struct.updateHalfBonds()
            _struct.sortNeighbors()
        }
    )
    executeCommand(cmd)
    return bondId
}

// Expand selection atom IDs: sgroup prim IDs → all atoms inside that sgroup
function _resolveAtomMoveIds(aids) {
    var result = [], seen = {}
    aids.forEach(function(aid) {
        if (_struct.atoms.get(aid)) {
            if (!seen[aid]) { seen[aid] = true; result.push(aid) }
        } else if (_struct.sgroups) {
            var sg = _struct.sgroups.get(aid)
            if (sg && sg.atoms) {
                sg.atoms.forEach(function(realAid) {
                    if (!seen[realAid]) { seen[realAid] = true; result.push(realAid) }
                })
            }
        }
    })
    return result
}

function moveSelection(dx, dy) {
    if (_selection.atom_ids.length === 0 && _selection.rxnArrow_ids.length === 0 && _selection.rxnPlus_ids.length === 0 && (!_selection.multitailArrow_ids || _selection.multitailArrow_ids.length === 0)) return
    if (_dragDelta.x === 0 && _dragDelta.y === 0) {
        _dragSelection = _resolveAtomMoveIds(_selection.atom_ids)
        _dragArrowSelection = _selection.rxnArrow_ids ? _selection.rxnArrow_ids.slice() : []
        _dragPlusSelection = _selection.rxnPlus_ids ? _selection.rxnPlus_ids.slice() : []
        _dragMultitailSelection = _selection.multitailArrow_ids ? _selection.multitailArrow_ids.slice() : []
        _dragOrigBBox = null
        if (_dragSelection.length > 0) {
            var _bMinX = Infinity, _bMinY = Infinity, _bMaxX = -Infinity, _bMaxY = -Infinity
            _dragSelection.forEach(function(aid) {
                var a = _struct.atoms.get(aid)
                if (a) {
                    if (a.pp.x < _bMinX) _bMinX = a.pp.x
                    if (a.pp.x > _bMaxX) _bMaxX = a.pp.x
                    if (a.pp.y < _bMinY) _bMinY = a.pp.y
                    if (a.pp.y > _bMaxY) _bMaxY = a.pp.y
                }
            })
            if (_bMinX <= _bMaxX) _dragOrigBBox = { minX: _bMinX, minY: _bMinY, maxX: _bMaxX, maxY: _bMaxY }
        }
    }
    // Page-boundary hard clamp: cap the cumulative delta against the
    // selection's ORIGINAL bbox (captured above) so the whole selection stops
    // together at the edge, like dragging a window against a screen edge,
    // rather than each atom clamping independently and flattening the shape.
    // If the selection itself is wider/taller than the page, leave that axis
    // unclamped rather than fighting it into an impossible state.
    var newDeltaX = _dragDelta.x + dx
    var newDeltaY = _dragDelta.y + dy
    if (_dragOrigBBox) {
        var _b = _dragOrigBBox
        var minDx = PAGE_MIN_X - _b.minX, maxDx = PAGE_MAX_X - _b.maxX
        var minDy = PAGE_MIN_Y - _b.minY, maxDy = PAGE_MAX_Y - _b.maxY
        if (minDx <= maxDx) newDeltaX = Math.max(minDx, Math.min(maxDx, newDeltaX))
        if (minDy <= maxDy) newDeltaY = Math.max(minDy, Math.min(maxDy, newDeltaY))
    }
    var appliedDx = newDeltaX - _dragDelta.x
    var appliedDy = newDeltaY - _dragDelta.y
    _dragDelta.x = newDeltaX
    _dragDelta.y = newDeltaY

    _dragSelection.forEach(function(aid) {
        var a = _struct.atoms.get(aid)
        if (a) a.pp.add_(new CoreLib.ChemCore.Vec2(appliedDx, appliedDy))
    })
    if (_selection.rxnArrow_ids) {
        _selection.rxnArrow_ids.forEach(function(arId) {
            var ar = _struct.rxnArrows.get(arId)
            if (ar) {
                if (ar.pos) {
                    ar.pos.forEach(function(p) { p.add_(new CoreLib.ChemCore.Vec2(appliedDx, appliedDy)) })
                } else {
                    if (ar.p1) ar.p1.add_(new CoreLib.ChemCore.Vec2(appliedDx, appliedDy))
                    if (ar.p2) ar.p2.add_(new CoreLib.ChemCore.Vec2(appliedDx, appliedDy))
                }
            }
        })
    }
    if (_selection.rxnPlus_ids) {
        _selection.rxnPlus_ids.forEach(function(plId) {
            var pl = _struct.rxnPluses.get(plId)
            if (pl) {
                if (pl.pp) pl.pp.add_(new CoreLib.ChemCore.Vec2(appliedDx, appliedDy))
                else if (pl.x !== undefined) { pl.x += appliedDx; pl.y += appliedDy }
            }
        })
    }
    if (_selection.multitailArrow_ids) {
        _selection.multitailArrow_ids.forEach(function(mtaId) {
            var mta = _struct.multitailArrows.get(mtaId)
            if (mta) {
                mta.spineTopX += appliedDx
                mta.spineTopY += appliedDy
            }
        })
    }
    _dirty = true
}

function commitMove() {
    if (_dragDelta.x !== 0 || _dragDelta.y !== 0) {
        var dx = _dragDelta.x
        var dy = _dragDelta.y
        var aids = _dragSelection.slice()
        var arIds = (_dragArrowSelection || []).slice()
        var plIds = (_dragPlusSelection || []).slice()
        var mtaIds = (_dragMultitailSelection || []).slice()
        var isFirst = true
        function applyDelta(sign) {
            aids.forEach(function(aid) {
                var a = _struct.atoms.get(aid)
                if (a) a.pp.add_(new CoreLib.ChemCore.Vec2(dx * sign, dy * sign))
            })
            arIds.forEach(function(arId) {
                var ar = _struct.rxnArrows.get(arId)
                if (ar) {
                    if (ar.pos) {
                        ar.pos.forEach(function(p) { p.add_(new CoreLib.ChemCore.Vec2(dx * sign, dy * sign)) })
                    } else {
                        if (ar.p1) ar.p1.add_(new CoreLib.ChemCore.Vec2(dx * sign, dy * sign))
                        if (ar.p2) ar.p2.add_(new CoreLib.ChemCore.Vec2(dx * sign, dy * sign))
                    }
                }
            })
            plIds.forEach(function(plId) {
                var pl = _struct.rxnPluses.get(plId)
                if (pl) {
                    if (pl.pp) pl.pp.add_(new CoreLib.ChemCore.Vec2(dx * sign, dy * sign))
                    else if (pl.x !== undefined) { pl.x += dx * sign; pl.y += dy * sign }
                }
            })
            mtaIds.forEach(function(mtaId) {
                var mta = _struct.multitailArrows.get(mtaId)
                if (mta) {
                    mta.spineTopX += dx * sign
                    mta.spineTopY += dy * sign
                }
            })
        }
        var cmd = makeCmd(
            function() { if (isFirst) { isFirst = false; return } applyDelta(1) },
            function() { isFirst = false; applyDelta(-1) }
        )
        executeCommand(cmd)
        _dragDelta = { x: 0, y: 0 }
        _dragSelection = []
        _dragArrowSelection = []
        _dragPlusSelection = []
        _dragMultitailSelection = []
        _dragOrigBBox = null
    }
}

// ---- ROTATE tool: live drag rotation around the selection centroid --------
// Mirrors moveSelection/commitMove's shape: repeated non-undoable live calls
// during the drag, snapshotting original positions once so repeated re-
// application from the same snapshot avoids floating-point drift, then a
// single undoable commit wrapping the whole gesture.
// Shared by rotate/scale: every selected item's position(s), across all four
// selection id lists (mirrors moveSelection's own multi-type handling), as a
// flat point list with enough tag info to write each one back afterward.
function _snapshotSelectionPoints() {
    var pts = []
    _resolveAtomMoveIds(_selection.atom_ids).forEach(function(aid) {
        var a = _struct.atoms.get(aid)
        if (a) pts.push({ kind: 'atom', id: aid, x: a.pp.x, y: a.pp.y })
    })
    if (_selection.rxnArrow_ids) {
        _selection.rxnArrow_ids.forEach(function(arId) {
            var ar = _struct.rxnArrows.get(arId)
            if (!ar) return
            if (ar.pos) {
                ar.pos.forEach(function(p, idx) { pts.push({ kind: 'rxnArrowPos', id: arId, idx: idx, x: p.x, y: p.y }) })
            } else {
                if (ar.p1) pts.push({ kind: 'rxnArrowP1', id: arId, x: ar.p1.x, y: ar.p1.y })
                if (ar.p2) pts.push({ kind: 'rxnArrowP2', id: arId, x: ar.p2.x, y: ar.p2.y })
            }
        })
    }
    if (_selection.rxnPlus_ids) {
        _selection.rxnPlus_ids.forEach(function(plId) {
            var pl = _struct.rxnPluses.get(plId)
            if (!pl) return
            if (pl.pp) pts.push({ kind: 'rxnPlusPP', id: plId, x: pl.pp.x, y: pl.pp.y })
            else if (pl.x !== undefined) pts.push({ kind: 'rxnPlusXY', id: plId, x: pl.x, y: pl.y })
        })
    }
    if (_selection.multitailArrow_ids) {
        _selection.multitailArrow_ids.forEach(function(mtaId) {
            var mta = _struct.multitailArrows.get(mtaId)
            if (mta) pts.push({ kind: 'multitail', id: mtaId, x: mta.spineTopX, y: mta.spineTopY })
        })
    }
    return pts
}

function _writeSelectionPoint(pt, nx, ny) {
    if (pt.kind === 'atom') { var a = _struct.atoms.get(pt.id); if (a) { a.pp.x = nx; a.pp.y = ny } }
    else if (pt.kind === 'rxnArrowPos') { var ar = _struct.rxnArrows.get(pt.id); if (ar && ar.pos && ar.pos[pt.idx]) { ar.pos[pt.idx].x = nx; ar.pos[pt.idx].y = ny } }
    else if (pt.kind === 'rxnArrowP1') { var ar1 = _struct.rxnArrows.get(pt.id); if (ar1 && ar1.p1) { ar1.p1.x = nx; ar1.p1.y = ny } }
    else if (pt.kind === 'rxnArrowP2') { var ar2 = _struct.rxnArrows.get(pt.id); if (ar2 && ar2.p2) { ar2.p2.x = nx; ar2.p2.y = ny } }
    else if (pt.kind === 'rxnPlusPP') { var pl1 = _struct.rxnPluses.get(pt.id); if (pl1 && pl1.pp) { pl1.pp.x = nx; pl1.pp.y = ny } }
    else if (pt.kind === 'rxnPlusXY') { var pl2 = _struct.rxnPluses.get(pt.id); if (pl2) { pl2.x = nx; pl2.y = ny } }
    else if (pt.kind === 'multitail') { var mta = _struct.multitailArrows.get(pt.id); if (mta) { mta.spineTopX = nx; mta.spineTopY = ny } }
}

// ---- Selection handles: rotate (generalized to every selected type) -------
function rotateSelectionLive(angleDelta) {
    if (_rotateDragOrigPos.length === 0) {
        var pts = _snapshotSelectionPoints()
        if (pts.length < 2) return
        var cx = 0, cy = 0
        pts.forEach(function(p) { cx += p.x; cy += p.y })
        _rotateDragOrigPos = pts
        _rotateDragCenter = { x: cx / pts.length, y: cy / pts.length }
        _rotateDragTotalAngle = 0
    }
    var cx = _rotateDragCenter.x, cy = _rotateDragCenter.y
    var candidateTotal = _rotateDragTotalAngle + angleDelta
    var cosA = Math.cos(candidateTotal), sinA = Math.sin(candidateTotal)

    // Page-boundary hard clamp: unlike the move case, a rotation's effect on
    // the bbox isn't a simple linear delta, so instead of solving for an exact
    // boundary angle, just refuse to advance rotation past the point where any
    // point would land outside the page -- the drag freezes there instead of
    // pushing content off-page (the user can still rotate back the other way).
    for (var i = 0; i < _rotateDragOrigPos.length; ++i) {
        var op = _rotateDragOrigPos[i]
        var odx = op.x - cx, ody = op.y - cy
        var nx = cx + odx * cosA - ody * sinA
        var ny = cy + odx * sinA + ody * cosA
        if (nx < PAGE_MIN_X || nx > PAGE_MAX_X || ny < PAGE_MIN_Y || ny > PAGE_MAX_Y) return
    }

    _rotateDragTotalAngle = candidateTotal
    _rotateDragOrigPos.forEach(function(p) {
        var dx = p.x - cx, dy = p.y - cy
        _writeSelectionPoint(p, cx + dx * cosA - dy * sinA, cy + dx * sinA + dy * cosA)
    })
    _dirty = true
}

function commitRotate() {
    if (_rotateDragOrigPos.length > 0 && _rotateDragTotalAngle !== 0) {
        var origPos = _rotateDragOrigPos.slice()
        var totalAngle = _rotateDragTotalAngle
        var cx = _rotateDragCenter.x, cy = _rotateDragCenter.y
        function applyAngle(angle) {
            var cosA = Math.cos(angle), sinA = Math.sin(angle)
            origPos.forEach(function(p) {
                var dx = p.x - cx, dy = p.y - cy
                _writeSelectionPoint(p, cx + dx * cosA - dy * sinA, cy + dx * sinA + dy * cosA)
            })
            _dirty = true
        }
        var isFirst = true
        var cmd = makeCmd(
            function() { if (isFirst) { isFirst = false; return } applyAngle(totalAngle) },
            function() { isFirst = false; applyAngle(0) }
        )
        executeCommand(cmd)
    }
    _rotateDragOrigPos = []
    _rotateDragTotalAngle = 0
    _rotateDragCenter = { x: 0, y: 0 }
}

// ---- Selection handles: resize (uniform scale about a fixed anchor point) --
// anchorX/anchorY is the handle's geometric opposite on the bbox (stays fixed
// through the drag) -- NOT the centroid, matching real corner-drag resize
// behavior (PowerPoint/ChemDraw-adjacent), unlike rotate which is centroid-based.
function scaleSelectionLive(factor, anchorX, anchorY) {
    if (_scaleDragOrigPos.length === 0) {
        var pts = _snapshotSelectionPoints()
        if (pts.length < 2) return
        _scaleDragOrigPos = pts
        _scaleDragAnchor = { x: anchorX, y: anchorY }
        _scaleDragTotalFactor = 1
    }
    // Guard against a handle dragged through its own anchor (collapse/invert).
    var candidateFactor = Math.max(0.05, factor)
    var ax = _scaleDragAnchor.x, ay = _scaleDragAnchor.y

    // Page-boundary hard clamp: freeze before any point would leave the page,
    // same "check candidate, refuse if out of bounds" approach rotate uses.
    for (var i = 0; i < _scaleDragOrigPos.length; ++i) {
        var op = _scaleDragOrigPos[i]
        var nx = ax + (op.x - ax) * candidateFactor
        var ny = ay + (op.y - ay) * candidateFactor
        if (nx < PAGE_MIN_X || nx > PAGE_MAX_X || ny < PAGE_MIN_Y || ny > PAGE_MAX_Y) return
    }

    _scaleDragTotalFactor = candidateFactor
    _scaleDragOrigPos.forEach(function(p) {
        _writeSelectionPoint(p, ax + (p.x - ax) * candidateFactor, ay + (p.y - ay) * candidateFactor)
    })
    _dirty = true
}

function commitScale() {
    if (_scaleDragOrigPos.length > 0 && _scaleDragTotalFactor !== 1) {
        var origPos = _scaleDragOrigPos.slice()
        var totalFactor = _scaleDragTotalFactor
        var ax = _scaleDragAnchor.x, ay = _scaleDragAnchor.y
        function applyFactor(factor) {
            origPos.forEach(function(p) {
                _writeSelectionPoint(p, ax + (p.x - ax) * factor, ay + (p.y - ay) * factor)
            })
            _dirty = true
        }
        var isFirst = true
        var cmd = makeCmd(
            function() { if (isFirst) { isFirst = false; return } applyFactor(totalFactor) },
            function() { isFirst = false; applyFactor(1) }
        )
        executeCommand(cmd)
    }
    _scaleDragOrigPos = []
    _scaleDragAnchor = { x: 0, y: 0 }
    _scaleDragTotalFactor = 1
}

function deleteAtomById(atomId) {
    var a = _struct.atoms.get(atomId)
    if (!a) {
        // Might be a contracted sgroup prim — delete all its atoms, bonds, and the sgroup
        if (!_struct.sgroups) return
        var sg = _struct.sgroups.get(atomId)
        if (!sg) return
        var realIds = sg.atoms ? sg.atoms.slice() : []
        var atomsData = [], bondsData = []
        realIds.forEach(function(aid) {
            var ra = _struct.atoms.get(aid)
            if (ra) atomsData.push({ id: aid, atom: ra })
        })
        _struct.bonds.forEach(function(b, bid) {
            if (realIds.indexOf(b.begin) >= 0 || realIds.indexOf(b.end) >= 0)
                bondsData.push({ id: bid, bond: b })
        })
        var sgId = atomId
        var cmd = makeCmd(
            function() {
                bondsData.forEach(function(bd) { _struct.bonds.delete(bd.id) })
                atomsData.forEach(function(ad) { _struct.atoms.delete(ad.id) })
                _struct.sgroups.delete(sgId)
                _struct.initHalfBonds(); _struct.initNeighbors(); _struct.updateHalfBonds(); _struct.sortNeighbors()
            },
            function() {
                atomsData.forEach(function(ad) { _struct.atoms.set(ad.id, ad.atom) })
                bondsData.forEach(function(bd) { _struct.bonds.set(bd.id, bd.bond) })
                _struct.sgroups.set(sgId, sg)
                _struct.initHalfBonds(); _struct.initNeighbors(); _struct.updateHalfBonds(); _struct.sortNeighbors()
            }
        )
        executeCommand(cmd)
        return
    }
    var bondsData = []
    _struct.bonds.forEach(function(b, bid) {
        if (b.begin === atomId || b.end === atomId) {
            bondsData.push({ id: bid, bond: b })
        }
    })
    // If this atom is a member of an *expanded* (visible) sgroup, deleting it here
    // (as opposed to the contracted-pill branch above, which deletes the whole
    // group) must also trim it from that sgroup's own atoms list -- otherwise the
    // sgroup silently keeps referencing a deleted atom id, which Struct.clone()
    // (every copySelection/KET export) treats as corruption on the next round-trip.
    var sgroupMemberships = []
    if (a.sgs && a.sgs.size > 0) {
        a.sgs.forEach(function(sgId) {
            var sg = _struct.sgroups.get(sgId)
            if (!sg || !sg.atoms) return
            var idx = sg.atoms.indexOf(atomId)
            if (idx >= 0) sgroupMemberships.push({ sg: sg, idx: idx })
        })
    }
    var cmd = makeCmd(
        function() {
            bondsData.forEach(function(bd) { _struct.bonds.delete(bd.id) })
            sgroupMemberships.forEach(function(m) { m.sg.atoms.splice(m.idx, 1) })
            _struct.atoms.delete(atomId)
            _struct.initHalfBonds()
            _struct.initNeighbors()
            _struct.updateHalfBonds()
            _struct.sortNeighbors()
        },
        function() {
            _struct.atoms.set(atomId, a)
            bondsData.forEach(function(bd) { _struct.bonds.set(bd.id, bd.bond) })
            sgroupMemberships.forEach(function(m) { m.sg.atoms.splice(m.idx, 0, atomId) })
            _struct.initHalfBonds()
            _struct.initNeighbors()
            _struct.updateHalfBonds()
            _struct.sortNeighbors()
        }
    )
    executeCommand(cmd)
}

function changeAtomLabel(atomId, newLabel) {
    if (!isValidAtomLabel(newLabel)) throw String("Invalid atom label: " + newLabel)
    var a = _struct.atoms.get(atomId)
    if (a && a.label !== newLabel) {
        var oldLabel = a.label
        var cmd = makeCmd(
            function() { var a2 = _struct.atoms.get(atomId); if(a2) a2.label = newLabel },
            function() { var a2 = _struct.atoms.get(atomId); if(a2) a2.label = oldLabel }
        )
        executeCommand(cmd)
    }
}

function setAtomMapping(atomId, mappingNumber) {
    var a = _struct.atoms.get(atomId)
    if (!a) return
    var oldAam = a.aam || 0
    if (oldAam === mappingNumber) return
    var cmd = makeCmd(
        function() { var a2 = _struct.atoms.get(atomId); if(a2) a2.aam = mappingNumber },
        function() { var a2 = _struct.atoms.get(atomId); if(a2) a2.aam = oldAam }
    )
    executeCommand(cmd)
}

function changeAtomCharge(atomId, newCharge) {
    newCharge = Math.max(-3, Math.min(3, Math.round(newCharge)))
    var a = _struct.atoms.get(atomId)
    if (a && a.charge !== newCharge) {
        var oldCharge = a.charge || 0
        var cmd = makeCmd(
            function() { var a2 = _struct.atoms.get(atomId); if(a2) a2.charge = newCharge },
            function() { var a2 = _struct.atoms.get(atomId); if(a2) a2.charge = oldCharge }
        )
        executeCommand(cmd)
    }
}

function setAttachmentPoint(atomId, order) {
    var a = _struct.atoms.get(atomId)
    if (!a) return
    var old = a.attachmentPoints
    var cmd = makeCmd(
        function() {
            var a2 = _struct.atoms.get(atomId);
            if (a2) {
                a2.attachmentPoints = order || null;
                _dirty = true;
            }
        },
        function() {
            var a2 = _struct.atoms.get(atomId);
            if (a2) {
                a2.attachmentPoints = old;
                _dirty = true;
            }
        }
    )
    executeCommand(cmd)
}

function changeAtomIsotope(atomId, isotope) {
    var a = _struct.atoms.get(atomId)
    if (!a) return
    isotope = Math.max(0, Math.round(isotope))
    var oldIsotope = a.isotope || 0
    var cmd = makeCmd(
        function() { var a2 = _struct.atoms.get(atomId); if(a2) a2.isotope = isotope },
        function() { var a2 = _struct.atoms.get(atomId); if(a2) a2.isotope = oldIsotope }
    )
    executeCommand(cmd)
}

function changeAtomRadical(atomId, radical) {
    var a = _struct.atoms.get(atomId)
    if (!a) return
    radical = Math.max(0, Math.min(3, Math.round(radical)))
    var oldRadical = a.radical || 0
    var cmd = makeCmd(
        function() { var a2 = _struct.atoms.get(atomId); if(a2) a2.radical = radical },
        function() { var a2 = _struct.atoms.get(atomId); if(a2) a2.radical = oldRadical }
    )
    executeCommand(cmd)
}

function changeAtomValence(atomId, valence) {
    var a = _struct.atoms.get(atomId)
    if (!a) return
    var oldValence = (a.explicitValence !== undefined && a.explicitValence >= 0) ? a.explicitValence : -1
    var cmd = makeCmd(
        function() { var a2 = _struct.atoms.get(atomId); if(a2) a2.explicitValence = valence },
        function() { var a2 = _struct.atoms.get(atomId); if(a2) a2.explicitValence = oldValence }
    )
    executeCommand(cmd)
}

function getAtomProperties(atomId) {
    var a = _struct.atoms.get(atomId)
    if (!a) return null
    return {
        id: atomId,
        label: a.label,
        charge: a.charge || 0,
        isotope: a.isotope || 0,
        radical: a.radical || 0,
        explicitValence: (a.explicitValence !== undefined && a.explicitValence >= 0) ? a.explicitValence : -1
    }
}

function deleteBondById(bondId) {
    var b = _struct.bonds.get(bondId)
    if (!b) return
    var cmd = makeCmd(
        function() {
            _struct.bonds.delete(bondId)
            _struct.initHalfBonds()
            _struct.initNeighbors()
            _struct.updateHalfBonds()
            _struct.sortNeighbors()
        },
        function() {
            _struct.bonds.set(bondId, b)
            _struct.initHalfBonds()
            _struct.initNeighbors()
            _struct.updateHalfBonds()
            _struct.sortNeighbors()
        }
    )
    executeCommand(cmd)
}

function changeBondType(bondId, newType, newStereo) {
    var b = _struct.bonds.get(bondId)
    if (b) {
        if (newStereo === undefined) newStereo = 0
        if (b.type !== newType || b.stereo !== newStereo) {
            var oldType = b.type
            var oldStereo = b.stereo || 0
            function syncBonds() { _struct.initHalfBonds(); _struct.initNeighbors(); _struct.updateHalfBonds(); _struct.sortNeighbors() }
            var cmd = makeCmd(
                function() { var b2 = _struct.bonds.get(bondId); if(b2) { b2.type = newType; b2.stereo = newStereo; } syncBonds() },
                function() { var b2 = _struct.bonds.get(bondId); if(b2) { b2.type = oldType; b2.stereo = oldStereo; } syncBonds() }
            )
            executeCommand(cmd)
        }
    }
}

// ---- Public API: Basic Tools -----------------------------------------------

function addBondAndAtom(startId, label, x, y, type, stereo) {
    if (!_struct.atoms.get(startId) && _struct.sgroups) {
        var _sg = _struct.sgroups.get(startId)
        if (_sg && _sg.attachmentPoints && _sg.attachmentPoints.length > 0)
            startId = _sg.attachmentPoints[0].atomId
        else if (_sg && _sg.atoms && _sg.atoms.length > 0)
            startId = _sg.atoms[0]
    }
    var startAtom = _struct.atoms.get(startId);
    if (!startAtom) return;

    var dist = getDistance(startAtom.pp.x, startAtom.pp.y, x, y);
    var finalX = x;
    var finalY = y;

    if (dist < 0.5) {
        var angle = 0;
        var ringAngle = getRingCenterAngle(startId);
        
        if (ringAngle !== null) {
            angle = ringAngle;
        } else {
            angle = getLargestEmptyAngle(startId);
        }
        
        var BL = 1.0; 
        finalX = startAtom.pp.x + Math.cos(angle) * BL;
        finalY = startAtom.pp.y + Math.sin(angle) * BL;
    }

    var collisionId = null;
    _struct.atoms.forEach(function(atom, id) {
        if (id !== startId && getDistance(atom.pp.x, atom.pp.y, finalX, finalY) < 0.3) {
            collisionId = id;
        }
    });

    if (collisionId !== null) {
        addBond(startId, collisionId, type, stereo);
        return;
    }

    var clampedEnd = _clampToPage(finalX, finalY)
    var p = new CoreLib.ChemCore.Vec2(clampedEnd.x, clampedEnd.y)
    var atomId = null
    var bondId = null
    var cmd = makeCmd(
        function() {
            if (atomId === null) {
                atomId = _struct.atoms.add(new CoreLib.ChemCore.Atom({ label: label || "C", charge: 0, pp: p }))
                bondId = _struct.bonds.add(new CoreLib.ChemCore.Bond({ type: type || 1, stereo: stereo || 0, begin: startId, end: atomId }))
            } else {
                _struct.atoms.set(atomId, new CoreLib.ChemCore.Atom({ label: label || "C", charge: 0, pp: p }))
                _struct.bonds.set(bondId, new CoreLib.ChemCore.Bond({ type: type || 1, stereo: stereo || 0, begin: startId, end: atomId }))
            }
            if (_struct.initHalfBonds) { _struct.initHalfBonds(); _struct.initNeighbors(); _struct.updateHalfBonds(); _struct.sortNeighbors() }
        },
        function() {
            _struct.bonds.delete(bondId)
            _struct.atoms.delete(atomId)
            if (_struct.initHalfBonds) { _struct.initHalfBonds(); _struct.initNeighbors(); _struct.updateHalfBonds(); _struct.sortNeighbors() }
        }
    )
    executeCommand(cmd)
}

    function addBondBetweenCoords(x1, y1, x2, y2, type, stereo) {
        // Creates a bond between two empty coordinate points
        var c1 = _clampToPage(x1, y1);
        var c2 = _clampToPage(x2, y2);
        var p1 = new CoreLib.ChemCore.Vec2(c1.x, c1.y);
        var p2 = new CoreLib.ChemCore.Vec2(c2.x, c2.y);
        var atomId1 = null;
        var atomId2 = null;
        var bondId = null;
        
        var cmd = makeCmd(
            function() {
                if (atomId1 === null) {
                    atomId1 = _struct.atoms.add(new CoreLib.ChemCore.Atom({ label: "C", charge: 0, pp: p1 }));
                    atomId2 = _struct.atoms.add(new CoreLib.ChemCore.Atom({ label: "C", charge: 0, pp: p2 }));
                    bondId = _struct.bonds.add(new CoreLib.ChemCore.Bond({ type: type || 1, stereo: stereo || 0, begin: atomId1, end: atomId2 }));
                } else {
                    _struct.atoms.set(atomId1, new CoreLib.ChemCore.Atom({ label: "C", charge: 0, pp: p1 }));
                    _struct.atoms.set(atomId2, new CoreLib.ChemCore.Atom({ label: "C", charge: 0, pp: p2 }));
                    _struct.bonds.set(bondId, new CoreLib.ChemCore.Bond({ type: type || 1, stereo: stereo || 0, begin: atomId1, end: atomId2 }));
                }
                if (_struct.initHalfBonds) { _struct.initHalfBonds(); _struct.initNeighbors(); _struct.updateHalfBonds(); _struct.sortNeighbors(); }
            },
            function() {
                _struct.bonds.delete(bondId);
                _struct.atoms.delete(atomId2);
                _struct.atoms.delete(atomId1);
                if (_struct.initHalfBonds) { _struct.initHalfBonds(); _struct.initNeighbors(); _struct.updateHalfBonds(); _struct.sortNeighbors(); }
            }
        );
        executeCommand(cmd);
    }
    
    // ---- Public API: Selection -------------------------------------------------

function normalizeStructure() {
    var oldPositions = []
    _struct.atoms.forEach(function(a, id) {
        oldPositions.push({ id: id, x: a.pp.x, y: a.pp.y })
    })
    var cmd = makeCmd(
        function() { if (_struct.rescale) _struct.rescale() },
        function() {
            oldPositions.forEach(function(p) {
                var a = _struct.atoms.get(p.id)
                if (a) { a.pp.x = p.x; a.pp.y = p.y }
            })
        }
    )
    executeCommand(cmd)
}

function centerStructure() {
    if (_struct.atoms.size === 0) return
    if (!_struct.getCoordBoundingBox) return
    var bb = _struct.getCoordBoundingBox()
    if (!bb || !bb.min || !bb.max) return
    var cx = (bb.min.x + bb.max.x) / 2
    var cy = (bb.min.y + bb.max.y) / 2
    var dx = -cx
    var dy = -cy
    var oldPositions = []
    _struct.atoms.forEach(function(a, id) {
        oldPositions.push({ id: id, x: a.pp.x, y: a.pp.y })
    })
    var dx2 = dx, dy2 = dy
    var cmd = makeCmd(
        function() {
            _struct.atoms.forEach(function(a) {
                a.pp.x += dx2; a.pp.y += dy2
            })
        },
        function() {
            oldPositions.forEach(function(p) {
                var a = _struct.atoms.get(p.id)
                if (a) { a.pp.x = p.x; a.pp.y = p.y }
            })
        }
    )
    executeCommand(cmd)
}

function alignAtoms(direction) {
    var ids = _selection.atom_ids.slice()
    if (ids.length < 2) return
    var oldPos = []
    var minV = null, maxV = null, sumV = 0
    ids.forEach(function(aid) {
        var a = _struct.atoms.get(aid)
        if (a) {
            var v = (direction === "left" || direction === "right" || direction === "centerH") ? a.pp.x : a.pp.y
            oldPos.push({ id: aid, x: a.pp.x, y: a.pp.y })
            if (minV === null || v < minV) minV = v
            if (maxV === null || v > maxV) maxV = v
            sumV += v
        }
    })
    var target
    if (direction === "left" || direction === "top") target = minV
    else if (direction === "right" || direction === "bottom") target = maxV
    else target = sumV / ids.length

    var cmd = makeCmd(
        function() {
            oldPos.forEach(function(p) {
                var a = _struct.atoms.get(p.id)
                if (a) {
                    if (direction === "left" || direction === "right" || direction === "centerH") a.pp.x = target
                    else a.pp.y = target
                }
            })
            _dirty = true
        },
        function() {
            oldPos.forEach(function(p) {
                var a = _struct.atoms.get(p.id)
                if (a) { a.pp.x = p.x; a.pp.y = p.y }
            })
            _dirty = true
        }
    )
    executeCommand(cmd)
}

function distributeAtoms(direction) {
    var ids = _selection.atom_ids.slice()
    if (ids.length < 3) return
    var oldPos = []
    var coords = []
    ids.forEach(function(aid) {
        var a = _struct.atoms.get(aid)
        if (a) {
            oldPos.push({ id: aid, x: a.pp.x, y: a.pp.y })
            coords.push({ id: aid, v: (direction === "horizontal") ? a.pp.x : a.pp.y })
        }
    })
    coords.sort(function(a, b) { return a.v - b.v })
    var minC = coords[0].v, maxC = coords[coords.length - 1].v
    var step = (maxC - minC) / (coords.length - 1)
    var newPositions = {}
    coords.forEach(function(c, i) {
        newPositions[c.id] = minC + step * i
    })
    var cmd = makeCmd(
        function() {
            oldPos.forEach(function(p) {
                var a = _struct.atoms.get(p.id)
                if (a) {
                    if (direction === "horizontal") a.pp.x = newPositions[p.id]
                    else a.pp.y = newPositions[p.id]
                }
            })
            _dirty = true
        },
        function() {
            oldPos.forEach(function(p) {
                var a = _struct.atoms.get(p.id)
                if (a) { a.pp.x = p.x; a.pp.y = p.y }
            })
            _dirty = true
        }
    )
    executeCommand(cmd)
}

// "Layout Selected": re-position only the selected atoms, leaving everything
// else exactly fixed. Implemented entirely worker-side against chem-core's
// own atom/bond model -- NOT via Indigo. Indigo's indigoLayoutSelected is a
// dead header stub (declared, never implemented, in any release checked
// including a fresh local v1.45.0 compile), and indigoLayout's real
// submolecule-filtered mode (_layoutSingleComponent in
// molecule_layout_graph.cpp) always regenerates the WHOLE connected
// component once any vertex is free to move, using old positions only to
// loosely re-orient the result afterward -- it cannot "freeze the rest,
// recompute a subset" for a plain small molecule (that capability is real
// only for tgroups/monomer biopolymer structures). Confirmed by reading the
// source directly and reproducing the exact same (undesired) numeric result
// twice, not guessed.
//
// v1 scope, deliberately narrow to what can be done safely and predictably:
// only a simple, unbranched, acyclic chain of selected atoms attached to the
// rest of the structure at exactly one point. Anything else (disconnected
// sub-selection, a branch point, a cycle, zero or multiple attachment
// points) is declined -- no-op + console.warn -- rather than risk producing
// a broken or overlapping layout.
function layoutSelectedChain() {
    var sel = _selection.atom_ids || []
    if (sel.length < 1) return
    var selSet = {}
    sel.forEach(function(id) { selSet[id] = true })

    // Classify every bond touching the selection: "internal" (both ends
    // selected) builds the chain graph; exactly one end selected marks the
    // single allowed attachment point to the fixed structure.
    var internalAdj = {}
    var anchorBonds = []
    sel.forEach(function(id) { internalAdj[id] = [] })
    _struct.bonds.forEach(function(b, bid) {
        var beginSel = !!selSet[b.begin], endSel = !!selSet[b.end]
        if (beginSel && endSel) {
            internalAdj[b.begin].push(b.end)
            internalAdj[b.end].push(b.begin)
        } else if (beginSel !== endSel) {
            anchorBonds.push(beginSel ? { insideId: b.begin, outsideId: b.end } : { insideId: b.end, outsideId: b.begin })
        }
    })

    if (anchorBonds.length !== 1) {
        console.warn("layoutSelectedChain: needs exactly one connection point to the rest of the structure, found " + anchorBonds.length)
        return
    }
    for (var i = 0; i < sel.length; i++) {
        if (internalAdj[sel[i]].length > 2) {
            console.warn("layoutSelectedChain: branching within the selection is not supported")
            return
        }
    }

    // Walk the chain from the atom bonded to the anchor; must visit every
    // selected atom exactly once (rules out cycles and any disconnected
    // sub-group within the selection).
    var anchor = anchorBonds[0]
    var chain = [anchor.insideId]
    var visited = {}
    visited[anchor.insideId] = true
    var prevId = null
    var curId = anchor.insideId
    while (chain.length < sel.length) {
        var neighbors = internalAdj[curId] || []
        var nextId = null
        for (var j = 0; j < neighbors.length; j++) {
            if (neighbors[j] !== prevId && !visited[neighbors[j]]) { nextId = neighbors[j]; break }
        }
        if (nextId === null) break
        chain.push(nextId)
        visited[nextId] = true
        prevId = curId
        curId = nextId
    }
    if (chain.length !== sel.length) {
        console.warn("layoutSelectedChain: selection must form a single unbranched, acyclic chain")
        return
    }

    var anchorAtom = _struct.atoms.get(anchor.outsideId)
    if (!anchorAtom) return

    // Reuses addChain's exact zigzag convention (v8_worker.js addChain: same
    // Math.PI/6 half-angle, same StandardBondLength) and getLargestEmptyAngle's
    // established single-neighbor placement heuristic (already used for the
    // short-drag chain-extend fallback) -- not new geometry math.
    var sbl = CoreLib.ChemCore.StandardBondLength || 1.5
    var half = Math.PI / 6
    var theta = getLargestEmptyAngle(anchor.outsideId)

    var oldPositions = chain.map(function(id) {
        var a = _struct.atoms.get(id)
        return { id: id, x: a.pp.x, y: a.pp.y }
    })
    var newPositions = []
    var px = anchorAtom.pp.x, py = anchorAtom.pp.y
    for (var k = 0; k < chain.length; k++) {
        var ang = theta + ((k % 2 === 0) ? half : -half)
        px += sbl * Math.cos(ang)
        py += sbl * Math.sin(ang)
        newPositions.push({ id: chain[k], x: px, y: py })
    }

    var cmd = makeCmd(
        function() {
            newPositions.forEach(function(p) {
                var a = _struct.atoms.get(p.id)
                if (a) { a.pp.x = p.x; a.pp.y = p.y }
            })
            _dirty = true
        },
        function() {
            oldPositions.forEach(function(p) {
                var a = _struct.atoms.get(p.id)
                if (a) { a.pp.x = p.x; a.pp.y = p.y }
            })
            _dirty = true
        }
    )
    executeCommand(cmd)
}

function transformSelection(mode) {
    var ids = _resolveAtomMoveIds(_selection.atom_ids)
    if (!ids || ids.length < 2) return
    var oldPos = []
    var cx = 0, cy = 0, n = 0
    ids.forEach(function(aid) {
        var a = _struct.atoms.get(aid)
        if (a) { oldPos.push({ id: aid, x: a.pp.x, y: a.pp.y }); cx += a.pp.x; cy += a.pp.y; n++ }
    })
    if (n < 2) return
    cx /= n; cy /= n

    // Flips invert chirality: wedge bonds fully inside the selection swap
    // up<->down (stereo 1 <-> 6) so the depiction stays chemically consistent.
    var idSet = {}
    ids.forEach(function(aid) { idSet[aid] = true })
    var stereoSwaps = []
    if (mode === "flip_h" || mode === "flip_v") {
        _struct.bonds.forEach(function(b, bid) {
            if (idSet[b.begin] && idSet[b.end] && (b.stereo === 1 || b.stereo === 6))
                stereoSwaps.push({ id: bid, old: b.stereo })
        })
    }

    function apply(p) {
        if (mode === "rotate_cw")  return { x: cx + (p.y - cy), y: cy - (p.x - cx) }
        if (mode === "rotate_ccw") return { x: cx - (p.y - cy), y: cy + (p.x - cx) }
        if (mode === "flip_h")     return { x: 2 * cx - p.x, y: p.y }
        if (mode === "flip_v")     return { x: p.x, y: 2 * cy - p.y }
        return p
    }

    var cmd = makeCmd(
        function() {
            oldPos.forEach(function(p) {
                var a = _struct.atoms.get(p.id)
                if (a) { var q = apply(p); a.pp.x = q.x; a.pp.y = q.y }
            })
            stereoSwaps.forEach(function(s) {
                var b = _struct.bonds.get(s.id)
                if (b) b.stereo = (s.old === 1) ? 6 : 1
            })
            _dirty = true
        },
        function() {
            oldPos.forEach(function(p) {
                var a = _struct.atoms.get(p.id)
                if (a) { a.pp.x = p.x; a.pp.y = p.y }
            })
            stereoSwaps.forEach(function(s) {
                var b = _struct.bonds.get(s.id)
                if (b) b.stereo = s.old
            })
            _dirty = true
        }
    )
    executeCommand(cmd)
}

// ---- Chain tool --------------------------------------------------------------
// Lays a zig-zag carbon chain from (x1,y1) toward (x2,y2); atom count derives
// from the drag distance. Reuses addRing's merge+fuse pattern so chain endpoints
// graft onto existing atoms they overlap.
function addChain(x1, y1, x2, y2) {
    var _c1 = _clampToPage(x1, y1); x1 = _c1.x; y1 = _c1.y
    var _c2 = _clampToPage(x2, y2); x2 = _c2.x; y2 = _c2.y
    var bondLen = CoreLib.ChemCore.StandardBondLength || 1.5
    var dx = x2 - x1, dy = y2 - y1
    var dist = Math.sqrt(dx * dx + dy * dy)
    var nBonds = Math.max(1, Math.round(dist / bondLen))
    var theta = Math.atan2(dy, dx)
    var half = Math.PI / 6

    var oldStruct = snapshotStruct(_struct)
    var newStruct = null
    var cmd = makeCmd(
        function() {
            if (newStruct !== null) { _struct = newStruct; return }
            var tempStruct = new CoreLib.ChemCore.Struct()
            var prev = null
            var px = x1, py = y1
            for (var i = 0; i <= nBonds; i++) {
                var a = tempStruct.atoms.add(new CoreLib.ChemCore.Atom({ label: "C", charge: 0, pp: new CoreLib.ChemCore.Vec2(px, py) }))
                if (prev !== null) tempStruct.bonds.add(new CoreLib.ChemCore.Bond({ type: 1, begin: prev, end: a }))
                prev = a
                var ang = theta + ((i % 2 === 0) ? half : -half)
                px += bondLen * Math.cos(ang)
                py += bondLen * Math.sin(ang)
            }
            var aidMap = new Map()
            tempStruct.mergeInto(_struct, undefined, undefined, undefined, undefined, aidMap)
            fuseOverlappingAtoms()
            if (_struct.initHalfBonds) { _struct.initHalfBonds(); _struct.initNeighbors(); _struct.updateHalfBonds(); _struct.sortNeighbors() }
            newStruct = snapshotStruct(_struct)
        },
        function() {
            _struct = oldStruct
            if (_struct.initHalfBonds) { _struct.initHalfBonds(); _struct.initNeighbors(); _struct.updateHalfBonds(); _struct.sortNeighbors() }
        }
    )
    executeCommand(cmd)
}

// ---- Text annotations ----------------------------------------------------------
// chem-core's Text class isn't exported; plain objects with the same duck-typed
// shape (content, position, pos[], clone) satisfy Struct cloning and the
// KetSerializer's native "text" node handling.
// KetSerializer's textToKet JSON.parses `content` (Lexical editor format) and
// reads a 4-corner `pos` bounding box — a bare string with 2 pos points throws.
function _makeAtomList(labelsCsv, notList) {
    var labels = (labelsCsv || "").split(",").map(function(s) { return s.trim() }).filter(function(s) { return s.length > 0 })
    var ids = []
    labels.forEach(function(lbl) {
        var el = CoreLib.ChemCore.Elements.get(lbl)
        if (el && ids.indexOf(el.number) === -1) ids.push(el.number)
    })
    if (ids.length === 0) return null
    return {
        ids: ids,
        notList: !!notList,
        labelList: function() {
            return this.ids.map(function(num) {
                var el = CoreLib.ChemCore.Elements.get(num)
                return el ? el.label : "?"
            })
        }
    }
}

// labelsCsv: comma-separated element symbols, e.g. "C,N,O". Sets atom.label to the "L#"
// sentinel chem-core.js expects for atom-list query atoms.
function setAtomQueryList(atomId, labelsCsv, notList) {
    var a = _struct.atoms.get(atomId)
    if (!a) return
    var newAtomList = _makeAtomList(labelsCsv, notList)
    if (!newAtomList) return
    var oldLabel = a.label
    var oldAtomList = a.atomList
    var cmd = makeCmd(
        function() { a.label = "L#"; a.atomList = newAtomList },
        function() { a.label = oldLabel; a.atomList = oldAtomList }
    )
    executeCommand(cmd)
}

function clearAtomQueryList(atomId, fallbackLabel) {
    var a = _struct.atoms.get(atomId)
    if (!a || a.label !== "L#") return
    var oldLabel = a.label
    var oldAtomList = a.atomList
    var newLabel = fallbackLabel || "C"
    var cmd = makeCmd(
        function() { a.label = newLabel; a.atomList = null },
        function() { a.label = oldLabel; a.atomList = oldAtomList }
    )
    executeCommand(cmd)
}

