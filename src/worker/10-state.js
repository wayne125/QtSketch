var _VALID_QUERY_LABELS = { A: true, AH: true, Q: true, QH: true, M: true, MH: true, X: true, XH: true, R: true }

function isValidAtomLabel(label) {
    if (!label || typeof label !== "string") return false
    if (CoreLib.ChemCore.Elements.get(label)) return true
    if (label in CoreLib.ChemCore.AtomLabel) return true
    if (_VALID_QUERY_LABELS[label] === true) return true
    if (CoreLib.ChemCore.attachmentPointNames && CoreLib.ChemCore.attachmentPointNames.indexOf(label) >= 0) return true
    return false
}

function isRGroupLabel(label) {
    if (!label || typeof label !== "string") return false
    if (CoreLib.ChemCore.attachmentPointNames) {
        return CoreLib.ChemCore.attachmentPointNames.indexOf(label) >= 0
    }
    return /^R[1-8]$/.test(label)
}

// ============================================================================
// ---- Internal State --------------------------------------------------------

var _struct = null
var _selection = { atom_ids: [], bond_ids: [], rxnArrow_ids: [], rxnPlus_ids: [], multitailArrow_ids: [], bbox: null }

var _history = []
var _historyPointer = -1
var _showExplicitH = false
var HISTORY_SIZE = 50

var _dirty = false
var _initialized = false

// ---- Private Helpers -------------------------------------------------------

// ---- Geometry Math Helpers -------------------------------------------------

function getDistance(x1, y1, x2, y2) {
    var dx = x2 - x1;
    var dy = y2 - y1;
    return Math.sqrt(dx*dx + dy*dy);
}

function getAngle(x1, y1, x2, y2) {
    return Math.atan2(y2 - y1, x2 - x1);
}

function getLargestEmptyAngle(atomId) {
    var atom = _struct.atoms.get(atomId);
    if (!atom) return 0;
    
    var neighbors = [];
    _struct.bonds.forEach(function(b, bid) {
        if (b.begin === atomId) {
            var n = _struct.atoms.get(b.end);
            neighbors.push(getAngle(atom.pp.x, atom.pp.y, n.pp.x, n.pp.y));
        } else if (b.end === atomId) {
            var n = _struct.atoms.get(b.begin);
            neighbors.push(getAngle(atom.pp.x, atom.pp.y, n.pp.x, n.pp.y));
        }
    });
    
    if (neighbors.length === 0) return 0;
    if (neighbors.length === 1) {
        return neighbors[0] + 2.61799; 
    }
    
    neighbors.sort(function(a, b) { return a - b; });
    var maxGap = 0;
    var bestAngle = 0;
    for (var i = 0; i < neighbors.length; i++) {
        var a1 = neighbors[i];
        var a2 = neighbors[(i + 1) % neighbors.length];
        var gap = a2 - a1;
        while (gap <= 0) gap += Math.PI * 2;
        if (gap > maxGap) {
            maxGap = gap;
            bestAngle = a1 + gap / 2;
        }
    }
    return bestAngle;
}

function getRingCenterAngle(atomId) {
    var atom = _struct.atoms.get(atomId);
    if (!atom) return null;
    var cx = 0, cy = 0, count = 0;
    _struct.bonds.forEach(function(b, bid) {
        var n = null;
        if (b.begin === atomId) n = _struct.atoms.get(b.end);
        else if (b.end === atomId) n = _struct.atoms.get(b.begin);
        if (n) {
            cx += n.pp.x;
            cy += n.pp.y;
            count++;
        }
    });
    if (count < 2) return null; // Not in a junction
    cx /= count;
    cy /= count;
    return getAngle(cx, cy, atom.pp.x, atom.pp.y);
}

// Returns a function mapping any (x, y) through the rotate + uniform-scale +
// translate that sends P1→Q1 and P2→Q2 exactly. Always orientation-preserving
// (positive determinant, no reflection), so cross-product side tests keep
// their sign through the transform — insertLibraryTemplateFused relies on that
// to steer which side of the target bond the template lands on purely by the
// choice of endpoint mapping. Returns null for a degenerate (zero-length) P1P2.
function makeSimilarityTransform(p1x, p1y, p2x, p2y, q1x, q1y, q2x, q2y) {
    var dp = getDistance(p1x, p1y, p2x, p2y)
    if (dp < 1e-9) return null
    var s = getDistance(q1x, q1y, q2x, q2y) / dp
    var rot = getAngle(q1x, q1y, q2x, q2y) - getAngle(p1x, p1y, p2x, p2y)
    var cosR = Math.cos(rot), sinR = Math.sin(rot)
    return function(x, y) {
        var rx = x - p1x, ry = y - p1y
        return {
            x: q1x + s * (rx * cosR - ry * sinR),
            y: q1y + s * (rx * sinR + ry * cosR)
        }
    }
}

// Smallest ring containing the given bond, as an ordered atom-id cycle
// (consecutive entries bonded; last connects back to first via bondId itself),
// or null if the bond is not part of any ring. BFS shortest path between the
// bond's endpoints with the bond itself removed. Read-only.
function shortestRingThroughBond(struct, bondId) {
    var bond = struct.bonds.get(bondId)
    if (!bond) return null
    var adj = {}
    struct.bonds.forEach(function(b, bid) {
        if (bid === bondId) return
        if (!adj[b.begin]) adj[b.begin] = []
        if (!adj[b.end]) adj[b.end] = []
        adj[b.begin].push(b.end)
        adj[b.end].push(b.begin)
    })
    var start = bond.begin, goal = bond.end
    var prev = {}
    prev[start] = null
    var queue = [start]
    while (queue.length > 0) {
        var cur = queue.shift()
        if (cur === goal) break
        var nbrs = adj[cur] || []
        for (var i = 0; i < nbrs.length; i++) {
            if (!(nbrs[i] in prev)) { prev[nbrs[i]] = cur; queue.push(nbrs[i]) }
        }
    }
    if (!(goal in prev)) return null
    var cycle = []
    for (var a = goal; a !== null; a = prev[a]) cycle.push(a)
    return cycle
}

// Which side of the segment QA→QB has less existing structure to grow a ring
// onto? Returns +1 or -1: the sign of cross((B-A), (P-A)) for points P on the
// chosen side. Mass-scoring heuristic matching V8Process::getRingPreviewCoords'
// bond-hover branch; the cursor position only breaks the tie when both sides
// are empty (same precedent as the interactive TEMPLATE_ ring tool).
function chooseEmptySide(qax, qay, qbx, qby, excludeAids, cx, cy) {
    var vx = qbx - qax, vy = qby - qay
    var scorePos = 0, scoreNeg = 0
    _struct.atoms.forEach(function(a, aid) {
        if (excludeAids.indexOf(aid) >= 0) return
        var cross = vx * (a.pp.y - qay) - vy * (a.pp.x - qax)
        if (cross > 0) scorePos += cross
        else if (cross < 0) scoreNeg -= cross
    })
    if (scorePos === 0 && scoreNeg === 0) {
        var cursorCross = vx * (cy - qay) - vy * (cx - qax)
        return cursorCross >= 0 ? 1 : -1
    }
    return scorePos <= scoreNeg ? 1 : -1
}

function fuseOverlappingAtoms() {
    var toDelete = [];
    var mergeMap = {};
    var allAtoms = Array.from(_struct.atoms.keys());

    for (var i = 0; i < allAtoms.length; i++) {
        var a1_id = allAtoms[i];
        if (toDelete.indexOf(a1_id) >= 0) continue;
        var a1 = _struct.atoms.get(a1_id);

        for (var j = i + 1; j < allAtoms.length; j++) {
            var a2_id = allAtoms[j];
            if (toDelete.indexOf(a2_id) >= 0) continue;
            var a2 = _struct.atoms.get(a2_id);

            if (getDistance(a1.pp.x, a1.pp.y, a2.pp.x, a2.pp.y) < 0.1) {
                toDelete.push(a2_id);
                mergeMap[a2_id] = a1_id;
            }
        }
    }

    var bondsToDelete = [];
    _struct.bonds.forEach(function(b, bid) {
        if (mergeMap[b.begin] !== undefined) b.begin = mergeMap[b.begin];
        if (mergeMap[b.end] !== undefined) b.end = mergeMap[b.end];
        if (b.begin === b.end) bondsToDelete.push(bid);
    });

    // Dedupe bonds that now connect the same fused atom pair. The FIRST-seen
    // bond (always the pre-existing one, since it was inserted before the new
    // ring was merged in) keeps its type unchanged — a newly-drawn ring must
    // never silently override an already-committed bond order at the fusion
    // seam (previously this took max(type), which could force a fusion edge
    // to double and break Kekule alternation on the existing ring).
    var seenBonds = {};
    _struct.bonds.forEach(function(b, bid) {
        if (bondsToDelete.indexOf(bid) >= 0) return;
        var pair = Math.min(b.begin, b.end) + "-" + Math.max(b.begin, b.end);
        if (seenBonds[pair] !== undefined) {
            bondsToDelete.push(bid);
        } else {
            seenBonds[pair] = bid;
        }
    });

    bondsToDelete.forEach(function(bid) { _struct.bonds.delete(bid); });
    toDelete.forEach(function(aid) { _struct.atoms.delete(aid); });
    return mergeMap;
}


// Page/canvas boundary, in chemical-coordinate space (see chem-core.js's
// StandardBondLength = 1.5 for scale), centered on the origin. Proportioned
// roughly like A4 landscape and sized generously so a typical reaction
// scheme fits comfortably (~40 bond-lengths wide). Confirmed as a hard
// clamp with the user (unlike ChemDraw itself, whose page lines are only a
// soft print/layout guide) -- exact real-world mm mapping is deliberately
// left to the future full-rulers system, not this pass.
var PAGE_MIN_X = -30, PAGE_MAX_X = 30
var PAGE_MIN_Y = -21, PAGE_MAX_Y = 21

function _clampToPage(x, y) {
    return {
        x: Math.max(PAGE_MIN_X, Math.min(PAGE_MAX_X, x)),
        y: Math.max(PAGE_MIN_Y, Math.min(PAGE_MAX_Y, y))
    }
}

var _dragDelta = { x: 0, y: 0 }
var _dragSelection = []
var _dragArrowSelection = []
var _dragPlusSelection = []
var _dragMultitailSelection = []
var _dragOrigBBox = null  // atom bbox snapshot at drag start, for page-boundary clamping

var _rotateDragOrigPos = []   // [{kind, id, [idx], x, y}] snapshot at rotate-drag start
var _rotateDragTotalAngle = 0
var _rotateDragCenter = { x: 0, y: 0 }

var _scaleDragOrigPos = []    // [{kind, id, [idx], x, y}] snapshot at resize-drag start
var _scaleDragAnchor = { x: 0, y: 0 }
var _scaleDragTotalFactor = 1

function executeCommand(cmd) {
    // Remove future redo states
    if (_historyPointer + 1 < _history.length) {
        _history.splice(_historyPointer + 1)
    }
    try {
        cmd.execute()
    } catch (e) {
        _dirty = true
        throw e
    }
    _history.push(cmd)
    _historyPointer++
    if (_history.length > HISTORY_SIZE) {
        _history.shift()
        _historyPointer--
    }
    _dirty = true
}

function makeCmd(executeFn, invertFn) {
    var cmd = new CoreLib.ChemCore.Command()
    cmd.addOperation(executeFn)
    cmd.addInverseOperation(invertFn)
    var _baseInvert = cmd.invert.bind(cmd)
    cmd.invert = function() { _baseInvert().execute() }
    return cmd
}

function snapshotStruct(s) {
    var copy = new CoreLib.ChemCore.Struct()
    var maxId = -1
    s.atoms.forEach(function(a, id) {
        copy.atoms.set(id, new CoreLib.ChemCore.Atom({
            label: a.label, charge: a.charge || 0,
            pp: new CoreLib.ChemCore.Vec2(a.pp.x, a.pp.y),
            isotope: a.isotope, explicitValence: a.explicitValence,
            radical: a.radical, stereoParity: a.stereoParity,
            aam: a.aam || 0,
            atomList: a.atomList || null
        }))
        if (id > maxId) maxId = id
    })
    if (copy.atoms.nextId !== undefined) copy.atoms.nextId = Math.max(copy.atoms.nextId || 0, maxId + 1)
    maxId = -1
    s.bonds.forEach(function(b, id) {
        copy.bonds.set(id, new CoreLib.ChemCore.Bond({
            type: b.type, stereo: b.stereo || 0, begin: b.begin, end: b.end
        }))
        if (id > maxId) maxId = id
    })
    if (copy.bonds.nextId !== undefined) copy.bonds.nextId = Math.max(copy.bonds.nextId || 0, maxId + 1)
    if (s.rxnArrows && s.rxnArrows.forEach) {
        s.rxnArrows.forEach(function(ar, id) {
            copy.rxnArrows.set(id, ar.clone ? ar.clone() : ar)
        })
    }
    if (s.rxnPluses && s.rxnPluses.forEach) {
        s.rxnPluses.forEach(function(pl, id) {
            copy.rxnPluses.set(id, pl.clone ? pl.clone() : pl)
        })
    }
    if (s.sgroups && s.sgroups.forEach) {
        s.sgroups.forEach(function(sg, id) {
            // Preserve sg's prototype (real SGroup instances have setFunctionalGroup/
            // isContracted/isExpanded/etc. on their prototype) — Object.assign({}, sg)
            // would silently drop them, and markFragments() calls them on every sgroup.
            var sgCopy = Object.assign(Object.create(Object.getPrototypeOf(sg)), sg)
            if (sg.atoms) sgCopy.atoms = sg.atoms.slice()
            // Attachment points are also real class instances (AttachmentPoint.clone()
            // is called by SGroup.cloneAttachmentPoints during MOL/KET serialization) —
            // same prototype-preservation reasoning as sgCopy above.
            if (sg.attachmentPoints) sgCopy.attachmentPoints = sg.attachmentPoints.map(function(ap) { return Object.assign(Object.create(Object.getPrototypeOf(ap)), ap) })
            copy.sgroups.set(id, sgCopy)
        })
    }
    if (s.multitailArrows && s.multitailArrows.forEach) {
        s.multitailArrows.forEach(function(mta, id) {
            copy.multitailArrows.set(id, mta.clone ? mta.clone() : mta)
        })
    }
    if (s.rgroups && s.rgroups.forEach) {
        s.rgroups.forEach(function(rg, num) {
            copy.rgroups.set(num, rg.clone ? rg.clone() : rg)
        })
    }
    if (s.texts && s.texts.forEach) {
        s.texts.forEach(function(t, id) {
            copy.texts.set(id, t.clone ? t.clone() : t)
        })
    }
    if (s.images && s.images.forEach) {
        s.images.forEach(function(img, id) {
            copy.images.set(id, img.clone ? img.clone() : img)
        })
    }
    copy.stereoFlags = s.stereoFlags ? Object.assign({}, s.stereoFlags) : { type: 'abs', groupId: 0 }
    if (copy.initHalfBonds) { copy.initHalfBonds(); copy.initNeighbors(); copy.updateHalfBonds(); copy.sortNeighbors() }
    if (copy.markFragments) copy.markFragments()
    return copy
}

// ---- Public API: Lifecycle -------------------------------------------------

function init() {
    _struct = new CoreLib.ChemCore.Struct()
    _selection = { atom_ids: [], bond_ids: [], rxnArrow_ids: [], rxnPlus_ids: [], multitailArrow_ids: [], bbox: null }
    _history = []
    _historyPointer = -1
    _dirty = false
    _initialized = true
    _dragDelta = { x: 0, y: 0 }
    _dragSelection = []
    _dragArrowSelection = []
    _dragPlusSelection = []
}

// ---- Public API: Structure Manipulation ------------------------------------

function currentSelection() {
    return _selection
}

function selectItem(atomId, bondId, rxnArrowId, rxnPlusId, multitailArrowId) {
    if (atomId != null) {
        _selection = { atom_ids: [atomId], bond_ids: [], rxnArrow_ids: [], rxnPlus_ids: [], multitailArrow_ids: [], bbox: null }
    } else if (bondId != null) {
        _selection = { atom_ids: [], bond_ids: [bondId], rxnArrow_ids: [], rxnPlus_ids: [], multitailArrow_ids: [], bbox: null }
    } else if (rxnArrowId !== null && rxnArrowId !== undefined) {
        _selection = { atom_ids: [], bond_ids: [], rxnArrow_ids: [rxnArrowId], rxnPlus_ids: [], multitailArrow_ids: [], bbox: null }
    } else if (rxnPlusId !== null && rxnPlusId !== undefined) {
        _selection = { atom_ids: [], bond_ids: [], rxnArrow_ids: [], rxnPlus_ids: [rxnPlusId], multitailArrow_ids: [], bbox: null }
    } else if (multitailArrowId !== null && multitailArrowId !== undefined) {
        _selection = { atom_ids: [], bond_ids: [], rxnArrow_ids: [], rxnPlus_ids: [], multitailArrow_ids: [multitailArrowId], bbox: null }
    } else {
        _selection = { atom_ids: [], bond_ids: [], rxnArrow_ids: [], rxnPlus_ids: [], multitailArrow_ids: [], bbox: null }
    }
}

function addItemToSelection(atomId, bondId) {
    if (atomId !== null && atomId !== undefined) {
        if (_selection.atom_ids.indexOf(atomId) < 0)
            _selection.atom_ids = _selection.atom_ids.concat([atomId])
    } else if (bondId !== null && bondId !== undefined) {
        if (_selection.bond_ids.indexOf(bondId) < 0)
            _selection.bond_ids = _selection.bond_ids.concat([bondId])
    }
}

function removeItemFromSelection(atomId, bondId) {
    if (atomId !== null && atomId !== undefined) {
        _selection.atom_ids = _selection.atom_ids.filter(function(id) { return id !== atomId })
    } else if (bondId !== null && bondId !== undefined) {
        _selection.bond_ids = _selection.bond_ids.filter(function(id) { return id !== bondId })
    }
}

function selectFragment(atomId, bondId) {
    if (atomId === null && bondId !== null) {
        var bond = _struct.bonds.get(bondId)
        if (bond) atomId = bond.begin
    }

    if (atomId === null) {
        _selection = { atom_ids: [], bond_ids: [], rxnArrow_ids: [], rxnPlus_ids: [], multitailArrow_ids: [], bbox: null }
        return
    }

    // Resolve sgroup prim ID to a real atom inside it
    if (!_struct.atoms.get(atomId) && _struct.sgroups) {
        var sg0 = _struct.sgroups.get(atomId)
        if (sg0 && sg0.atoms && sg0.atoms.length > 0) {
            var seedId = (sg0.attachmentPoints && sg0.attachmentPoints.length > 0)
                ? sg0.attachmentPoints[0].atomId : sg0.atoms[0]
            atomId = seedId
        }
    }

    var atom = _struct.atoms.get(atomId)
    if (!atom) return
    
    var fid = atom.fragment
    if (fid === undefined || fid === -1) {
        // Fallback if markFragments wasn't called or failed
        _selection = { atom_ids: [atomId], bond_ids: [], rxnArrow_ids: [], rxnPlus_ids: [], multitailArrow_ids: [], bbox: null }
        return
    }
    
    var atomPile = _struct.getFragmentIds(fid)
    var aids = []
    if (atomPile && atomPile.values) {
        aids = Array.from(atomPile.values())
    } else if (atomPile && atomPile.forEach) {
        atomPile.forEach(function(val) { aids.push(val) })
    }
    
    // Promote contracted sgroup atoms → sgroup prim IDs
    var hiddenAtoms = {}
    if (_struct.sgroups) {
        _struct.sgroups.forEach(function(sg, sgId) {
            if (sg.type !== 'SUP' || (sg.isExpanded && sg.isExpanded())) return
            if (!sg.atoms) return
            var allIn = sg.atoms.every(function(a) { return aids.indexOf(a) >= 0 })
            if (allIn) {
                sg.atoms.forEach(function(a) { hiddenAtoms[a] = true })
                aids.push(sgId)
            }
        })
    }
    aids = aids.filter(function(id) { return !hiddenAtoms[id] })

    var aidSet = {}
    for (var i = 0; i < aids.length; i++) aidSet[aids[i]] = true

    var bids = []
    _struct.bonds.forEach(function(b, id) {
        if (!hiddenAtoms[b.begin] && !hiddenAtoms[b.end] && aidSet[b.begin] && aidSet[b.end])
            bids.push(id)
    })

    _selection = { atom_ids: aids, bond_ids: bids, rxnArrow_ids: [], rxnPlus_ids: [], multitailArrow_ids: [], bbox: null }
}

function selectAll() {
    var hiddenAtoms = {}
    var aids = []
    if (_struct.sgroups) {
        _struct.sgroups.forEach(function(sg, sgId) {
            if (sg.type !== 'SUP' || (sg.isExpanded && sg.isExpanded())) return
            aids.push(sgId)
            if (sg.atoms) sg.atoms.forEach(function(aid) { hiddenAtoms[aid] = true })
        })
    }
    var bids = []
    _struct.atoms.forEach(function(a, id) { if (!hiddenAtoms[id]) aids.push(id) })
    _struct.bonds.forEach(function(b, id) { if (!hiddenAtoms[b.begin] && !hiddenAtoms[b.end]) bids.push(id) })
    var raid = []
    if (_struct.rxnArrows) _struct.rxnArrows.forEach(function(ar, id) { raid.push(id) })
    var rpid = []
    if (_struct.rxnPluses) _struct.rxnPluses.forEach(function(pl, id) { rpid.push(id) })
    _selection = { atom_ids: aids, bond_ids: bids, rxnArrow_ids: raid, rxnPlus_ids: rpid, bbox: null }
}

function selectByRect(x1, y1, x2, y2) {
    var minX = Math.min(x1, x2), maxX = Math.max(x1, x2)
    var minY = Math.min(y1, y2), maxY = Math.max(y1, y2)

    // Build set of atoms hidden inside contracted sgroups
    var hiddenAtoms = {}
    var contractedSgroupIds = []
    if (_struct.sgroups) {
        _struct.sgroups.forEach(function(sg, sgId) {
            if (sg.type !== 'SUP' || (sg.isExpanded && sg.isExpanded())) return
            if (!sg.atoms || sg.atoms.length === 0) return
            // Representative position: attachment point atom or centroid
            var px = 0, py = 0
            var attachId = (sg.attachmentPoints && sg.attachmentPoints.length > 0) ? sg.attachmentPoints[0].atomId : null
            if (attachId !== null) {
                var aa = _struct.atoms.get(attachId)
                if (aa) { px = aa.pp.x; py = aa.pp.y }
            } else {
                sg.atoms.forEach(function(aid) { var ra = _struct.atoms.get(aid); if (ra) { px += ra.pp.x; py += ra.pp.y } })
                px /= sg.atoms.length; py /= sg.atoms.length
            }
            if (px >= minX && px <= maxX && py >= minY && py <= maxY) {
                contractedSgroupIds.push(sgId)
                sg.atoms.forEach(function(aid) { hiddenAtoms[aid] = true })
            } else {
                // Even if sgroup pill is outside, its atoms are hidden — exclude from free atom selection
                sg.atoms.forEach(function(aid) { hiddenAtoms[aid] = true })
            }
        })
    }

    var aids = contractedSgroupIds.slice()
    _struct.atoms.forEach(function(a, id) {
        if (hiddenAtoms[id]) return  // skip atoms inside contracted sgroups
        if (a.pp.x >= minX && a.pp.x <= maxX && a.pp.y >= minY && a.pp.y <= maxY) {
            aids.push(id)
        }
    })
    
    var aidSet = {}
    for (var i = 0; i < aids.length; ++i) aidSet[aids[i]] = true
    
    var _rc1 = {x: minX, y: minY}, _rc2 = {x: maxX, y: minY}
    var _rc3 = {x: maxX, y: maxY}, _rc4 = {x: minX, y: maxY}
    function lineIntersectsRect(p1, p2) {
        if (p1.x >= minX && p1.x <= maxX && p1.y >= minY && p1.y <= maxY) return true
        if (p2.x >= minX && p2.x <= maxX && p2.y >= minY && p2.y <= maxY) return true
        return CoreLib.ChemCore.Box2Abs.segmentIntersection(p1, p2, _rc1, _rc2) ||
               CoreLib.ChemCore.Box2Abs.segmentIntersection(p1, p2, _rc2, _rc3) ||
               CoreLib.ChemCore.Box2Abs.segmentIntersection(p1, p2, _rc3, _rc4) ||
               CoreLib.ChemCore.Box2Abs.segmentIntersection(p1, p2, _rc4, _rc1)
    }

    var bids = []
    _struct.bonds.forEach(function(b, id) {
        if (aidSet[b.begin] && aidSet[b.end]) {
            bids.push(id)
        } else {
            var a1 = _struct.atoms.get(b.begin)
            var a2 = _struct.atoms.get(b.end)
            if (a1 && a2 && lineIntersectsRect(a1.pp, a2.pp)) bids.push(id)
        }
    })
    var raid = []
    if (_struct.rxnArrows) {
        _struct.rxnArrows.forEach(function(ar, id) {
            var p1 = (ar.pos && ar.pos[0]) ? ar.pos[0] : (ar.p1 || {x:0,y:0})
            var p2 = (ar.pos && ar.pos[1]) ? ar.pos[1] : (ar.p2 || {x:0,y:0})
            if (lineIntersectsRect(p1, p2)) raid.push(id)
        })
    }
    var rpid = []
    if (_struct.rxnPluses) {
        _struct.rxnPluses.forEach(function(pl, id) {
            var px = pl.pp ? pl.pp.x : pl.x
            var py = pl.pp ? pl.pp.y : pl.y
            if (px >= minX && px <= maxX && py >= minY && py <= maxY) {
                rpid.push(id)
            }
        })
    }
    _selection = { atom_ids: aids, bond_ids: bids, rxnArrow_ids: raid, rxnPlus_ids: rpid, bbox: null }
}

function _pointInPolygon(px, py, poly) {
    var inside = false
    for (var i = 0, j = poly.length - 1; i < poly.length; j = i++) {
        var xi = poly[i].x, yi = poly[i].y
        var xj = poly[j].x, yj = poly[j].y
        var intersect = ((yi > py) !== (yj > py)) &&
            (px < (xj - xi) * (py - yi) / (yj - yi) + xi)
        if (intersect) inside = !inside
    }
    return inside
}

// Lasso select. ChemDraw semantics (confirmed, not a naive "touched" test): an
// atom is selected only if inside the freeform path, and a bond only if BOTH
// its atoms are -- entirely enclosed, no partial/crossing inclusion. Mirrors
// selectByRect's structure (sgroup-pill handling via representative point)
// but swaps the rectangle contains-test for point-in-polygon and drops
// selectByRect's own edge-crossing fallback for bonds/arrows, since that
// fallback is exactly the "touched" behavior ChemDraw's lasso does not use.
function selectByLasso(pointsFlat) {
    if (!pointsFlat || pointsFlat.length < 6) {
        _selection = { atom_ids: [], bond_ids: [], rxnArrow_ids: [], rxnPlus_ids: [], bbox: null }
        return
    }
    var poly = []
    for (var i = 0; i < pointsFlat.length; i += 2) poly.push({ x: pointsFlat[i], y: pointsFlat[i + 1] })

    var hiddenAtoms = {}
    var contractedSgroupIds = []
    if (_struct.sgroups) {
        _struct.sgroups.forEach(function(sg, sgId) {
            if (sg.type !== 'SUP' || (sg.isExpanded && sg.isExpanded())) return
            if (!sg.atoms || sg.atoms.length === 0) return
            var px = 0, py = 0
            var attachId = (sg.attachmentPoints && sg.attachmentPoints.length > 0) ? sg.attachmentPoints[0].atomId : null
            if (attachId !== null) {
                var aa = _struct.atoms.get(attachId)
                if (aa) { px = aa.pp.x; py = aa.pp.y }
            } else {
                sg.atoms.forEach(function(aid) { var ra = _struct.atoms.get(aid); if (ra) { px += ra.pp.x; py += ra.pp.y } })
                px /= sg.atoms.length; py /= sg.atoms.length
            }
            sg.atoms.forEach(function(aid) { hiddenAtoms[aid] = true })
            if (_pointInPolygon(px, py, poly)) contractedSgroupIds.push(sgId)
        })
    }

    var aids = contractedSgroupIds.slice()
    _struct.atoms.forEach(function(a, id) {
        if (hiddenAtoms[id]) return
        if (_pointInPolygon(a.pp.x, a.pp.y, poly)) aids.push(id)
    })

    var aidSet = {}
    for (var k = 0; k < aids.length; ++k) aidSet[aids[k]] = true

    var bids = []
    _struct.bonds.forEach(function(b, id) {
        if (aidSet[b.begin] && aidSet[b.end]) bids.push(id)
    })

    var raid = []
    if (_struct.rxnArrows) {
        _struct.rxnArrows.forEach(function(ar, id) {
            var p1 = (ar.pos && ar.pos[0]) ? ar.pos[0] : (ar.p1 || { x: 0, y: 0 })
            var p2 = (ar.pos && ar.pos[1]) ? ar.pos[1] : (ar.p2 || { x: 0, y: 0 })
            if (_pointInPolygon(p1.x, p1.y, poly) && _pointInPolygon(p2.x, p2.y, poly)) raid.push(id)
        })
    }
    var rpid = []
    if (_struct.rxnPluses) {
        _struct.rxnPluses.forEach(function(pl, id) {
            var px = pl.pp ? pl.pp.x : pl.x
            var py = pl.pp ? pl.pp.y : pl.y
            if (_pointInPolygon(px, py, poly)) rpid.push(id)
        })
    }
    _selection = { atom_ids: aids, bond_ids: bids, rxnArrow_ids: raid, rxnPlus_ids: rpid, bbox: null }
}

function addSelectionByRect(x1, y1, x2, y2) {
    var minX = Math.min(x1, x2), maxX = Math.max(x1, x2)
    var minY = Math.min(y1, y2), maxY = Math.max(y1, y2)

    var existingAtoms = {}
    _selection.atom_ids.forEach(function(id) { existingAtoms[id] = true })
    var existingBonds = {}
    _selection.bond_ids.forEach(function(id) { existingBonds[id] = true })

    var hiddenAtoms = {}
    if (_struct.sgroups) {
        _struct.sgroups.forEach(function(sg, sgId) {
            if (sg.type !== 'SUP' || (sg.isExpanded && sg.isExpanded())) return
            if (!sg.atoms || sg.atoms.length === 0) return
            var px = 0, py = 0
            var attachId = (sg.attachmentPoints && sg.attachmentPoints.length > 0) ? sg.attachmentPoints[0].atomId : null
            if (attachId !== null) {
                var aa = _struct.atoms.get(attachId)
                if (aa) { px = aa.pp.x; py = aa.pp.y }
            } else {
                sg.atoms.forEach(function(aid) { var ra = _struct.atoms.get(aid); if (ra) { px += ra.pp.x; py += ra.pp.y } })
                px /= sg.atoms.length; py /= sg.atoms.length
            }
            sg.atoms.forEach(function(aid) { hiddenAtoms[aid] = true })
            if (px >= minX && px <= maxX && py >= minY && py <= maxY) {
                if (!existingAtoms[sgId]) { _selection.atom_ids = _selection.atom_ids.concat([sgId]); existingAtoms[sgId] = true }
            }
        })
    }
    _struct.atoms.forEach(function(a, id) {
        if (hiddenAtoms[id]) return
        if (a.pp.x >= minX && a.pp.x <= maxX && a.pp.y >= minY && a.pp.y <= maxY) {
            if (!existingAtoms[id]) { _selection.atom_ids = _selection.atom_ids.concat([id]); existingAtoms[id] = true }
        }
    })
    _struct.bonds.forEach(function(b, id) {
        var a1 = _struct.atoms.get(b.begin)
        var a2 = _struct.atoms.get(b.end)
        if (a1 && a2 && a1.pp.x >= minX && a1.pp.x <= maxX && a1.pp.y >= minY && a1.pp.y <= maxY &&
                        a2.pp.x >= minX && a2.pp.x <= maxX && a2.pp.y >= minY && a2.pp.y <= maxY) {
            if (!existingBonds[id]) { _selection.bond_ids = _selection.bond_ids.concat([id]); existingBonds[id] = true }
        }
    })
}

function deleteSelection() {
    var atomsData = []
    var bondsData = []
    var arrowsData = []
    var plusesData = []
    var multitailData = []
    var sgroupsData = []

    // Expand sgroup prim IDs to real atoms; collect sgroups to delete
    var resolvedAtomIds = _resolveAtomMoveIds(_selection.atom_ids)
    resolvedAtomIds.forEach(function(aid) {
        var a = _struct.atoms.get(aid)
        if (a) atomsData.push({ id: aid, atom: a })
    })
    _selection.atom_ids.forEach(function(aid) {
        if (_struct.sgroups && !_struct.atoms.get(aid)) {
            var sg = _struct.sgroups.get(aid)
            if (sg) sgroupsData.push({ id: aid, sg: sg })
        }
    })

    _struct.bonds.forEach(function(b, bid) {
        var deleteIt = false
        if (_selection.bond_ids.indexOf(bid) >= 0) deleteIt = true
        if (resolvedAtomIds.indexOf(b.begin) >= 0 || resolvedAtomIds.indexOf(b.end) >= 0) deleteIt = true
        if (deleteIt) {
            bondsData.push({ id: bid, bond: b })
        }
    })
    
    if (_selection.rxnArrow_ids) {
        _selection.rxnArrow_ids.forEach(function(arId) {
            var ar = _struct.rxnArrows.get(arId)
            if (ar) arrowsData.push({ id: arId, arrow: ar })
        })
    }
    if (_selection.rxnPlus_ids) {
        _selection.rxnPlus_ids.forEach(function(plId) {
            var pl = _struct.rxnPluses.get(plId)
            if (pl) plusesData.push({ id: plId, plus: pl })
        })
    }
    if (_selection.multitailArrow_ids) {
        _selection.multitailArrow_ids.forEach(function(mtaId) {
            var mta = _struct.multitailArrows.get(mtaId)
            if (mta) multitailData.push({ id: mtaId, mta: mta })
        })
    }

    // As in deleteAtomById: an atom being deleted here may be an individually-selected
    // member of an *expanded* sgroup (as opposed to sgroupsData above, which only
    // covers deleting a whole contracted pill) -- its id must come out of that
    // sgroup's own atoms list too, or the sgroup keeps referencing a deleted atom.
    // Grouped per-sgroup since a multi-select can remove several members of the same
    // group at once; restores the correct member *set* on undo, not necessarily each
    // atom's exact original position in the array (not meaningful here -- sgroup.atoms
    // is read elsewhere by membership, never by position).
    var sgroupAtomRemovals = {}
    atomsData.forEach(function(ad) {
        if (!ad.atom.sgs || ad.atom.sgs.size === 0) return
        ad.atom.sgs.forEach(function(sgId) {
            var sg = _struct.sgroups.get(sgId)
            if (!sg || !sg.atoms || sg.atoms.indexOf(ad.id) < 0) return
            if (!sgroupAtomRemovals[sgId]) sgroupAtomRemovals[sgId] = { sg: sg, removedIds: [] }
            sgroupAtomRemovals[sgId].removedIds.push(ad.id)
        })
    })
    var sgroupAtomRemovalList = Object.keys(sgroupAtomRemovals).map(function(k) { return sgroupAtomRemovals[k] })

    var cmd = makeCmd(
        function() {
            bondsData.forEach(function(bd) { _struct.bonds.delete(bd.id) })
            sgroupAtomRemovalList.forEach(function(m) {
                m.sg.atoms = m.sg.atoms.filter(function(id) { return m.removedIds.indexOf(id) < 0 })
            })
            atomsData.forEach(function(ad) { _struct.atoms.delete(ad.id) })
            sgroupsData.forEach(function(sd) { if (_struct.sgroups) _struct.sgroups.delete(sd.id) })
            arrowsData.forEach(function(ad) { _struct.rxnArrows.delete(ad.id) })
            plusesData.forEach(function(pd) { _struct.rxnPluses.delete(pd.id) })
            multitailData.forEach(function(md) { _struct.multitailArrows.delete(md.id) })
            _struct.initHalfBonds()
            _struct.initNeighbors()
            _struct.updateHalfBonds()
            _struct.sortNeighbors()
        },
        function() {
            atomsData.forEach(function(ad) { _struct.atoms.set(ad.id, ad.atom) })
            sgroupAtomRemovalList.forEach(function(m) {
                m.removedIds.forEach(function(id) { m.sg.atoms.push(id) })
            })
            bondsData.forEach(function(bd) { _struct.bonds.set(bd.id, bd.bond) })
            sgroupsData.forEach(function(sd) { if (_struct.sgroups) _struct.sgroups.set(sd.id, sd.sg) })
            arrowsData.forEach(function(ad) { _struct.rxnArrows.set(ad.id, ad.arrow) })
            plusesData.forEach(function(pd) { _struct.rxnPluses.set(pd.id, pd.plus) })
            multitailData.forEach(function(md) { _struct.multitailArrows.set(md.id, md.mta) })
            _struct.initHalfBonds()
            _struct.initNeighbors()
            _struct.updateHalfBonds()
            _struct.sortNeighbors()
        }
    )

    executeCommand(cmd)
    _selection = { atom_ids: [], bond_ids: [], rxnArrow_ids: [], rxnPlus_ids: [], multitailArrow_ids: [], bbox: null }
}

// ---- Public API: Undo / Redo -----------------------------------------------

function undo() {
    if (_historyPointer < 0) return
    var cmd = _history[_historyPointer]
    cmd.invert()
    _historyPointer--
    _selection = { atom_ids: [], bond_ids: [], rxnArrow_ids: [], rxnPlus_ids: [], multitailArrow_ids: [], bbox: null }
    _dragDelta = { x: 0, y: 0 }
    _dragSelection = []
    _dragArrowSelection = []
    _dragPlusSelection = []
    _dirty = true
}

function redo() {
    if (_historyPointer >= _history.length - 1) return
    _historyPointer++
    var cmd = _history[_historyPointer]
    cmd.execute()
    _selection = { atom_ids: [], bond_ids: [], rxnArrow_ids: [], rxnPlus_ids: [], multitailArrow_ids: [], bbox: null }
    _dirty = true
}

function isDirty() {
    return _dirty
}

function canUndo() {
    return _historyPointer >= 0
}

function canRedo() {
    return _historyPointer < _history.length - 1
}

// ---- Public API: Render Primitives -----------------------------------------

function buildRenderPrimitives(showExplicitH) {
    if (_struct.initHalfBonds) _struct.initHalfBonds()
    if (_struct.initNeighbors) _struct.initNeighbors()
    if (_struct.markFragments) _struct.markFragments()
    if (_struct.updateHalfBonds) _struct.updateHalfBonds()
    if (_struct.sortNeighbors) _struct.sortNeighbors()
    if (_struct.findLoops) _struct.findLoops()
    if (showExplicitH && _struct.setImplicitHydrogen) _struct.setImplicitHydrogen()
    if (_struct.setStereoLabelsToAtoms) _struct.setStereoLabelsToAtoms()

    // ---- Contracted sgroup detection ----------------------------------------
    // Build maps before iterating atoms/bonds so we can skip hidden internals.
    var contractedSgroups = {}   // sgId → { id, label, x, y, atomIds: Set, attachAtomId }
    var atomInSgroup = {}        // atomId → sgId  (for atoms inside a contracted sgroup)

    if (_struct.sgroups) {
        _struct.sgroups.forEach(function(sg, sgId) {
            if (sg.type !== 'SUP') return
            if (sg.isExpanded && sg.isExpanded()) return
            var label = (sg.data && sg.data.name) ? sg.data.name : ''
            if (!label) return

            // Position: attachment point atom, falling back to centroid
            var attachAtomId = null
            if (sg.attachmentPoints && sg.attachmentPoints.length > 0)
                attachAtomId = sg.attachmentPoints[0].atomId
            var px = 0, py = 0
            if (attachAtomId !== null) {
                var aa = _struct.atoms.get(attachAtomId)
                if (aa) { px = aa.pp.x; py = aa.pp.y }
            } else {
                var n = 0
                sg.atoms.forEach(function(aid) {
                    var a = _struct.atoms.get(aid)
                    if (a) { px += a.pp.x; py += a.pp.y; n++ }
                })
                if (n > 0) { px /= n; py /= n }
            }

            var atomSet = {}
            sg.atoms.forEach(function(aid) { atomSet[aid] = true; atomInSgroup[aid] = sgId })

            contractedSgroups[sgId] = {
                id: sgId,
                label: label,
                x: px, y: py,
                atomIds: atomSet,
                attachAtomId: attachAtomId,
                isSgroup: true,
                charge: 0,
                stereoLabel: '',
                cipLabel: '',
                isRGroup: false,
                isotope: 0,
                radical: 0,
                explicitValence: -1,
                implicitHCount: 0,
                hOnLeft: false,
                element: '',
                color: '#202020',
                aam: 0,
                attachmentPoints: 0,
                checkWarning: ''
            }
        })
    }
    // -------------------------------------------------------------------------

    var atomsArray = []
    var atomsById = {}
    
    var minX = null, maxX = null, minY = null, maxY = null
    
    _struct.atoms.forEach(function(a, id) {
        // Skip atoms hidden inside a contracted sgroup
        if (atomInSgroup[id] !== undefined) return

        var isAtomList = (a.label === "L#" && a.atomList)
        var renderLabel = a.label

        // Smart implicit H logic
        // 1. Heteroatoms always show their H's
        // 2. Carbons only show H's if showExplicitH is true, or if they are a terminal carbon (only 1 neighbor)
        var isHetero = (a.label !== "C" && a.label !== "H")
        var isTerminal = (a.neighbors && a.neighbors.length <= 1)
        var rgroupFlag = isRGroupLabel(a.label)
        if (rgroupFlag) { isHetero = false; isTerminal = false }
        if (isAtomList) { isHetero = false; isTerminal = false }

        if (isAtomList) {
            var listLabels = a.atomList.ids.map(function(num) {
                var el = CoreLib.ChemCore.Elements.get(num)
                return el ? el.label : "?"
            })
            renderLabel = (a.atomList.notList ? "!" : "") + "[" + listLabels.join(",") + "]"
        } else if ((showExplicitH || isHetero || isTerminal) && a.implicitH > 0) {
            if (a.implicitH === 1) renderLabel += "H"
            else renderLabel += "H" + a.implicitH
        }

        var elemData = CoreLib.ChemCore.Elements.get(a.label)

        // Compute hOnLeft: place H on left when neighbours are predominantly to the right
        var nbXSum = 0, nbCount = 0
        if (a.neighbors) {
            a.neighbors.forEach(function(nb) {
                var na = _struct.atoms.get(nb.aid)
                if (na) { nbXSum += na.pp.x; nbCount++ }
            })
        }

        var prim = {
            id: id,
            x: a.pp.x,
            y: a.pp.y,
            label: renderLabel,
            element: a.label,
            charge: a.charge || 0,
            stereoLabel: a.stereoLabel || "",
            cipLabel: a.cipLabel || "",
            stereoType: a.stereoType || 0,
            stereoGroup: a.stereoGroup || 0,
            color: rgroupFlag ? "#7B68EE" : (CoreLib.ChemCore.ElementColor[a.label] || "#202020"),
            isRGroup: rgroupFlag,
            atomicNum: elemData ? elemData.number : 0,
            atomicTitle: (elemData && elemData.title) ? elemData.title : "",
            atomicMass: elemData ? (Math.round(elemData.mass * 1000) / 1000) : 0,
            implicitHCount: Math.max(0, a.implicitH || 0),
            hOnLeft: nbCount > 0 && (nbXSum / nbCount) > a.pp.x,
            isotope: a.isotope || 0,
            radical: a.radical || 0,
            explicitValence: (a.explicitValence !== undefined && a.explicitValence >= 0) ? a.explicitValence : -1,
            aam: a.aam || 0,
            attachmentPoints: a.attachmentPoints || 0,
            checkWarning: a.checkWarning || "",
            isAtomList: !!isAtomList,
            atomListElements: isAtomList ? a.atomList.ids.map(function(num) {
                var el = CoreLib.ChemCore.Elements.get(num)
                return el ? el.label : "?"
            }).join(",") : "",
            atomListNot: isAtomList ? !!a.atomList.notList : false
        }
        atomsArray.push(prim)
        atomsById[id] = prim

        if (minX === null || a.pp.x < minX) minX = a.pp.x
        if (maxX === null || a.pp.x > maxX) maxX = a.pp.x
        if (minY === null || a.pp.y < minY) minY = a.pp.y
        if (maxY === null || a.pp.y > maxY) maxY = a.pp.y
    })

    // Inject contracted sgroup prims — they appear in atomsArray and atomsById
    // so that existing bond rendering and label rendering handle them automatically.
    var sgroupsArray = []
    Object.keys(contractedSgroups).forEach(function(sgId) {
        var csg = contractedSgroups[sgId]
        atomsArray.push(csg)
        atomsById[sgId] = csg
        sgroupsArray.push(csg)
        if (minX === null || csg.x < minX) minX = csg.x
        if (maxX === null || csg.x > maxX) maxX = csg.x
        if (minY === null || csg.y < minY) minY = csg.y
        if (maxY === null || csg.y > maxY) maxY = csg.y
    })

    var aromaticBondIds = {}
    // Maps bond id -> {x,y} ring center, consumed by MoleculeLayer.qml to offset
    // double/triple bond lines toward the ring interior instead of a fixed side
    // that only happens to look right when every ring shares the same winding
    // order addRing() happens to produce (e.g. library-template rings like
    // Pyridine, or freehand-drawn rings, can wind either way).
    var bondRingCenter = {}
    var ringsArray = []
    if (_struct.loops) {
        _struct.loops.forEach(function(l, id) {
            var atomIds = []
            var isAromatic = false
            var hasBondType4 = false
            var hbids = l.hbs || []
            hbids.forEach(function(hbid) {
                var hb = _struct.halfBonds.get(hbid)
                if (hb) {
                    atomIds.push(hb.begin)
                    var bond = _struct.bonds.get(hb.bid)
                    if (bond && bond.type === 4) { isAromatic = true; hasBondType4 = true }
                }
            })
            
            // Actually, in sketch Benzene adds bonds of type 1 and 2. Let's just say a 6-membered ring of alternating double/single bonds is aromatic.
            if (!isAromatic && atomIds.length === 6) {
                var dbCount = 0
                hbids.forEach(function(hbid) {
                    var hb = _struct.halfBonds.get(hbid)
                    if (hb) {
                        var bond = _struct.bonds.get(hb.bid)
                        if (bond && bond.type === 2) dbCount++
                    }
                })
                if (dbCount === 3) isAromatic = true
            }

            if (isAromatic) {
                hbids.forEach(function(hbid) {
                    var hb = _struct.halfBonds.get(hbid)
                    if (hb) aromaticBondIds[hb.bid] = true
                })
            }

            if (atomIds.length > 0) {
                var cx = 0, cy = 0
                atomIds.forEach(function(aid) {
                    var a = _struct.atoms.get(aid)
                    if (a) { cx += a.pp.x; cy += a.pp.y }
                })
                cx /= atomIds.length
                cy /= atomIds.length

                hbids.forEach(function(hbid) {
                    var hb = _struct.halfBonds.get(hbid)
                    if (hb && bondRingCenter[hb.bid] === undefined) {
                        bondRingCenter[hb.bid] = { x: cx, y: cy }
                    }
                })

                var a0 = _struct.atoms.get(atomIds[0])
                var radius = 0
                if (a0) {
                    var dx = a0.pp.x - cx
                    var dy = a0.pp.y - cy
                    radius = Math.sqrt(dx*dx + dy*dy)
                }

                ringsArray.push({
                    id: id,
                    atoms: atomIds,
                    x: cx,
                    y: cy,
                    radius: radius,
                    isAromatic: isAromatic,
                    // True aromatic bond-order rings (Indigo Aromatize, loaded aromatic
                    // SMILES) have no Kekule structure to depict, so they render an
                    // inscribed circle. Rings only inferred aromatic via the alternating-
                    // bond heuristic already show it via those bonds — no circle needed.
                    hasBondType4: hasBondType4
                })
            }
        })
    }

    var bondsArray = []
    _struct.bonds.forEach(function(b, id) {
        var beginSgId = atomInSgroup[b.begin]
        var endSgId   = atomInSgroup[b.end]

        // Skip bonds fully inside a contracted sgroup
        if (beginSgId !== undefined && beginSgId === endSgId) return

        // For cross-bonds, replace the sgroup-side endpoint with the sgroup prim id
        var begin = (beginSgId !== undefined) ? parseInt(beginSgId) : b.begin
        var end   = (endSgId   !== undefined) ? parseInt(endSgId)   : b.end

        var invalidStereo = false
        if (b.stereo > 0 && _struct.atomGetNeighbors && beginSgId === undefined && endSgId === undefined) {
            var bNeighs = _struct.atomGetNeighbors(b.begin)
            var eNeighs = _struct.atomGetNeighbors(b.end)
            invalidStereo = !CoreLib.ChemCore.StereoValidator.isCorrectStereoCenter(b, bNeighs, eNeighs, _struct)
        }
        var ringCenter = bondRingCenter[id]
        bondsArray.push({
            id: id,
            begin: begin,
            end: end,
            type: b.type,
            stereo: (beginSgId !== undefined || endSgId !== undefined) ? 0 : (b.stereo || 0),
            inAromaticRing: aromaticBondIds[id] || false,
            invalidStereo: invalidStereo,
            checkWarning: b.checkWarning || "",
            beginIsSgroup: beginSgId !== undefined,
            endIsSgroup: endSgId !== undefined,
            cipLabel: b.cipLabel || "",
            reactingCenterStatus: b.reactingCenterStatus || 0,
            ringCenterX: ringCenter !== undefined ? ringCenter.x : undefined,
            ringCenterY: ringCenter !== undefined ? ringCenter.y : undefined
        })
    })

    var rxnArrowsArray = []
    if (_struct.rxnArrows) {
        _struct.rxnArrows.forEach(function(arr, id) {
            if (arr.pos && arr.pos.length >= 2) {
                rxnArrowsArray.push({
                    id: id,
                    p1: { x: arr.pos[0].x, y: arr.pos[0].y },
                    p2: { x: arr.pos[1].x, y: arr.pos[1].y },
                    mode: arr.mode || "filled-triangle",
                    conditionsText: arr.conditionsText || { above: "", below: "" },
                    curvature: arr.curvature || null
                })
            }
        })
    }

    var rxnPlusesArray = []
    if (_struct.rxnPluses) {
        _struct.rxnPluses.forEach(function(plus, id) {
            if (plus.pp) {
                rxnPlusesArray.push({
                    id: id,
                    x: plus.pp.x,
                    y: plus.pp.y
                })
            }
        })
    }

    var multitailArrowsArray = []
    if (_struct.multitailArrows) {
        _struct.multitailArrows.forEach(function(mta, id) {
            var tails = []
            // Top tail
            tails.push({ x: mta.spineTopX - mta.tailLength, y: mta.spineTopY })
            // Inner tails
            if (mta.tailsYOffset) {
                mta.tailsYOffset.forEach(function(yOff) {
                    tails.push({ x: mta.spineTopX - mta.tailLength, y: mta.spineTopY + yOff })
                })
            }
            // Bottom tail
            tails.push({ x: mta.spineTopX - mta.tailLength, y: mta.spineTopY + mta.height })
            multitailArrowsArray.push({
                id: id,
                spineTopX: mta.spineTopX,
                spineTopY: mta.spineTopY,
                height: mta.height,
                tailLength: mta.tailLength,
                headOffsetX: mta.headOffsetX,
                headOffsetY: mta.headOffsetY,
                tails: tails
            })
        })
    }

    return {
        atoms: atomsArray,
        bonds: bondsArray,
        rings: ringsArray,
        sgroups: sgroupsArray,
        rxnArrows: rxnArrowsArray,
        rxnPluses: rxnPlusesArray,
        multitailArrows: multitailArrowsArray,
        bbox: minX !== null ? { minX: minX, minY: minY, maxX: maxX, maxY: maxY } : null,
        atomsById: atomsById,
        stereoFlags: _struct.stereoFlags || { type: 'abs', groupId: 0 },
        texts: (function() {
            var out = []
            if (_struct.texts) _struct.texts.forEach(function(t, id) {
                out.push({ id: id, x: t.position.x, y: t.position.y, content: _plainFromTextJson(t.content || "") })
            })
            return out
        })(),
        images: (function() {
            var out = []
            if (_struct.images) _struct.images.forEach(function(img, id) {
                var tl = img.getTopLeftPosition ? img.getTopLeftPosition() : img._center.sub(img.halfSize)
                out.push({ id: id, x: tl.x, y: tl.y, w: img.halfSize.x * 2, h: img.halfSize.y * 2, bitmap: img.bitmap })
            })
            return out
        })(),
        rgroups: (function() {
            var out = []
            if (_struct.rgroups) {
                _struct.rgroups.forEach(function(rg, num) {
                    var members = []
                    rg.frags.forEach(function(fid) {
                        var pile = _struct.getFragmentIds ? _struct.getFragmentIds(fid) : null
                        var aids = []
                        if (pile && pile.values) aids = Array.from(pile.values())
                        else if (pile && pile.forEach) pile.forEach(function(v) { aids.push(v) })
                        members.push({ fragId: fid, atomIds: aids })
                    })
                    out.push({ number: num, range: rg.range || "", resth: !!rg.resth, ifthen: rg.ifthen || 0, members: members })
                })
            }
            out.sort(function(a, b) { return a.number - b.number })
            return out
        })()
    }
}

// ---- Demo Molecule: Benzene ------------------------------------------------

