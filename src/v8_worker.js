// ---- Engine shims ----------------------------------------------------------
// Under Node.js, __native is absent; the real fs/path/process globals are used
// instead (see the fallback branch below). Under the embedded QuickJS engine,
// V8Process's bridge binds a global `__native` object with these methods
// before this file is eval'd, and there is no fs/path/process/console at all.
const _hasNative = typeof __native !== "undefined";

const fs = _hasNative
    ? { readFileSync: function(p) { return __native.readFileSync(p); },
        existsSync: function(p) { return __native.existsSync(p); } }
    : require("fs");
const path = _hasNative
    ? { join: function() { return Array.prototype.slice.call(arguments).join("/"); } }
    : require("path");
// Not named __dirname on purpose: Node wraps this whole file in one function
// with __dirname as a parameter, so a `var __dirname` anywhere in the file
// (even unexecuted) would hoist and shadow that parameter for every reference
// in the file, not just after the point of declaration.
const _workerDir = _hasNative ? __native.dirname : __dirname;

// Same reasoning rules out `var console =`/`var process =` here: hoisting would
// shadow Node's real globals to `undefined` everywhere else in this file, even
// on the Node path where this assignment never runs. Plain property writes on
// globalThis carry no hoisting risk and are a no-op when _hasNative is false.
if (_hasNative) {
    globalThis.console = {
        log: function(s) { __native.log(String(s)); },
        warn: function() { __native.log(Array.prototype.slice.call(arguments).map(String).join(" ")); },
        error: function() { __native.errorLog(Array.prototype.slice.call(arguments).map(String).join(" ")); }
    };
    globalThis.process = {
        stderr: { write: function(s) { __native.errorLog(String(s)); } },
        stdout: { write: function(s) { __native.log(String(s)); } },
        exit: function(code) { __native.fatal("worker exited with code " + code); },
        on: function() {} // uncaughtException has no meaning in-process; dispatch() below wraps every call in try/catch instead
    };
}

// Load chem-core.js
let CoreLib = {};
try {
    const chemCorePath = path.join(_workerDir, "..", "chem-core.js");
    const chemCoreCode = fs.readFileSync(chemCorePath, "utf8").replace(/\.pragma library\s*/, "");
    eval(chemCoreCode + "\nCoreLib.ChemCore = ChemCore;");
} catch (e) {
    process.stderr.write("FATAL: Failed to load chem-core.js: " + e.message + "\n");
    process.exit(1);
}

var _sdfBatchRecords = [];
var _sdfProps = {};

// ============================================================================
// ---- Functional Group Library (loaded once at startup) ---------------------

var _fgStructs = {}
var _saltsStructs = {}
var _libraryStructs = {}
var _fgMeta = {}       // name -> { group }
var _libraryMeta = {}  // name -> { group }

;(function loadTemplates() {
    function loadSdf(relPath, target) {
        var fullPath = path.join(_workerDir, "..", relPath)
        if (!fs.existsSync(fullPath)) return
        try {
            var items = new CoreLib.ChemCore.SdfSerializer().deserialize(fs.readFileSync(fullPath, "utf8"))
            items.forEach(function(item) {
                if (item.struct && item.struct.name) target[item.struct.name] = item.struct
            })
        } catch (e) {
            process.stderr.write("Warning: could not load " + relPath + ": " + e.message + "\n")
        }
    }
    function parseGroupMeta(relPath, target) {
        var fullPath = path.join(_workerDir, "..", relPath)
        if (!fs.existsSync(fullPath)) return
        try {
            var content = fs.readFileSync(fullPath, "utf8")
            content.split("$$$$").forEach(function(entry) {
                entry = entry.trim()
                if (!entry) return
                var name = entry.split("\n")[0].trim()
                if (!name) return
                var m = entry.match(/>  <group>\s*\n([^\n]+)/)
                target[name] = { group: m ? m[1].trim() : '' }
                // Ketcher's template-fusion attachment metadata: atomid = which atom
                // aligns when dropping onto an existing atom, bondid = which bond
                // aligns when dropping onto an existing bond. The two are independent
                // references (the atom need not be an endpoint of the bond).
                // 0-based, mapping directly onto Pool ids: verified against all 235
                // carrying templates (every value in range 0..n-1, and 104 templates
                // use the value 0, which a 1-based scheme could not produce).
                var am = entry.match(/>  <atomid>\s*\n(\d+)/)
                var bm = entry.match(/>  <bondid>\s*\n(\d+)/)
                if (am) target[name].atomIdx = parseInt(am[1], 10)
                if (bm) target[name].bondIdx = parseInt(bm[1], 10)
            })
        } catch (e) {}
    }
    loadSdf("templates/fg.sdf", _fgStructs)
    CoreLib.ChemCore.FunctionalGroupsProvider.getInstance().setFunctionalGroupsList(Object.values(_fgStructs))
    parseGroupMeta("templates/fg.sdf", _fgMeta)
    loadSdf("templates/salts-and-solvents.sdf", _saltsStructs)
    CoreLib.ChemCore.SaltsAndSolventsProvider.getInstance().setSaltsAndSolventsList(Object.values(_saltsStructs))
    loadSdf("templates/library.sdf", _libraryStructs)
    parseGroupMeta("templates/library.sdf", _libraryMeta)
    // Defensive validation: clear fusion indices that don't resolve against the
    // parsed struct, so malformed metadata degrades to plain (non-fused)
    // placement instead of failing later inside insertLibraryTemplateFused.
    Object.keys(_libraryMeta).forEach(function(name) {
        var meta = _libraryMeta[name]
        var st = _libraryStructs[name]
        if (!st) { delete meta.atomIdx; delete meta.bondIdx; return }
        if (meta.atomIdx !== undefined && st.atoms.get(meta.atomIdx) === undefined) delete meta.atomIdx
        if (meta.bondIdx !== undefined && st.bonds.get(meta.bondIdx) === undefined) delete meta.bondIdx
    })
})()

// ---- Atom Label Validator --------------------------------------------------

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
    if (_selection.atom_ids.length === 0 && _selection.rxnArrow_ids.length === 0 && _selection.rxnPlus_ids.length === 0 && _selection.multitailArrow_ids.length === 0) return
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

var _clipboard = null

function copySelection() {
    if (_selection.atom_ids.length === 0) return null
    try {
        var realAtomIds = _resolveAtomMoveIds(_selection.atom_ids)
        var atomSet = new CoreLib.ChemCore.Pile(realAtomIds)
        var bondSet = new CoreLib.ChemCore.Pile(_selection.bond_ids)
        var subStruct = _struct.clone(atomSet, bondSet)
        _clipboard = subStruct
        return "cloned"
    } catch (e) {
        return null
    }
}

// KetSerializer.serializeMicromolecules (chem-core.js) walks struct.multitailArrows
// directly and calls .toKetNode() on each entry, assuming a real chem-core
// MultitailArrow class instance -- this app's own multitail arrows
// (_makeMultitailArrow) are plain data objects with no such method, so
// serializeMicromolecules throws the instant one exists (confirmed via a
// standalone repro: "TypeError: multitailArrow.toKetNode is not a function").
// Swap in an empty pool for just the call's duration; every caller already
// re-injects the correct KET nodes afterward via _injectMultitailArrows, built
// directly from the plain-object data, so nothing is lost.
function _serializeMicromoleculesSafe(struct) {
    var saved = struct.multitailArrows
    struct.multitailArrows = new CoreLib.ChemCore.Pool()
    try {
        return new CoreLib.ChemCore.KetSerializer().serializeMicromolecules(struct)
    } finally {
        struct.multitailArrows = saved
    }
}

function getClipboardAsKet() {
    var ketStr = ""
    if (_clipboard && CoreLib.ChemCore.KetSerializer) {
        try {
            // serializeMicromolecules already returns a JSON string, not an object
            var ketObj = JSON.parse(_serializeMicromoleculesSafe(_clipboard))
            _injectArrowExtras(ketObj, _clipboard)
            _injectMultitailArrows(ketObj, _clipboard)
            ketStr = JSON.stringify(ketObj)
        } catch (e) { ketStr = "" }
    }
    console.log(JSON.stringify({ type: "structureResponse", reqId: "clipboard_ket", data: ketStr }))
}

function importKetAtPosition(ketStr, cx, cy) {
    try {
        var ketObj = JSON.parse(ketStr)
        var extras = _extractArrowExtras(ketObj)
        var cleanedData = JSON.stringify(ketObj)
        var tempStruct = _deserializeStruct("ket", cleanedData)
        if (!tempStruct) return
        _reattachArrowExtras(tempStruct, extras)
        var oldClipboard = _clipboard
        _clipboard = tempStruct
        pasteSelection(cx, cy)
        _clipboard = oldClipboard
    } catch (e) {
        console.warn("importKetAtPosition failed:", e.message)
    }
}

function cutSelection() {
    var c = copySelection()
    if (c) {
        deleteSelection()
    }
    return c
}

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

function getMoleculeName() {
    console.log(JSON.stringify({ type: "structureResponse", reqId: "mol_name", data: _struct.name || "" }));
}

function setMoleculeName(name) {
    var oldName = _struct.name || "";
    var newName = name || "";
    if (oldName === newName) return;
    var cmd = makeCmd(
        function() { _struct.name = newName; _dirty = true; },
        function() { _struct.name = oldName; _dirty = true; }
    );
    executeCommand(cmd);
}


function setStereoDescriptors(jsonMap) {
    try {
        var map = JSON.parse(jsonMap)
    } catch (e) {
        return
    }
    var atomMap = map.atoms || {}
    var bondMap = map.bonds || {}

    // Build atom ID list in pool iteration order (matches MolSerializer atom numbering)
    var atomIds = []
    _struct.atoms.forEach(function(a, id) { atomIds.push(id) })

    // Reset all cipLabel fields
    _struct.atoms.forEach(function(a) { a.cipLabel = ""; a.stereoType = 0; a.stereoGroup = 0 })
    _struct.bonds.forEach(function(b) { b.cipLabel = "" })

    // Apply atom-based descriptors (R/S/r/s)
    Object.keys(atomMap).forEach(function(idx) {
        var atomIdx = parseInt(idx)
        if (atomIdx >= 0 && atomIdx < atomIds.length) {
            var atomId = atomIds[atomIdx]
            var a = _struct.atoms.get(atomId)
            if (a) {
                var entry = atomMap[idx]
                a.cipLabel = entry.cipLabel || ""
                a.stereoType = entry.type || 0
                a.stereoGroup = entry.group || 0
            }
        }
    })

    // Apply bond-based descriptors (E/Z), keyed "minAtomIdx-maxAtomIdx"
    Object.keys(bondMap).forEach(function(key) {
        var parts = key.split("-")
        var idx1 = parseInt(parts[0]), idx2 = parseInt(parts[1])
        if (idx1 < 0 || idx1 >= atomIds.length || idx2 < 0 || idx2 >= atomIds.length) return
        var atomId1 = atomIds[idx1], atomId2 = atomIds[idx2]
        var bondId = _struct.bonds.find(function(bid, b) {
            return (b.begin === atomId1 && b.end === atomId2) || (b.begin === atomId2 && b.end === atomId1)
        })
        if (bondId === null) return
        var b = _struct.bonds.get(bondId)
        if (b) b.cipLabel = bondMap[key].cipLabel || ""
    })

    // Output state update with result="stereoUpdated" to avoid re-triggering propUpdateTimer
    var state = buildRenderPrimitives(_showExplicitH)
    var selection = currentSelection()
    console.log(JSON.stringify({
        status: "ok",
        state: state,
        selection: selection,
        isDirty: isDirty(),
        canUndo: canUndo(),
        canRedo: canRedo(),
        result: "stereoUpdated"
    }))
}

function setCheckIssues(jsonMap) {
    try {
        var parsed = JSON.parse(jsonMap)
    } catch (e) {
        return
    }
    var issues = parsed.issues || []
    // Build atom/bond ID lists in pool iteration order (matches MolSerializer numbering)
    var atomIds = []
    _struct.atoms.forEach(function(a, id) { atomIds.push(id) })
    var bondIds = []
    _struct.bonds.forEach(function(b, id) { bondIds.push(id) })

    // Reset all checkWarning fields
    _struct.atoms.forEach(function(a) { a.checkWarning = "" })
    _struct.bonds.forEach(function(b) { b.checkWarning = "" })

    // Apply issues
    issues.forEach(function(issue) {
        var target = issue.target || "atom"
        var type = issue.type || ""
        var ids = issue.ids || []
        ids.forEach(function(idx) {
            if (target === "atom") {
                if (idx >= 0 && idx < atomIds.length) {
                    var a = _struct.atoms.get(atomIds[idx])
                    if (a) a.checkWarning = type
                }
            } else {
                if (idx >= 0 && idx < bondIds.length) {
                    var b = _struct.bonds.get(bondIds[idx])
                    if (b) b.checkWarning = type
                }
            }
        })
    })

    var state = buildRenderPrimitives(_showExplicitH)
    var selection = currentSelection()
    console.log(JSON.stringify({
        status: "ok",
        state: state,
        selection: selection,
        isDirty: isDirty(),
        canUndo: canUndo(),
        canRedo: canRedo(),
        result: "checkUpdated"
    }))
}

function pasteSelection(cx, cy) {
    if (!_clipboard) return
    _insertStructAt(_clipboard, cx, cy)
}

function insertRecognizedStructure(molfile, cx, cy) {
    try {
        var recognized = _deserializeStruct("mol", molfile)
        if (!recognized || recognized.atoms.size === 0) {
            console.warn("insertRecognizedStructure: empty or unparseable molfile")
            return
        }
        _insertStructAt(recognized, cx, cy)
    } catch (e) {
        console.warn("insertRecognizedStructure failed:", e.message)
    }
}

// Shared by pasteSelection (source: _clipboard) and insertRecognizedStructure (source: an
// Imago-recognized structure) - clones sourceStruct's atoms/bonds/SUP-sgroups into _struct,
// centered at (cx, cy). Deliberately takes the source struct as a parameter rather than reading
// _clipboard directly, so callers other than paste never touch the user's real clipboard.
function _insertStructAt(sourceStruct, cx, cy) {
    var _clampedPaste = _clampToPage(cx, cy); cx = _clampedPaste.x; cy = _clampedPaste.y

    try {
        var pastedStruct = sourceStruct.clone()

        var minX = null, minY = null, maxX = null, maxY = null
        pastedStruct.atoms.forEach(function(a) {
        if (minX === null || a.pp.x < minX) minX = a.pp.x
        if (maxX === null || a.pp.x > maxX) maxX = a.pp.x
        if (minY === null || a.pp.y < minY) minY = a.pp.y
        if (maxY === null || a.pp.y > maxY) maxY = a.pp.y
    })
    
    var dx = 0, dy = 0
    if (minX !== null) {
        var pastedCx = (minX + maxX) / 2
        var pastedCy = (minY + maxY) / 2
        dx = cx - pastedCx
        dy = cy - pastedCy
    }

    var addedAtoms = []
    var addedBonds = []
    var addedSgroups = []  // {id, sg} pairs for redo

    var cmd = makeCmd(
        function() {
            var atomMap = new Map()
            if (addedAtoms.length === 0) {
                pastedStruct.atoms.forEach(function(a, aid) {
                    var newA = new CoreLib.ChemCore.Atom({
                        label: a.label, charge: a.charge, isotope: a.isotope, explicitValence: a.explicitValence,
                        radical: a.radical, stereoParity: a.stereoParity,
                        pp: new CoreLib.ChemCore.Vec2(a.pp.x + dx, a.pp.y + dy)
                    })
                    var newId = _struct.atoms.add(newA)
                    atomMap.set(aid, newId)
                    addedAtoms.push({ id: newId, atom: newA })
                })
                pastedStruct.bonds.forEach(function(b, bid) {
                    var newB = new CoreLib.ChemCore.Bond({
                        type: b.type, stereo: b.stereo,
                        begin: atomMap.get(b.begin), end: atomMap.get(b.end)
                    })
                    var newId = _struct.bonds.add(newB)
                    addedBonds.push({ id: newId, bond: newB })
                })
                if (pastedStruct.sgroups) {
                    pastedStruct.sgroups.forEach(function(sg) {
                        if (sg.type !== 'SUP') return
                        // Preserve sg's prototype — see snapshotStruct's sgroup copy for why.
                        var newSg = Object.assign(Object.create(Object.getPrototypeOf(sg)), sg)
                        newSg.atoms = (sg.atoms || [])
                            .map(function(aid) { return atomMap.get(aid) })
                            .filter(function(x) { return x !== undefined })
                        newSg.attachmentPoints = []
                        if (sg.attachmentPoints && sg.attachmentPoints.length > 0) {
                            sg.attachmentPoints.forEach(function(ap) {
                                var mappedId = atomMap.get(ap.atomId)
                                if (mappedId !== undefined) {
                                    var newAp = Object.assign(Object.create(Object.getPrototypeOf(ap)), ap)
                                    newAp.atomId = mappedId
                                    if (ap.leaveAtomId !== undefined) {
                                        newAp.leaveAtomId = atomMap.get(ap.leaveAtomId)
                                    }
                                    newSg.attachmentPoints.push(newAp)
                                }
                            })
                        }
                        var newSgId = _struct.sgroups.add(newSg)
                        newSg.id = newSgId
                        newSg.atoms.forEach(function(aid) {
                            var atom = _struct.atoms.get(aid)
                            if (atom) atom.sgs.add(newSgId)
                        })
                        addedSgroups.push({ id: newSgId, sg: newSg })
                    })
                    if (_struct.bindSGroupsToFunctionalGroups) _struct.bindSGroupsToFunctionalGroups()
                }
            } else {
                addedAtoms.forEach(function(ad) { _struct.atoms.set(ad.id, ad.atom) })
                addedBonds.forEach(function(bd) { _struct.bonds.set(bd.id, bd.bond) })
                addedSgroups.forEach(function(sd) {
                    _struct.sgroups.set(sd.id, sd.sg)
                    sd.sg.atoms.forEach(function(aid) {
                        var atom = _struct.atoms.get(aid)
                        if (atom) atom.sgs.add(sd.id)
                    })
                })
                if (_struct.bindSGroupsToFunctionalGroups) _struct.bindSGroupsToFunctionalGroups()
            }
            _struct.initHalfBonds()
            _struct.initNeighbors()
            _struct.updateHalfBonds()
            _struct.sortNeighbors()
        },
        function() {
            addedSgroups.forEach(function(sd) {
                sd.sg.atoms.forEach(function(aid) {
                    var atom = _struct.atoms.get(aid)
                    if (atom) atom.sgs.delete(sd.id)
                })
                _struct.sgroups.delete(sd.id)
                // Mirror insertFunctionalGroup's invert: bindSGroupsToFunctionalGroups()
                // (called in execute, above) creates a FunctionalGroup entry per SUP
                // sgroup; without this cleanup it survives as a zombie reference to a
                // now-deleted sgroup, which Struct.clone() picks back up on the next
                // copy/paste or KET export.
                if (_struct.functionalGroups) {
                    _struct.functionalGroups.forEach(function(fg, fgId) {
                        if (fg.relatedSGroupId === sd.id) _struct.functionalGroups.delete(fgId)
                    })
                }
            })
            addedBonds.forEach(function(bd) { _struct.bonds.delete(bd.id) })
            addedAtoms.forEach(function(ad) { _struct.atoms.delete(ad.id) })
            _struct.initHalfBonds()
            _struct.initNeighbors()
            _struct.updateHalfBonds()
            _struct.sortNeighbors()
        }
    )
    executeCommand(cmd)

    // Post-paste selection: sgroup prims for contracted groups, real atom IDs for expanded
    var selAtomIds = []
    var selBondIds = addedBonds.map(function(bd) { return bd.id })
    var sgroupAtomSet = {}
    addedSgroups.forEach(function(sd) {
        if (!sd.sg.data.expanded) {
            selAtomIds.push(sd.id)  // use sgroup prim ID
            sd.sg.atoms.forEach(function(aid) { sgroupAtomSet[aid] = true })
        }
    })
    addedAtoms.forEach(function(ad) { if (!sgroupAtomSet[ad.id]) selAtomIds.push(ad.id) })
    _selection = { atom_ids: selAtomIds, bond_ids: selBondIds, rxnArrow_ids: [], rxnPlus_ids: [], multitailArrow_ids: [], bbox: null }
    } catch (e) {
        // console.error("pasteSelection ERROR: " + e.message + "\n" + e.stack)
    }
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

function addBenzeneRing(cx, cy) {
    var r = CoreLib.ChemCore.StandardBondLength || 1.5
    var addedAtoms = []
    for (var i = 0; i < 6; ++i) {
        var angle = (Math.PI / 3) * i - Math.PI / 2
        var px = cx + r * Math.cos(angle)
        var py = cy + r * Math.sin(angle)
        var p = new CoreLib.ChemCore.Vec2(px, py)
        var aid = _struct.atoms.add(new CoreLib.ChemCore.Atom({ label: "C", charge: 0, pp: p }))
        addedAtoms.push(aid)
    }
    for (var j = 0; j < 6; ++j) {
        var bType = (j % 2 === 0) ? 2 : 1
        _struct.bonds.add(new CoreLib.ChemCore.Bond({ type: bType, begin: addedAtoms[j], end: addedAtoms[(j + 1) % 6] }))
    }
    _struct.initHalfBonds()
    _struct.initNeighbors()
    _struct.updateHalfBonds()
    _struct.sortNeighbors()
    _dirty = true
    return addedAtoms
}

function loadBenzene() {
    init()
    addBenzeneRing(4.0, 4.0)
    _dirty = false
}

// ---- Tools -----------------------------------------------------------------

// After a new ring is fused onto an existing structure, decide bond order
// (single/double) around the ring cycle. If one of the ring's edges coincides
// with a bond that already existed before this ring was added (a fusion
// seam), that edge's type is treated as fixed and the rest of the ring is
// alternated starting from it, so the seam's Kekule pattern stays consistent
// with the ring it was fused onto instead of clashing (previously this caused
// two adjacent double bonds at the seam, e.g. 4 double bonds on one ring of a
// fused naphthalene instead of the correct 3/2 split). If no edge is shared
// with a pre-existing bond, falls back to the plain isolated-ring pattern.
function perceiveRingAlternation(ringAtoms, oldBondTypesByPair, aromatic) {
    var n = ringAtoms.length;
    if (n < 3) return;

    function findBondId(a1, a2) {
        return _struct.bonds.find(function(bid, b) {
            return (b.begin === a1 && b.end === a2) || (b.begin === a2 && b.end === a1);
        });
    }

    var edgeBondIds = [];
    var edgeAnchorType = [];
    var anchorIdx = -1;
    for (var k = 0; k < n; k++) {
        var a1 = ringAtoms[k], a2 = ringAtoms[(k + 1) % n];
        var pairKey = Math.min(a1, a2) + "-" + Math.max(a1, a2);
        edgeBondIds.push(findBondId(a1, a2));
        var anchorType = oldBondTypesByPair[pairKey];
        edgeAnchorType.push(anchorType !== undefined ? anchorType : null);
        if (anchorType !== undefined && anchorIdx === -1) anchorIdx = k;
    }

    if (anchorIdx === -1) {
        // Freestanding ring (no fusion): default alternating pattern.
        // aromatic === false opts out (plain cyclohexane); undefined keeps
        // the historical always-alternate-6-rings behavior.
        if (aromatic === false) return;
        if (n !== 6) return;
        for (var k2 = 0; k2 < n; k2++) {
            var bond = _struct.bonds.get(edgeBondIds[k2]);
            if (bond) bond.type = (k2 % 2 === 0) ? 2 : 1;
        }
        return;
    }

    // Fused, but explicitly non-aromatic (e.g. cyclooctane grafted onto an
    // existing bond): every new edge was already created single by addRing,
    // and the anchor edge itself keeps its pre-existing type untouched below
    // — so there is nothing to alternate. Without this check, an even-sized
    // saturated template (TEMPLATE_4/6/8) fused onto an existing bond would
    // wrongly gain alternating double bonds from the block below.
    if (aromatic === false) return;

    // Only a clean single-seam alternation is handled explicitly; ring
    // systems fused at more than one edge keep whichever pre-existing bonds
    // they already had (never overwritten) and are otherwise left as-is.
    if (n % 2 !== 0) return;
    // Both atoms of the anchor edge already belong to the ring it was fused
    // from, where (if that ring is a valid Kekule structure) each of them
    // already has exactly one double bond among its two pre-existing bonds -
    // whether that's the anchor edge itself or the fusion atom's OTHER
    // pre-existing neighbor. Either way, the new edge each fusion atom gains
    // into this ring must be single: the anchor's own type does not affect
    // this. Alternating outward from there (single, double, single, ...)
    // closes correctly since n is even.
    for (var step = 1; step < n; step++) {
        var k3 = (anchorIdx + step) % n;
        if (edgeAnchorType[k3] !== null) continue; // another pre-existing edge; leave untouched
        var bond3 = _struct.bonds.get(edgeBondIds[k3]);
        if (!bond3) continue;
        bond3.type = (step % 2 === 1) ? 1 : 2;
    }
}

// aromatic: false → freestanding ring keeps all single bonds (cyclohexane);
// true/undefined → historical behavior (freestanding 6-rings alternate).
function addRing(coords, aromatic) {
    var oldStruct = snapshotStruct(_struct);
    var newStruct = null;
    var cmd = makeCmd(
        function() {
            if (newStruct !== null) { _struct = newStruct; return; }

            // Bond types the ring might fuse onto, captured from the structure
            // as it stood before this ring is added.
            var oldBondTypesByPair = {};
            oldStruct.bonds.forEach(function(b) {
                var key = Math.min(b.begin, b.end) + "-" + Math.max(b.begin, b.end);
                oldBondTypesByPair[key] = b.type;
            });

            var tempStruct = new CoreLib.ChemCore.Struct();
            var localAtoms = [];
            for (var i = 0; i < coords.length; i += 2) {
                var p = new CoreLib.ChemCore.Vec2(coords[i], coords[i+1]);
                var a = new CoreLib.ChemCore.Atom({ label: "C", charge: 0, pp: p });
                localAtoms.push(tempStruct.atoms.add(a));
            }
            var n = localAtoms.length;
            // All ring bonds start single; correct alternation is derived
            // after fusion in perceiveRingAlternation, once we know whether
            // any edge coincides with a pre-existing bond.
            for (var j = 0; j < n; ++j) {
                tempStruct.bonds.add(new CoreLib.ChemCore.Bond({ type: 1, begin: localAtoms[j], end: localAtoms[(j + 1) % n] }));
            }

            var aidMap = new Map();
            tempStruct.mergeInto(_struct, undefined, undefined, undefined, undefined, aidMap);
            var mergedAtoms = localAtoms.map(function(aid) { return aidMap.get(aid); });

            var fuseMergeMap = fuseOverlappingAtoms();
            var finalRingAtoms = mergedAtoms.map(function(aid) {
                return fuseMergeMap[aid] !== undefined ? fuseMergeMap[aid] : aid;
            });

            perceiveRingAlternation(finalRingAtoms, oldBondTypesByPair, aromatic);

            if (_struct.initHalfBonds) { _struct.initHalfBonds(); _struct.initNeighbors(); _struct.updateHalfBonds(); _struct.sortNeighbors(); }
            newStruct = snapshotStruct(_struct);
        },
        function() {
            _struct = oldStruct;
            if (_struct.initHalfBonds) { _struct.initHalfBonds(); _struct.initNeighbors(); _struct.updateHalfBonds(); _struct.sortNeighbors(); }
        }
    )
    executeCommand(cmd)
}

function clearCanvas() {
    var oldStruct = _struct
    var oldSelection = {
        atom_ids: _selection.atom_ids.slice(),
        bond_ids: _selection.bond_ids.slice(),
        rxnArrow_ids: _selection.rxnArrow_ids.slice(),
        rxnPlus_ids: _selection.rxnPlus_ids.slice(),
        multitailArrow_ids: _selection.multitailArrow_ids.slice(),
        bbox: _selection.bbox
    }
    var cmd = makeCmd(
        function() {
            _struct = new CoreLib.ChemCore.Struct()
            _selection = { atom_ids: [], bond_ids: [], rxnArrow_ids: [], rxnPlus_ids: [], multitailArrow_ids: [], bbox: null }
        },
        function() {
            _struct = oldStruct
            _selection = {
                atom_ids: oldSelection.atom_ids.slice(),
                bond_ids: oldSelection.bond_ids.slice(),
                rxnArrow_ids: oldSelection.rxnArrow_ids.slice(),
                rxnPlus_ids: oldSelection.rxnPlus_ids.slice(),
                multitailArrow_ids: oldSelection.multitailArrow_ids.slice(),
                bbox: oldSelection.bbox
            }
        }
    )
    executeCommand(cmd)
}

// ---- Functional Groups -----------------------------------------------------

function insertFunctionalGroup(fgName, cx, cy, targetAtomId) {
    var _clampedAnchor = _clampToPage(cx, cy); cx = _clampedAnchor.x; cy = _clampedAnchor.y
    var fgStruct = _fgStructs[fgName] || _saltsStructs[fgName] || _libraryStructs[fgName]

    if (!fgStruct || fgStruct.atoms.size === 0) {
        // Fallback: placeholder atom with the FG name as label
        var atomId = null
        var cmd = makeCmd(
            function() {
                var p = new CoreLib.ChemCore.Vec2(cx, cy)
                if (atomId === null) atomId = _struct.atoms.add(new CoreLib.ChemCore.Atom({ label: fgName, charge: 0, pp: p }))
                else _struct.atoms.set(atomId, new CoreLib.ChemCore.Atom({ label: fgName, charge: 0, pp: p }))
            },
            function() { _struct.atoms.delete(atomId) }
        )
        executeCommand(cmd)
        return
    }

    // Find the FG's attachment atom (from its SUP sgroup's first attachment point)
    var fgAttachAtomId = null
    if (fgStruct.sgroups) {
        fgStruct.sgroups.forEach(function(sg) {
            if (fgAttachAtomId !== null) return
            if (sg.type === 'SUP' && sg.attachmentPoints && sg.attachmentPoints.length > 0) {
                fgAttachAtomId = sg.attachmentPoints[0].atomId
            }
        })
    }

    // Determine whether we graft (merge attach point with targetAtomId)
    var graft = (targetAtomId !== null && targetAtomId !== undefined &&
                 _struct.atoms.get(targetAtomId) !== undefined &&
                 fgAttachAtomId !== null)

    // Compute FG bounding-box centre, anchored so attach atom lands on target
    var fgMinX = null, fgMaxX = null, fgMinY = null, fgMaxY = null
    fgStruct.atoms.forEach(function(a) {
        if (fgMinX === null || a.pp.x < fgMinX) fgMinX = a.pp.x
        if (fgMaxX === null || a.pp.x > fgMaxX) fgMaxX = a.pp.x
        if (fgMinY === null || a.pp.y < fgMinY) fgMinY = a.pp.y
        if (fgMaxY === null || a.pp.y > fgMaxY) fgMaxY = a.pp.y
    })
    var dx, dy
    if (graft) {
        var attachA = fgStruct.atoms.get(fgAttachAtomId)
        var targetA = _struct.atoms.get(targetAtomId)
        dx = targetA.pp.x - attachA.pp.x
        dy = targetA.pp.y - attachA.pp.y
    } else {
        dx = cx - (fgMinX + fgMaxX) / 2
        dy = cy - (fgMinY + fgMaxY) / 2
    }

    var addedAtoms = [], addedBonds = [], addedSgroups = []
    var cmd2 = makeCmd(
        function() {
            var atomMap = new Map()
            // In graft mode, map the FG's attach atom to the existing target atom
            if (graft) atomMap.set(fgAttachAtomId, targetAtomId)
            if (addedAtoms.length === 0) {
                fgStruct.atoms.forEach(function(a, aid) {
                    if (atomMap.has(aid)) return  // skip: already mapped (graft attach point)
                    var newA = new CoreLib.ChemCore.Atom({
                        label: a.label, charge: a.charge || 0,
                        pp: new CoreLib.ChemCore.Vec2(a.pp.x + dx, a.pp.y + dy)
                    })
                    var newId = _struct.atoms.add(newA)
                    atomMap.set(aid, newId)
                    addedAtoms.push({ id: newId, atom: newA })
                })
                fgStruct.bonds.forEach(function(b) {
                    var mappedBegin = atomMap.get(b.begin)
                    var mappedEnd = atomMap.get(b.end)
                    if (mappedBegin === undefined || mappedEnd === undefined || mappedBegin === mappedEnd) return
                    var newB = new CoreLib.ChemCore.Bond({
                        type: b.type, stereo: b.stereo || 0,
                        begin: mappedBegin, end: mappedEnd
                    })
                    var newId = _struct.bonds.add(newB)
                    addedBonds.push({ id: newId, bond: newB })
                })
                // Copy sgroups (SUP superatom groups = contracted abbreviations)
                if (fgStruct.sgroups) {
                    fgStruct.sgroups.forEach(function(sg) {
                        if (sg.type !== 'SUP') return
                        // Preserve sg's prototype — see snapshotStruct's sgroup copy for why.
                        var newSg = Object.assign(Object.create(Object.getPrototypeOf(sg)), sg)
                        if (!newSg.data) newSg.data = {}
                        newSg.data.name = (newSg.data && newSg.data.name) ? newSg.data.name : fgName
                        newSg.data.expanded = false
                        newSg.atoms = (sg.atoms || [])
                            .map(function(aid) { return atomMap.get(aid) })
                            .filter(function(x) { return x !== undefined })
                        newSg.attachmentPoints = []
                        if (sg.attachmentPoints && sg.attachmentPoints.length > 0) {
                            sg.attachmentPoints.forEach(function(ap) {
                                var mappedAtomId = atomMap.get(ap.atomId)
                                if (mappedAtomId !== undefined) {
                                    var newAp = Object.assign(Object.create(Object.getPrototypeOf(ap)), ap)
                                    newAp.atomId = mappedAtomId
                                    if (ap.leaveAtomId !== undefined) {
                                        newAp.leaveAtomId = atomMap.get(ap.leaveAtomId)
                                    }
                                    newSg.attachmentPoints.push(newAp)
                                }
                            })
                        }
                        var newSgId = _struct.sgroups.add(newSg)
                        newSg.id = newSgId
                        newSg.atoms.forEach(function(aid) {
                            var atom = _struct.atoms.get(aid)
                            if (atom) atom.sgs.add(newSgId)
                        })
                        addedSgroups.push({ id: newSgId, sg: newSg })
                    })
                    if (_struct.bindSGroupsToFunctionalGroups) _struct.bindSGroupsToFunctionalGroups()
                }
            } else {
                addedAtoms.forEach(function(ad) { _struct.atoms.set(ad.id, ad.atom) })
                addedBonds.forEach(function(bd) { _struct.bonds.set(bd.id, bd.bond) })
                addedSgroups.forEach(function(sd) {
                    _struct.sgroups.set(sd.id, sd.sg)
                    sd.sg.atoms.forEach(function(aid) {
                        var atom = _struct.atoms.get(aid)
                        if (atom) atom.sgs.add(sd.id)
                    })
                })
                if (_struct.bindSGroupsToFunctionalGroups) _struct.bindSGroupsToFunctionalGroups()
            }
            _struct.initHalfBonds(); _struct.initNeighbors(); _struct.updateHalfBonds(); _struct.sortNeighbors()
        },
        function() {
            addedSgroups.forEach(function(sd) {
                sd.sg.atoms.forEach(function(aid) {
                    var atom = _struct.atoms.get(aid)
                    if (atom) atom.sgs.delete(sd.id)
                })
                _struct.sgroups.delete(sd.id)
                if (_struct.functionalGroups) {
                    _struct.functionalGroups.forEach(function(fg, fgId) {
                        if (fg.relatedSGroupId === sd.id) _struct.functionalGroups.delete(fgId)
                    })
                }
            })
            addedBonds.forEach(function(bd) { _struct.bonds.delete(bd.id) })
            addedAtoms.forEach(function(ad) { _struct.atoms.delete(ad.id) })
            _struct.initHalfBonds(); _struct.initNeighbors(); _struct.updateHalfBonds(); _struct.sortNeighbors()
        }
    )
    executeCommand(cmd2)
}

// Bond-fused placement of a library ring template: maps the template's
// designated fusion bond (from library.sdf's <bondid> metadata, parsed into
// _libraryMeta[].bondIdx) exactly onto the clicked existing bond via a 2-point
// similarity transform, then reuses addRing's merge/fuse/re-alternate shape so
// the seam bond keeps its pre-existing type and the new ring Kekulizes
// consistently with the structure it fused onto. Falls back to plain
// insertFunctionalGroup placement whenever fusion isn't applicable (template
// without metadata, stale/missing target bond, template bond not in a ring) —
// QML never needs to know which of the 235/41 templates support fusion.
// Out of scope (same documented limitation as addRing/perceiveRingAlternation):
// dropping onto a bond already shared between two fused rings (multi-seam).
function insertLibraryTemplateFused(fgName, cx, cy, targetBondId) {
    var fgStruct = _libraryStructs[fgName]
    var meta = _libraryMeta[fgName]
    var targetBond = (targetBondId !== null && targetBondId !== undefined)
        ? _struct.bonds.get(targetBondId) : undefined

    var fusionBond = (fgStruct && meta && meta.bondIdx !== undefined)
        ? fgStruct.bonds.get(meta.bondIdx) : undefined
    var ringCycle = fusionBond ? shortestRingThroughBond(fgStruct, meta.bondIdx) : null

    if (!targetBond || !fusionBond || !ringCycle) {
        insertFunctionalGroup(fgName, cx, cy)
        return
    }

    var pa = fgStruct.atoms.get(fusionBond.begin)
    var pb = fgStruct.atoms.get(fusionBond.end)
    var qa = _struct.atoms.get(targetBond.begin)
    var qb = _struct.atoms.get(targetBond.end)
    if (!pa || !pb || !qa || !qb) {
        insertFunctionalGroup(fgName, cx, cy)
        return
    }

    // Which side of its own fusion bond does the template's ring mass sit on?
    var ccx = 0, ccy = 0
    ringCycle.forEach(function(aid) {
        var a = fgStruct.atoms.get(aid)
        ccx += a.pp.x; ccy += a.pp.y
    })
    ccx /= ringCycle.length; ccy /= ringCycle.length
    var templSide = ((pb.pp.x - pa.pp.x) * (ccy - pa.pp.y) -
                     (pb.pp.y - pa.pp.y) * (ccx - pa.pp.x)) >= 0 ? 1 : -1

    // Which side of the target bond should the ring grow onto?
    var targetSide = chooseEmptySide(qa.pp.x, qa.pp.y, qb.pp.x, qb.pp.y,
                                     [targetBond.begin, targetBond.end], cx, cy)

    // The transform is orientation-preserving, so mapping PA→QA,PB→QB lands the
    // ring on side `templSide` of QA→QB; the swapped mapping flips it. Pick the
    // endpoint assignment that puts the ring on the emptier side.
    var t = (templSide === targetSide)
        ? makeSimilarityTransform(pa.pp.x, pa.pp.y, pb.pp.x, pb.pp.y, qa.pp.x, qa.pp.y, qb.pp.x, qb.pp.y)
        : makeSimilarityTransform(pa.pp.x, pa.pp.y, pb.pp.x, pb.pp.y, qb.pp.x, qb.pp.y, qa.pp.x, qa.pp.y)
    if (!t) {
        insertFunctionalGroup(fgName, cx, cy)
        return
    }

    // Alternation mode for the fused ring, from the template's own authored
    // bond types around that cycle: any non-single bond → aromatic-style
    // alternation (undefined); all single → saturated, leave single (false).
    var ringAromatic = undefined
    var allSingle = true
    for (var rc = 0; rc < ringCycle.length; rc++) {
        var ra1 = ringCycle[rc], ra2 = ringCycle[(rc + 1) % ringCycle.length]
        fgStruct.bonds.forEach(function(b) {
            if ((b.begin === ra1 && b.end === ra2) || (b.begin === ra2 && b.end === ra1)) {
                if (b.type !== 1) allSingle = false
            }
        })
    }
    if (allSingle) ringAromatic = false

    var oldStruct = snapshotStruct(_struct)
    var newStruct = null
    var cmd = makeCmd(
        function() {
            if (newStruct !== null) { _struct = newStruct; return }

            var oldBondTypesByPair = {}
            oldStruct.bonds.forEach(function(b) {
                var key = Math.min(b.begin, b.end) + "-" + Math.max(b.begin, b.end)
                oldBondTypesByPair[key] = b.type
            })

            // Build the transformed template copy. Authored bond types are
            // preserved (substituents, heteroatom bonds, other rings of a
            // polycyclic template); the fusion ring's non-seam edges get
            // re-derived by perceiveRingAlternation below, and the seam edge
            // itself keeps the pre-existing bond's type via fuseOverlappingAtoms'
            // first-seen-wins dedup.
            var tempStruct = new CoreLib.ChemCore.Struct()
            var localIdMap = new Map()
            fgStruct.atoms.forEach(function(a, aid) {
                var p = t(a.pp.x, a.pp.y)
                var newA = new CoreLib.ChemCore.Atom({
                    label: a.label, charge: a.charge || 0,
                    pp: new CoreLib.ChemCore.Vec2(p.x, p.y)
                })
                localIdMap.set(aid, tempStruct.atoms.add(newA))
            })
            fgStruct.bonds.forEach(function(b) {
                tempStruct.bonds.add(new CoreLib.ChemCore.Bond({
                    type: b.type, stereo: b.stereo || 0,
                    begin: localIdMap.get(b.begin), end: localIdMap.get(b.end)
                }))
            })

            var aidMap = new Map()
            tempStruct.mergeInto(_struct, undefined, undefined, undefined, undefined, aidMap)
            var fuseMergeMap = fuseOverlappingAtoms()

            var finalRingAtoms = ringCycle.map(function(aid) {
                var merged = aidMap.get(localIdMap.get(aid))
                return fuseMergeMap[merged] !== undefined ? fuseMergeMap[merged] : merged
            })

            perceiveRingAlternation(finalRingAtoms, oldBondTypesByPair, ringAromatic)

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

// ---- SGroup expand/contract toggle ----------------------------------------

function toggleSgroupExpanded(sgId) {
    if (!_struct.sgroups) return
    var sg = _struct.sgroups.get(sgId)
    if (!sg || sg.type !== 'SUP') return
    var wasExpanded = Boolean(sg.data.expanded)
    var cmd = makeCmd(
        function() { sg.data.expanded = !wasExpanded },
        function() { sg.data.expanded = wasExpanded }
    )
    executeCommand(cmd)
}

// ---- Serialization ---------------------------------------------------------

function _deserializeStruct(fmt, data) {
    if (typeof data !== "string") throw String("Invalid data type for deserialization");
    if (data.length > 10 * 1024 * 1024) throw String("File too large");

    if (fmt === "mol" && CoreLib.ChemCore.MolSerializer) {
        return new CoreLib.ChemCore.MolSerializer().deserialize(data)
    } else if (fmt === "sdf" && CoreLib.ChemCore.SdfSerializer) {
        var items = new CoreLib.ChemCore.SdfSerializer().deserialize(data)
        if (items && items.length > 0) return items[0].struct
    } else if (fmt === "ket" && CoreLib.ChemCore.KetSerializer) {
        return new CoreLib.ChemCore.KetSerializer().deserializeMicromolecules(data)
    }
    return false
}

// ---- KET round-trip helpers for extra arrow fields -------------------------
// chem-core.js KetSerializer only preserves mode/pos/height for arrows.
// We inject/extract conditionsText and curvature around the native serialization.

function _injectArrowExtras(ketObj, struct) {
    if (!struct.rxnArrows || !ketObj.root || !ketObj.root.nodes) return
    var arrowNodes = []
    for (var i = 0; i < ketObj.root.nodes.length; i++) {
        if (ketObj.root.nodes[i].type === "arrow") arrowNodes.push(ketObj.root.nodes[i])
    }
    var idx = 0
    struct.rxnArrows.forEach(function(ar, id) {
        if (idx < arrowNodes.length) {
            if (!arrowNodes[idx].data) arrowNodes[idx].data = {}
            if (ar.conditionsText && (ar.conditionsText.above || ar.conditionsText.below)) {
                arrowNodes[idx].data.conditionsText = { above: ar.conditionsText.above, below: ar.conditionsText.below }
            }
            if (ar.curvature) arrowNodes[idx].data.curvature = { x: ar.curvature.x, y: ar.curvature.y }
            idx++
        }
    })
}

function _extractArrowExtras(ketObj) {
    var extras = []
    if (!ketObj.root || !ketObj.root.nodes) return extras
    for (var i = 0; i < ketObj.root.nodes.length; i++) {
        var node = ketObj.root.nodes[i]
        if (node.type === "arrow" && node.data) {
            extras.push({
                conditionsText: node.data.conditionsText || null,
                curvature: node.data.curvature || null
            })
            delete node.data.conditionsText
            delete node.data.curvature
        }
    }
    return extras
}

function _reattachArrowExtras(struct, extras) {
    if (!struct.rxnArrows || extras.length === 0) return
    var idx = 0
    struct.rxnArrows.forEach(function(ar, id) {
        if (idx < extras.length) {
            ar.conditionsText = extras[idx].conditionsText ? { above: extras[idx].conditionsText.above || "", below: extras[idx].conditionsText.below || "" } : { above: "", below: "" }
            ar.curvature = extras[idx].curvature || null
            idx++
        }
    })
}

// ---- KET round-trip for multi-tail arrows ----------------------------------
// The native KetSerializer expects MultitailArrow class instances with toKetNode().
// We use plain objects, so we inject/extract KET nodes ourselves.

function _injectMultitailArrows(ketObj, struct) {
    if (!struct.multitailArrows || !ketObj.root || !ketObj.root.nodes) return
    struct.multitailArrows.forEach(function(mta, id) {
        var topTail = { x: mta.spineTopX - mta.tailLength, y: mta.spineTopY }
        var bottomTail = { x: mta.spineTopX - mta.tailLength, y: mta.spineTopY + mta.height }
        var innerTails = []
        if (mta.tailsYOffset) {
            mta.tailsYOffset.forEach(function(yOff) {
                innerTails.push({ x: mta.spineTopX - mta.tailLength, y: mta.spineTopY + yOff })
            })
        }
        var allTails = [topTail].concat(innerTails).concat([bottomTail])
        var node = {
            type: _MULTITAIL_SERIALIZE_KEY,
            data: {
                head: { position: { x: mta.spineTopX + mta.headOffsetX, y: mta.spineTopY + mta.headOffsetY } },
                spine: { pos: [
                    { x: mta.spineTopX, y: mta.spineTopY },
                    { x: mta.spineTopX, y: mta.spineTopY + mta.height }
                ]},
                tails: { pos: allTails },
                zOrder: 0
            },
            selected: false
        }
        ketObj.root.nodes.push(node)
    })
}

function _extractMultitailArrows(ketObj) {
    var extracted = []
    if (!ketObj.root || !ketObj.root.nodes) return extracted
    var remaining = []
    for (var i = 0; i < ketObj.root.nodes.length; i++) {
        var node = ketObj.root.nodes[i]
        if (node.type === _MULTITAIL_SERIALIZE_KEY) {
            var d = node.data
            var spine = d.spine.pos
            var tails = d.tails.pos
            var head = d.head.position
            var spineTopX = spine[0].x
            var spineTopY = spine[0].y
            var height = spine[1].y - spine[0].y
            var tailLength = spineTopX - tails[0].x
            var headOffsetX = head.x - spineTopX
            var headOffsetY = head.y - spineTopY
            // Inner tails: all tails except first and last (border tails)
            var innerOffsets = []
            for (var j = 1; j < tails.length - 1; j++) {
                innerOffsets.push(tails[j].y - spineTopY)
            }
            extracted.push({
                spineTopX: spineTopX,
                spineTopY: spineTopY,
                height: height,
                tailLength: tailLength,
                headOffsetX: headOffsetX,
                headOffsetY: headOffsetY,
                tailsYOffset: innerOffsets
            })
        } else {
            remaining.push(node)
        }
    }
    ketObj.root.nodes = remaining
    return extracted
}

function _reattachMultitailArrows(struct, extracted) {
    if (extracted.length === 0) return
    if (!struct.multitailArrows) struct.multitailArrows = new CoreLib.ChemCore.Pool()
    extracted.forEach(function(mta) {
        var arrow = _makeMultitailArrow(
            mta.spineTopX, mta.spineTopY, mta.height, mta.tailLength, mta.headOffsetX, mta.headOffsetY
        )
        arrow.tailsYOffset = mta.tailsYOffset || []
        if (typeof struct.addMultitailArrow === 'function') {
            struct.addMultitailArrow(arrow)
        } else {
            struct.multitailArrows.add(arrow)
        }
    })
}

// ---- Public API: Serialization --------------------------------------------

function loadMolfile(molStr) {
    if (!molStr) return
    var MolSerializerClass = CoreLib.ChemCore.MolSerializer
    var serializer = new MolSerializerClass()
    try {
        var loaded = serializer.deserialize(molStr)
        if (!loaded) {
            return;
        }
        loaded.initHalfBonds()
        loaded.initNeighbors()
        loaded.updateHalfBonds()
        loaded.sortNeighbors()

        // MDL molfile/RXN (what every loadMolfile() caller round-trips through --
        // the 5 Indigo write-back ops, plus biopolymer expansion; genuine file-open
        // goes through loadStructure() instead, never here) has no concept of
        // Ketcher's own text annotations or multitail arrows. Without this, clicking
        // Layout/Aromatize/Dearomatize/Normalize/Standardize on a canvas with either
        // silently deletes them -- confirmed by direct testing. Carry them over from
        // the struct being replaced; a text label's position is an absolute x,y with
        // no atom anchor, so it may end up visually off if Layout moves things a lot,
        // but that's a smaller problem than losing it outright.
        if (_struct.texts && loaded.texts) {
            _struct.texts.forEach(function(t, id) { loaded.texts.set(id, t) })
        }
        if (_struct.multitailArrows && loaded.multitailArrows) {
            _struct.multitailArrows.forEach(function(mta, id) { loaded.multitailArrows.set(id, mta) })
        }
        if (_struct.images && loaded.images) {
            _struct.images.forEach(function(img, id) { loaded.images.set(id, img) })
        }

        var oldStruct = _struct
        var oldSelection = {
            atom_ids: _selection.atom_ids.slice(),
            bond_ids: _selection.bond_ids.slice(),
            rxnArrow_ids: _selection.rxnArrow_ids ? _selection.rxnArrow_ids.slice() : [],
            rxnPlus_ids: _selection.rxnPlus_ids ? _selection.rxnPlus_ids.slice() : [],
            multitailArrow_ids: _selection.multitailArrow_ids ? _selection.multitailArrow_ids.slice() : [],
            bbox: _selection.bbox
        }
        var cmd = makeCmd(
            function() { _struct = loaded; _selection = { atom_ids: [], bond_ids: [], rxnArrow_ids: [], rxnPlus_ids: [], multitailArrow_ids: [], bbox: null } },
            function() { _struct = oldStruct; _selection = oldSelection }
        )
        executeCommand(cmd)
    } catch (e) {
        console.warn("Failed to deserialize molfile:", e.message)
    }
}

function getStructure(fmt) {
    try {
        if (fmt === "mol" && CoreLib.ChemCore.MolSerializer) {
            return new CoreLib.ChemCore.MolSerializer().serialize(_struct)
        } else if (fmt === "sdf" && CoreLib.ChemCore.SdfSerializer) {
            return new CoreLib.ChemCore.SdfSerializer().serialize([{ struct: _struct, props: {} }])
        } else if (fmt === "ket" && CoreLib.ChemCore.KetSerializer) {
            // serializeMicromolecules already returns a JSON string, not an object
            var ketObj = JSON.parse(_serializeMicromoleculesSafe(_struct))
            _injectArrowExtras(ketObj, _struct)
            _injectMultitailArrows(ketObj, _struct)
            if (ketObj.root) ketObj.root.stereoFlags = _struct.stereoFlags || { type: 'abs', groupId: 0 }
            return JSON.stringify(ketObj)
        }
    } catch (e) {
        return ""
    }
    return ""
}

function _applyLoadedStruct(loaded) {
    loaded.initHalfBonds()
    loaded.initNeighbors()
    loaded.updateHalfBonds()
    loaded.sortNeighbors()
    var oldStruct = _struct
    var oldSelection = {
        atom_ids: _selection.atom_ids.slice(),
        bond_ids: _selection.bond_ids.slice(),
        rxnArrow_ids: _selection.rxnArrow_ids ? _selection.rxnArrow_ids.slice() : [],
        rxnPlus_ids: _selection.rxnPlus_ids ? _selection.rxnPlus_ids.slice() : [],
        multitailArrow_ids: _selection.multitailArrow_ids ? _selection.multitailArrow_ids.slice() : [],
        bbox: _selection.bbox
    }
    var cmd = makeCmd(
        function() { _struct = loaded; _selection = { atom_ids: [], bond_ids: [], rxnArrow_ids: [], rxnPlus_ids: [], multitailArrow_ids: [], bbox: null } },
        function() { _struct = oldStruct; _selection = oldSelection }
    )
    executeCommand(cmd)
}

function deserializeMol(data) {
    try {
        var loaded = _deserializeStruct("mol", data)
        if (!loaded) return
        _applyLoadedStruct(loaded)
    } catch (e) {
        console.warn("deserializeMol failed:", e.message)
    }
}

function deserializeSdf(data) {
    try {
        var loaded = _deserializeStruct("sdf", data)
        if (!loaded) return
        _applyLoadedStruct(loaded)
    } catch (e) {
        console.warn("deserializeSdf failed:", e.message)
    }
}

function _buildBatchRecordsFromStructs(items) {
    var count = items.length
    var limit = Math.min(count, 500)
    var records = []
    var savedStruct = _struct
    var savedShowH = _showExplicitH

    for (var i = 0; i < limit; i++) {
        var item = items[i]
        var label = (item.struct && item.struct.name && item.struct.name.trim().length > 0) ? item.struct.name.trim() : "Record " + (i + 1)
        
        _struct = item.struct
        _showExplicitH = false
        var thumbState
        try {
            thumbState = buildRenderPrimitives(false)
        } catch (e) {
            thumbState = { atoms: [], bonds: [] }
        }
        
        var bbox = thumbState.bbox
        var atoms = []
        var bonds = []
        if (bbox) {
            var w = bbox.maxX - bbox.minX || 1
            var h = bbox.maxY - bbox.minY || 1
            var pad = 0.1
            var scale = (1 - 2 * pad) / Math.max(w, h)
            var offX = pad + (1 - 2 * pad - w * scale) / 2
            var offY = pad + (1 - 2 * pad - h * scale) / 2
            if (thumbState.atoms) {
                thumbState.atoms.forEach(function(a) {
                    atoms.push({
                        x: offX + (a.x - bbox.minX) * scale,
                        y: offY + (a.y - bbox.minY) * scale,
                        label: a.element || ""
                    })
                })
            }
            if (thumbState.bonds) {
                thumbState.bonds.forEach(function(b) {
                    var a1 = thumbState.atomsById[b.begin.toString()]
                    var a2 = thumbState.atomsById[b.end.toString()]
                    if (a1 && a2) {
                        bonds.push({
                            x1: offX + (a1.x - bbox.minX) * scale,
                            y1: offY + (a1.y - bbox.minY) * scale,
                            x2: offX + (a2.x - bbox.minX) * scale,
                            y2: offY + (a2.y - bbox.minY) * scale,
                            type: b.type
                        })
                    }
                })
            }
        }
        records.push({ index: i, label: label, thumb: { atoms: atoms, bonds: bonds } })
    }
    
    _struct = savedStruct
    _showExplicitH = savedShowH
    
    return { count: count, records: records }
}

function deserializeRdfBatch(recordsJson) {
    try {
        var parsed
        try { parsed = JSON.parse(recordsJson) } catch (e) { parsed = [] }
        var recs = Array.isArray(parsed) ? parsed : []
        if (recs.length === 0) {
            console.log(JSON.stringify({ type: "structureResponse", reqId: "sdf_batch_list", data: JSON.stringify({ count: 0, records: [] }) }))
            return
        }
        var items = []
        for (var i = 0; i < recs.length; i++) {
            try {
                var struct = _deserializeStruct("mol", recs[i].molfile)
                if (struct) items.push({ struct: struct })
            } catch (e) { /* skip a record that fails to parse, don't abort the whole batch */ }
        }
        _sdfBatchRecords = items
        var result = _buildBatchRecordsFromStructs(items)
        console.log(JSON.stringify({ type: "structureResponse", reqId: "sdf_batch_list", data: JSON.stringify(result) }))
    } catch (e) {
        console.warn("deserializeRdfBatch failed:", e.message)
        console.log(JSON.stringify({ type: "structureResponse", reqId: "sdf_batch_list", data: JSON.stringify({ count: 0, records: [] }) }))
    }
}

function deserializeIndigoBatch(recordsJson) {
    try {
        var parsed
        try { parsed = JSON.parse(recordsJson) } catch (e) { parsed = [] }
        var recs = Array.isArray(parsed) ? parsed : []
        if (recs.length === 0) {
            console.log(JSON.stringify({ type: "structureResponse", reqId: "sdf_batch_list", data: JSON.stringify({ count: 0, records: [] }) }))
            return
        }
        var items = []
        for (var i = 0; i < recs.length; i++) {
            try {
                var struct = _deserializeStruct("mol", recs[i].molfile)
                if (struct) items.push({ struct: struct })
            } catch (e) { /* skip a record that fails to parse, don't abort the whole batch */ }
        }
        _sdfBatchRecords = items
        var result = _buildBatchRecordsFromStructs(items)
        console.log(JSON.stringify({ type: "structureResponse", reqId: "sdf_batch_list", data: JSON.stringify(result) }))
    } catch (e) {
        console.warn("deserializeIndigoBatch failed:", e.message)
        console.log(JSON.stringify({ type: "structureResponse", reqId: "sdf_batch_list", data: JSON.stringify({ count: 0, records: [] }) }))
    }
}

function deserializeSdfBatch(data) {
    try {
        var items = null
        if (CoreLib.ChemCore.SdfSerializer) {
            items = new CoreLib.ChemCore.SdfSerializer().deserialize(data)
        }
        if (!items || items.length === 0) {
            console.log(JSON.stringify({ type: "structureResponse", reqId: "sdf_batch_list", data: JSON.stringify({ count: 0, records: [] }) }))
            return
        }

        _sdfBatchRecords = items
        var result = _buildBatchRecordsFromStructs(items)
        console.log(JSON.stringify({ type: "structureResponse", reqId: "sdf_batch_list", data: JSON.stringify(result) }))
    } catch (e) {
        console.warn("deserializeSdfBatch failed:", e.message)
        console.log(JSON.stringify({ type: "structureResponse", reqId: "sdf_batch_list", data: JSON.stringify({ count: 0, records: [] }) }))
    }
}

function loadSdfBatchRecord(index) {
    try {
        if (!_sdfBatchRecords || index < 0 || index >= _sdfBatchRecords.length) return
        var loaded = _sdfBatchRecords[index].struct
        if (!loaded) return
        _applyLoadedStruct(loaded)
        _sdfProps = _sdfBatchRecords[index].props || {}
        console.log(JSON.stringify({ type: "structureResponse", reqId: "sdf_batch_load", data: "" }))
    } catch (e) {
        console.warn("loadSdfBatchRecord failed:", e.message)
    }
}

function getSdfBatchMolfiles() {
    try {
        if (!_sdfBatchRecords || _sdfBatchRecords.length === 0) {
            console.log(JSON.stringify({ type: "structureResponse", reqId: "sdf_batch_molfiles", data: JSON.stringify({ molfiles: [], labels: [] }) }))
            return
        }
        var molfiles = []
        var labels = []
        var serializer = new CoreLib.ChemCore.MolSerializer()
        var limit = Math.min(_sdfBatchRecords.length, 500)
        for (var i = 0; i < limit; i++) {
            try {
                var item = _sdfBatchRecords[i]
                var mf = serializer.serialize(item.struct)
                if (mf) {
                    molfiles.push(mf)
                    labels.push((item.struct && item.struct.name && item.struct.name.trim().length > 0) ? item.struct.name.trim() : "Record " + (i + 1))
                }
            } catch (e) { /* skip a record that fails to serialize, don't abort the whole batch */ }
        }
        console.log(JSON.stringify({ type: "structureResponse", reqId: "sdf_batch_molfiles", data: JSON.stringify({ molfiles: molfiles, labels: labels }) }))
    } catch (e) {
        console.warn("getSdfBatchMolfiles failed:", e.message)
        console.log(JSON.stringify({ type: "structureResponse", reqId: "sdf_batch_molfiles", data: JSON.stringify({ molfiles: [], labels: [] }) }))
    }
}

function realignSdfBatch(alignedMolfilesJson) {
    try {
        var molfiles = JSON.parse(alignedMolfilesJson)
        if (!Array.isArray(molfiles) || !_sdfBatchRecords || molfiles.length !== _sdfBatchRecords.length) {
            console.warn("realignSdfBatch: length mismatch or no open batch")
            return
        }
        var items = []
        for (var i = 0; i < molfiles.length; i++) {
            var struct = null
            try { struct = _deserializeStruct("mol", molfiles[i]) } catch (e) { /* fall through */ }
            items.push({ struct: struct || _sdfBatchRecords[i].struct })
        }
        _sdfBatchRecords = items
        var result = _buildBatchRecordsFromStructs(items)
        console.log(JSON.stringify({ type: "structureResponse", reqId: "sdf_batch_realigned", data: JSON.stringify(result) }))
    } catch (e) {
        console.warn("realignSdfBatch failed:", e.message)
    }
}


function selectSubstructureMatches(matchesJson) {
    var parsed
    try { parsed = JSON.parse(matchesJson) } catch (e) { parsed = { matches: [] } }
    var matches = parsed.matches || []
    var idByIndex = {}
    var i = 0
    _struct.atoms.forEach(function(a, id) { i++; idByIndex[i] = id })
    var atomIdSet = {}
    matches.forEach(function(match) {
        match.forEach(function(idx) {
            if (idByIndex[idx] !== undefined) atomIdSet[idByIndex[idx]] = true
        })
    })
    var atomIds = Object.keys(atomIdSet).map(function(k) { return parseInt(k, 10) })
    var bondIds = []
    _struct.bonds.forEach(function(b, id) {
        if (atomIdSet[b.begin] && atomIdSet[b.end]) bondIds.push(id)
    })
    _selection = { atom_ids: atomIds, bond_ids: bondIds, rxnArrow_ids: [], rxnPlus_ids: [], bbox: null }
}

function getSdfProps() {
    console.log(JSON.stringify({ type: "structureResponse", reqId: "sdf_props", data: JSON.stringify(_sdfProps) }));
}

function deserializeKet(data) {
    try {
        var ketObj = JSON.parse(data)
        var stereoFlags = (ketObj.root && ketObj.root.stereoFlags) ? ketObj.root.stereoFlags : { type: 'abs', groupId: 0 }
        if (ketObj.root) delete ketObj.root.stereoFlags
        var extras = _extractArrowExtras(ketObj)
        var mtas = _extractMultitailArrows(ketObj)
        var cleanedData = JSON.stringify(ketObj)
        var loaded = _deserializeStruct("ket", cleanedData)
        if (!loaded) return
        _reattachArrowExtras(loaded, extras)
        _reattachMultitailArrows(loaded, mtas)
        loaded.stereoFlags = stereoFlags
        _applyLoadedStruct(loaded)
    } catch (e) {
        console.warn("deserializeKet failed:", e.message)
    }
}

function _makeArrow(mode, p1x, p1y, p2x, p2y, ctrlX, ctrlY) {
    return {
        mode: mode,
        pos: [new CoreLib.ChemCore.Vec2(p1x, p1y), new CoreLib.ChemCore.Vec2(p2x, p2y)],
        height: 0,
        conditionsText: { above: "", below: "" },
        curvature: (ctrlX !== undefined && ctrlX !== null) ? { x: ctrlX, y: ctrlY } : null,
        center: function() { return this.pos[0] },
        getInitiallySelected: function() { return false },
        resetInitiallySelected: function() {},
        clone: function() {
            return {
                mode: this.mode,
                pos: [new CoreLib.ChemCore.Vec2(this.pos[0].x, this.pos[0].y), new CoreLib.ChemCore.Vec2(this.pos[1].x, this.pos[1].y)],
                height: this.height,
                conditionsText: { above: this.conditionsText.above, below: this.conditionsText.below },
                curvature: this.curvature ? { x: this.curvature.x, y: this.curvature.y } : null,
                center: this.center,
                getInitiallySelected: this.getInitiallySelected,
                resetInitiallySelected: this.resetInitiallySelected,
                clone: this.clone
            }
        }
    }
}

function addRxnArrow(cx, cy, mode) {
    mode = mode || "filled-triangle"
    var arrow = _makeArrow(mode, cx, cy, cx + (CoreLib.ChemCore.StandardBondLength || 1.5) * 2.5, cy)
    var arrowId = null
    var cmd = makeCmd(
        function() {
            if (arrowId === null) { arrowId = _struct.rxnArrows.add(arrow) }
            else { _struct.rxnArrows.set(arrowId, arrow) }
        },
        function() { _struct.rxnArrows.delete(arrowId) }
    )
    executeCommand(cmd)
}

function addCurvedArrow(x1, y1, ctrlX, ctrlY, x2, y2) {
    var arrow = _makeArrow("curved-mechanism", x1, y1, x2, y2, ctrlX, ctrlY)
    var arrowId = null
    var cmd = makeCmd(
        function() {
            if (arrowId === null) { arrowId = _struct.rxnArrows.add(arrow) }
            else { _struct.rxnArrows.set(arrowId, arrow) }
        },
        function() { _struct.rxnArrows.delete(arrowId) }
    )
    executeCommand(cmd)
}

function setRxnArrowMode(id, newMode) {
    var arrow = _struct.rxnArrows.get(id)
    if (!arrow) return
    var oldMode = arrow.mode || "filled-triangle"
    if (oldMode === newMode) return
    var cmd = makeCmd(
        function() { arrow.mode = newMode },
        function() { arrow.mode = oldMode }
    )
    executeCommand(cmd)
}

function setRxnArrowConditions(id, above, below) {
    var arrow = _struct.rxnArrows.get(id)
    if (!arrow) return
    var oldAbove = (arrow.conditionsText && arrow.conditionsText.above) || ""
    var oldBelow = (arrow.conditionsText && arrow.conditionsText.below) || ""
    if (oldAbove === above && oldBelow === below) return
    var cmd = makeCmd(
        function() {
            if (!arrow.conditionsText) arrow.conditionsText = { above: "", below: "" }
            arrow.conditionsText.above = above
            arrow.conditionsText.below = below
        },
        function() {
            if (!arrow.conditionsText) arrow.conditionsText = { above: "", below: "" }
            arrow.conditionsText.above = oldAbove
            arrow.conditionsText.below = oldBelow
        }
    )
    executeCommand(cmd)
}

function setStereoFlags(type, groupId) {
    var oldFlags = _struct.stereoFlags ? Object.assign({}, _struct.stereoFlags) : { type: 'abs', groupId: 0 }
    var newFlags = { type: type || 'abs', groupId: groupId || 0 }
    if (oldFlags.type === newFlags.type && oldFlags.groupId === newFlags.groupId) return
    var cmd = makeCmd(
        function() { _struct.stereoFlags = Object.assign({}, newFlags) },
        function() { _struct.stereoFlags = Object.assign({}, oldFlags) }
    )
    executeCommand(cmd)
}

// ---- R-Groups (Markush) ----------------------------------------------------
// Uses chem-core.js's native Struct.rgroups Pool + Fragment membership (struct.frags,
// atom.fragment, struct.getFragmentIds) rather than a hand-rolled sgroup type, since the
// KetSerializer already natively emits/reads "rgroup" KET nodes keyed off struct.rgroups
// (prepareStructForKet / rgroupToStruct) — no custom inject/extract needed.
// RGroup itself isn't exported from chem-core.js, so we use a plain object with the same
// shape {frags, range, resth, ifthen, index} that rgroupLogicToKet/clone() read via duck typing.

function _makeRGroupEntry(rgroupNumber) {
    return {
        frags: new CoreLib.ChemCore.Pile(),
        range: "", resth: false, ifthen: 0, index: rgroupNumber,
        // Duck-typed to match chem-core.js's (unexported) RGroup.clone(fidMap) — required by
        // Struct.clone()/mergeInto(), which KetSerializer uses internally per-fragment.
        clone: function(fidMap) {
            var ret = _makeRGroupEntry(this.index)
            ret.range = this.range; ret.resth = this.resth; ret.ifthen = this.ifthen
            this.frags.forEach(function(fid) {
                if (!fidMap || fidMap.has(fid)) ret.frags.add(fidMap ? fidMap.get(fid) : fid)
            })
            return ret
        }
    }
}

function addRGroup(rgroupNumber) {
    if (_struct.rgroups.get(rgroupNumber)) return
    var entry = _makeRGroupEntry(rgroupNumber)
    var cmd = makeCmd(
        function() { _struct.rgroups.set(rgroupNumber, entry) },
        function() { _struct.rgroups.delete(rgroupNumber) }
    )
    executeCommand(cmd)
}

function deleteRGroup(rgroupNumber) {
    var oldRg = _struct.rgroups.get(rgroupNumber)
    if (!oldRg) return
    var cmd = makeCmd(
        function() { _struct.rgroups.delete(rgroupNumber) },
        function() { _struct.rgroups.set(rgroupNumber, oldRg) }
    )
    executeCommand(cmd)
}

function setRGroupLogic(rgroupNumber, range, resth, ifthen) {
    var rg = _struct.rgroups.get(rgroupNumber)
    if (!rg) return
    var oldRange = rg.range, oldResth = rg.resth, oldIfthen = rg.ifthen
    var newRange = range || "", newResth = !!resth, newIfthen = ifthen || 0
    if (oldRange === newRange && oldResth === newResth && oldIfthen === newIfthen) return
    var cmd = makeCmd(
        function() { rg.range = newRange; rg.resth = newResth; rg.ifthen = newIfthen },
        function() { rg.range = oldRange; rg.resth = oldResth; rg.ifthen = oldIfthen }
    )
    executeCommand(cmd)
}

// Promotes the fragment(s) touched by the current selection into member fragments of the
// given R-group (e.g. draw an alternate substituent as an unconnected fragment elsewhere on
// canvas, select it, then call this to register it as an R1/R2/... alternative).
function addRGroupMember(rgroupNumber) {
    var rg = _struct.rgroups.get(rgroupNumber)
    if (!rg) return
    if (_struct.markFragments) _struct.markFragments()
    var fragIds = {}
    _selection.atom_ids.forEach(function(aid) {
        var a = _struct.atoms.get(aid)
        if (a && a.fragment !== undefined && a.fragment !== null && a.fragment >= 0) fragIds[a.fragment] = true
    })
    var newFragIds = Object.keys(fragIds).map(Number).filter(function(fid) { return !rg.frags.has(fid) })
    if (newFragIds.length === 0) return
    var cmd = makeCmd(
        function() { newFragIds.forEach(function(fid) { rg.frags.add(fid) }) },
        function() { newFragIds.forEach(function(fid) { rg.frags.delete(fid) }) }
    )
    executeCommand(cmd)
}

function removeRGroupMember(rgroupNumber, fragId) {
    var rg = _struct.rgroups.get(rgroupNumber)
    if (!rg || !rg.frags.has(fragId)) return
    var cmd = makeCmd(
        function() { rg.frags.delete(fragId) },
        function() { rg.frags.add(fragId) }
    )
    executeCommand(cmd)
}

// ---- Selection transforms (rotate 90° / flip) -------------------------------
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
function _textJsonFromPlain(str) {
    var paragraphs = String(str).split("\n").map(function(line) {
        return { type: "paragraph", children: [{ type: "text", text: line }] }
    })
    return JSON.stringify({ root: { type: "root", children: paragraphs } })
}

function _plainFromTextJson(content) {
    try {
        var root = JSON.parse(content).root
        if (!root || !root.children) return String(content)
        return root.children.map(function(p) {
            return (p.children || []).map(function(c) { return c.text || "" }).join("")
        }).join("\n")
    } catch (e) {
        return String(content)
    }
}

// 4-corner bounding box KetSerializer's textToKet derives width/height from
// (pos[2].x - pos[0].x, pos[1].y - pos[0].y) — must be recomputed whenever the
// text content's length changes, not just set once at creation.
function _textBoxPos(x, y, plainStr) {
    var w = Math.max(1, String(plainStr).length * 0.3), h = 0.6
    return [
        new CoreLib.ChemCore.Vec2(x, y),
        new CoreLib.ChemCore.Vec2(x, y - h),
        new CoreLib.ChemCore.Vec2(x + w, y - h),
        new CoreLib.ChemCore.Vec2(x + w, y)
    ]
}

function _makeText(plainStr, x, y) {
    return {
        content: _textJsonFromPlain(plainStr),
        position: new CoreLib.ChemCore.Vec2(x, y),
        pos: _textBoxPos(x, y, plainStr),
        getInitiallySelected: function() { return false },
        resetInitiallySelected: function() {},
        setInitiallySelected: function() {},
        clone: function() { return _makeText(_plainFromTextJson(this.content), this.position.x, this.position.y) }
    }
}

function addText(plainStr, x, y) {
    if (!plainStr || !String(plainStr).trim()) return
    var t = _makeText(String(plainStr), x, y)
    var id = null
    var cmd = makeCmd(
        function() { if (id === null) id = _struct.texts.add(t); else _struct.texts.set(id, t); _dirty = true },
        function() { _struct.texts.delete(id); _dirty = true }
    )
    executeCommand(cmd)
}

function updateText(id, plainStr) {
    var t = _struct.texts.get(id)
    if (!t) return
    var oldContent = t.content
    var oldPos = t.pos
    var newContent = _textJsonFromPlain(plainStr)
    if (oldContent === newContent) return
    var newPos = _textBoxPos(t.position.x, t.position.y, plainStr)
    var cmd = makeCmd(
        function() { t.content = newContent; t.pos = newPos; _dirty = true },
        function() { t.content = oldContent; t.pos = oldPos; _dirty = true }
    )
    executeCommand(cmd)
}

function deleteText(id) {
    var t = _struct.texts.get(id)
    if (!t) return
    var cmd = makeCmd(
        function() { _struct.texts.delete(id); _dirty = true },
        function() { _struct.texts.set(id, t); _dirty = true }
    )
    executeCommand(cmd)
}

function _makeImage(bitmap, cx, cy, halfW, halfH) {
    var _center = new CoreLib.ChemCore.Vec2(cx, cy);
    var halfSize = new CoreLib.ChemCore.Vec2(halfW, halfH);
    return {
        bitmap: bitmap,
        _center: _center,
        halfSize: halfSize,
        clone: function() { return _makeImage(this.bitmap, this._center.x, this._center.y, this.halfSize.x, this.halfSize.y); },
        addPositionOffset: function(offset) { this._center = this._center.add(offset); },
        rescaleSize: function(scale) { this.halfSize = this.halfSize.scaled(scale); },
        center: function() { return this._center; },
        toKetNode: function() {
            var topLeftCorner = this._center.sub(this.halfSize);
            var base64Data = this.bitmap.replace(/^.*;base64,/, "");
            var match = /^data:(image\/.*);base64,/.exec(this.bitmap);
            var format = match ? match[1] : undefined;
            return {
                type: 'image',
                center: { x: this._center.x, y: -this._center.y, z: 0 },
                format: format,
                boundingBox: {
                    x: topLeftCorner.x,
                    y: -topLeftCorner.y,
                    z: 0,
                    width: this.halfSize.x * 2,
                    height: this.halfSize.y * 2
                },
                data: base64Data,
                selected: this.getInitiallySelected()
            };
        },
        getInitiallySelected: function() { return false; },
        resetInitiallySelected: function() {},
        setInitiallySelected: function() {}
    };
}

function addImage(base64DataUri, cx, cy, halfW, halfH) {
    if (!base64DataUri) return
    var img = _makeImage(base64DataUri, cx, cy, halfW, halfH)
    var id = null
    var cmd = makeCmd(
        function() { if (id === null) id = _struct.images.add(img); else _struct.images.set(id, img); _dirty = true },
        function() { _struct.images.delete(id); _dirty = true }
    )
    executeCommand(cmd)
}

function deleteImage(id) {
    var img = _struct.images.get(id)
    if (!img) return
    var cmd = makeCmd(
        function() { _struct.images.delete(id); _dirty = true },
        function() { _struct.images.set(id, img); _dirty = true }
    )
    executeCommand(cmd)
}

function moveImage(id, dx, dy) {
    var img = _struct.images.get(id)
    if (!img) return
    var cmd = makeCmd(
        function() { img.addPositionOffset(new CoreLib.ChemCore.Vec2(dx, dy)); _dirty = true },
        function() { img.addPositionOffset(new CoreLib.ChemCore.Vec2(-dx, -dy)); _dirty = true }
    )
    executeCommand(cmd)
}

function resizeImage(id, scaleFactor) {
    var img = _struct.images.get(id)
    if (!img) return
    var cmd = makeCmd(
        function() { img.rescaleSize(scaleFactor); _dirty = true },
        function() { img.rescaleSize(1 / scaleFactor); _dirty = true }
    )
    executeCommand(cmd)
}

// ---- Generic query atoms (atom lists) --------------------------------------
// chem-core.js's Atom already has native atomList/queryProperties fields and the
// KetSerializer/MolSerializer already read/write them (atomToKet's "atom-list" branch),
// gated on label === "L#". AtomList itself isn't exported from chem-core.js, so we use a
// plain object with the same {ids, notList, labelList()} shape (duck-typed, same pattern
// used for R-groups above).
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

function addRxnPlus(cx, cy) {
    var plus = {
        pp: new CoreLib.ChemCore.Vec2(cx, cy),
        getInitiallySelected: function() { return false },
        resetInitiallySelected: function() {},
        clone: function() {
            return {
                pp: new CoreLib.ChemCore.Vec2(this.pp.x, this.pp.y),
                getInitiallySelected: this.getInitiallySelected,
                resetInitiallySelected: this.resetInitiallySelected,
                clone: this.clone
            }
        }
    }
    var plusId = null
    var cmd = makeCmd(
        function() {
            if (plusId === null) { plusId = _struct.rxnPluses.add(plus) }
            else { _struct.rxnPluses.set(plusId, plus) }
        },
        function() { _struct.rxnPluses.delete(plusId) }
    )
    executeCommand(cmd)
}

function deleteRxnArrow(id) {
    var arrow = _struct.rxnArrows.get(id)
    if (!arrow) return
    var cmd = makeCmd(
        function() { _struct.rxnArrows.delete(id) },
        function() { _struct.rxnArrows.set(id, arrow) }
    )
    executeCommand(cmd)
}

function deleteRxnPlus(id) {
    var plus = _struct.rxnPluses.get(id)
    if (!plus) return
    var cmd = makeCmd(
        function() { _struct.rxnPluses.delete(id) },
        function() { _struct.rxnPluses.set(id, plus) }
    )
    executeCommand(cmd)
}

// ---- Multi-tail branching arrows (Phase 1b) --------------------------------
// Uses MULTITAIL_ARROW_KEY / MULTITAIL_ARROW_SERIALIZE_KEY from chem-core.js
// Multi-tail arrows have: spine (vertical), multiple tails (horizontal, left of spine),
// and a head (extending right from spine top). Stored in _struct.multitailArrows Pool.

var _MULTITAIL_SERIALIZE_KEY = CoreLib.ChemCore.MULTITAIL_ARROW_SERIALIZE_KEY || "multi-tailed-arrow"

function _makeMultitailArrow(spineTopX, spineTopY, height, tailLength, headOffsetX, headOffsetY) {
    return {
        spineTopX: spineTopX,
        spineTopY: spineTopY,
        height: height,
        tailLength: tailLength,
        headOffsetX: headOffsetX,
        headOffsetY: headOffsetY,
        tailsYOffset: [],
        clone: function() {
            return {
                spineTopX: this.spineTopX,
                spineTopY: this.spineTopY,
                height: this.height,
                tailLength: this.tailLength,
                headOffsetX: this.headOffsetX,
                headOffsetY: this.headOffsetY,
                tailsYOffset: this.tailsYOffset.slice(),
                clone: this.clone
            }
        }
    }
}

function addMultitailArrow(cx, cy) {
    var sbl = CoreLib.ChemCore.StandardBondLength || 1.5
    var arrow = _makeMultitailArrow(cx, cy, sbl * 3, sbl * 2, sbl * 2, 0)
    var arrowId = null
    var cmd = makeCmd(
        function() {
            if (arrowId === null) {
                if (typeof _struct.addMultitailArrow === 'function') { arrowId = _struct.addMultitailArrow(arrow) }
                else { arrowId = _struct.multitailArrows.add(arrow) }
            }
            else {
                if (typeof _struct.setMultitailArrow === 'function') { _struct.setMultitailArrow(arrowId, arrow) }
                else { _struct.multitailArrows.set(arrowId, arrow) }
            }
        },
        function() { _struct.multitailArrows.delete(arrowId) }
    )
    executeCommand(cmd)
}

function deleteMultitailArrow(id) {
    var arrow = _struct.multitailArrows.get(id)
    if (!arrow) return
    var cmd = makeCmd(
        function() { _struct.multitailArrows.delete(id) },
        function() { _struct.multitailArrows.set(id, arrow) }
    )
    executeCommand(cmd)
}

function addMultitailArrowTail(id) {
    var arrow = _struct.multitailArrows.get(id)
    if (!arrow) return
    var oldOffsets = arrow.tailsYOffset.slice()
    // Add a tail at the midpoint of the widest gap
    var allY = [0, arrow.height]
    oldOffsets.forEach(function(y) { allY.push(y) })
    allY.sort(function(a, b) { return a - b })
    var maxGap = 0, gapY = 0
    for (var i = 1; i < allY.length; i++) {
        var gap = allY[i] - allY[i-1]
        if (gap > maxGap) { maxGap = gap; gapY = (allY[i] + allY[i-1]) / 2 }
    }
    var newOffsets = oldOffsets.slice()
    newOffsets.push(gapY)
    newOffsets.sort(function(a, b) { return a - b })
    var cmd = makeCmd(
        function() { arrow.tailsYOffset = newOffsets },
        function() { arrow.tailsYOffset = oldOffsets }
    )
    executeCommand(cmd)
}

// ---- Command dispatch -------------------------------------------------
// Shared by both transports below: the Node stdin/readline loop parses one
// JSON line per call and hands off (cmd, args) here; the native engine's C++
// bridge calls this directly with a real JS array (no JSON round-trip needed).

init();

function _dispatchCommand(cmd, args) {
    try {
        let result = null;

        if (cmd === 'init') init();
        else if (cmd === 'loadMol') loadMolfile(args[0]);
        else if (cmd === 'addAtom') result = addAtom(args[0], args[1], args[2], args[3]);
        else if (cmd === 'addBondAndAtom') addBondAndAtom(args[0], args[1], args[2], args[3], args[4], args[5]);
        else if (cmd === 'addBondBetweenCoords') addBondBetweenCoords(args[0], args[1], args[2], args[3], args[4], args[5]);
        else if (cmd === 'addBond') addBond(args[0], args[1], args[2], args[3]);
        else if (cmd === 'addRing') addRing(args[0], args[1]);
        else if (cmd === 'deleteAtomById') deleteAtomById(args[0]);
        else if (cmd === 'deleteBondById') deleteBondById(args[0]);
        else if (cmd === 'deleteSelection') deleteSelection();
        else if (cmd === 'undo') undo();
        else if (cmd === 'redo') redo();
        else if (cmd === 'changeAtomLabel') changeAtomLabel(args[0], args[1]);
        else if (cmd === 'setAtomMapping') setAtomMapping(args[0], args[1]);
        else if (cmd === 'changeBondType') changeBondType(args[0], args[1], args[2]);
        else if (cmd === 'changeAtomCharge') changeAtomCharge(args[0], args[1]);
        else if (cmd === 'setAttachmentPoint') setAttachmentPoint(args[0], args[1]);
        else if (cmd === 'changeAtomIsotope') changeAtomIsotope(args[0], args[1]);
        else if (cmd === 'changeAtomRadical') changeAtomRadical(args[0], args[1]);
        else if (cmd === 'changeAtomValence') changeAtomValence(args[0], args[1]);
        else if (cmd === 'getAtomProperties') {
            const props = getAtomProperties(args[0]);
            console.log(JSON.stringify({ type: "structureResponse", reqId: "atom_props", data: JSON.stringify(props || {}) }));
            return;
        }
        else if (cmd === 'selectByRect') selectByRect(args[0], args[1], args[2], args[3]);
        else if (cmd === 'addSelectionByRect') addSelectionByRect(args[0], args[1], args[2], args[3]);
        else if (cmd === 'selectByLasso') selectByLasso(args[0]);
        else if (cmd === 'selectItem') selectItem(args[0], args[1], args[2], args[3], args[4]);
        else if (cmd === 'addItemToSelection') addItemToSelection(args[0], args[1]);
        else if (cmd === 'removeItemFromSelection') removeItemFromSelection(args[0], args[1]);
        else if (cmd === 'selectFragment') selectFragment(args[0], args[1]);
        else if (cmd === 'moveSelection') moveSelection(args[0], args[1]);
        else if (cmd === 'commitMove') commitMove();
        else if (cmd === 'rotateSelectionLive') rotateSelectionLive(args[0]);
        else if (cmd === 'commitRotate') commitRotate();
        else if (cmd === 'scaleSelectionLive') scaleSelectionLive(args[0], args[1], args[2]);
        else if (cmd === 'commitScale') commitScale();
        else if (cmd === 'centerStructure') centerStructure();
        else if (cmd === 'normalizeStructure') normalizeStructure();
        else if (cmd === 'alignAtoms') alignAtoms(args[0]);
        else if (cmd === 'distributeAtoms') distributeAtoms(args[0]);
        else if (cmd === 'setStereoDescriptors') setStereoDescriptors(args[0]);
        else if (cmd === 'setCheckIssues') setCheckIssues(args[0]);
        else if (cmd === 'copySelection') copySelection();
        else if (cmd === 'cutSelection') cutSelection();
        else if (cmd === 'pasteSelection') pasteSelection(args[0], args[1]);
        else if (cmd === 'insertRecognizedStructure') insertRecognizedStructure(args[0], args[1], args[2]);
        else if (cmd === 'getClipboardAsKet') { getClipboardAsKet(); return; }
        else if (cmd === 'importKetAtPosition') importKetAtPosition(args[0], args[1], args[2]);
        else if (cmd === 'selectAll') selectAll();
        else if (cmd === 'clearCanvas') clearCanvas();
        else if (cmd === 'loadBenzene') loadBenzene();
        else if (cmd === 'deserializeMol') deserializeMol(args[0]);
        else if (cmd === 'deserializeSdf') deserializeSdf(args[0]);
        else if (cmd === 'deserializeRdfBatch') deserializeRdfBatch(args[0]);
        else if (cmd === 'deserializeIndigoBatch') deserializeIndigoBatch(args[0]);
        else if (cmd === 'deserializeSdfBatch') deserializeSdfBatch(args[0]);
        else if (cmd === 'loadSdfBatchRecord') loadSdfBatchRecord(args[0]);
        else if (cmd === 'getSdfBatchMolfiles') getSdfBatchMolfiles();
        else if (cmd === 'realignSdfBatch') realignSdfBatch(args[0]);
        else if (cmd === 'deserializeKet') deserializeKet(args[0]);
        else if (cmd === 'addRxnArrow') addRxnArrow(args[0], args[1], args[2]);
        else if (cmd === 'addCurvedArrow') addCurvedArrow(args[0], args[1], args[2], args[3], args[4], args[5]);
        else if (cmd === 'setRxnArrowMode') setRxnArrowMode(args[0], args[1]);
        else if (cmd === 'setRxnArrowConditions') setRxnArrowConditions(args[0], args[1], args[2]);
        else if (cmd === 'setStereoFlags') setStereoFlags(args[0], args[1]);
        else if (cmd === 'addRGroup') addRGroup(args[0]);
        else if (cmd === 'deleteRGroup') deleteRGroup(args[0]);
        else if (cmd === 'setRGroupLogic') setRGroupLogic(args[0], args[1], args[2], args[3]);
        else if (cmd === 'addRGroupMember') addRGroupMember(args[0]);
        else if (cmd === 'removeRGroupMember') removeRGroupMember(args[0], args[1]);
        else if (cmd === 'transformSelection') transformSelection(args[0]);
        else if (cmd === 'addChain') addChain(args[0], args[1], args[2], args[3]);
        else if (cmd === 'addText') addText(args[0], args[1], args[2]);
        else if (cmd === 'updateText') updateText(args[0], args[1]);
        else if (cmd === 'deleteText') deleteText(args[0]);
        else if (cmd === 'addImage') addImage(args[0], args[1], args[2], args[3], args[4]);
        else if (cmd === 'deleteImage') deleteImage(args[0]);
        else if (cmd === 'moveImage') moveImage(args[0], args[1], args[2]);
        else if (cmd === 'resizeImage') resizeImage(args[0], args[1]);
        else if (cmd === 'setAtomQueryList') setAtomQueryList(args[0], args[1], args[2]);
        else if (cmd === 'clearAtomQueryList') clearAtomQueryList(args[0], args[1]);
        else if (cmd === 'addRxnPlus') addRxnPlus(args[0], args[1]);
        else if (cmd === 'deleteRxnArrow') deleteRxnArrow(args[0]);
        else if (cmd === 'deleteRxnPlus') deleteRxnPlus(args[0]);
        else if (cmd === 'addMultitailArrow') addMultitailArrow(args[0], args[1]);
        else if (cmd === 'deleteMultitailArrow') deleteMultitailArrow(args[0]);
        else if (cmd === 'addMultitailArrowTail') addMultitailArrowTail(args[0]);
        else if (cmd === 'layoutSelectedChain') layoutSelectedChain();
        else if (cmd === 'getMoleculeName') getMoleculeName();
        else if (cmd === 'setMoleculeName') setMoleculeName(args[0]);
        else if (cmd === 'selectSubstructureMatches') selectSubstructureMatches(args[0]);
        else if (cmd === 'getSdfProps') getSdfProps();
        else if (cmd === 'getStructure') {
            const structStr = getStructure(args[0]);
            console.log(JSON.stringify({ type: "structureResponse", reqId: args[1], data: structStr }));
            return;
        }
        else if (cmd === 'setShowExplicitH') { _showExplicitH = args[0]; }
        else if (cmd === 'insertFunctionalGroup') insertFunctionalGroup(args[0], args[1], args[2], args[3]);
        else if (cmd === 'insertLibraryTemplateFused') insertLibraryTemplateFused(args[0], args[1], args[2], args[3]);
        else if (cmd === 'toggleSgroupExpanded') toggleSgroupExpanded(args[0]);
        else if (cmd === 'getGenericsList') {
            var gList = CoreLib.ChemCore.genericsList.map(function(label) { return { label: label } })
            console.log(JSON.stringify({ type: "structureResponse", reqId: "generics", data: JSON.stringify(gList) }))
            return
        }
        else if (cmd === 'getGenericsDetails') {
            console.log(JSON.stringify({ type: "structureResponse", reqId: "generics_details", data: JSON.stringify(CoreLib.ChemCore.Generics) }))
            return
        }
        else if (cmd === 'getSaltsAndSolventsList') {
            var sList = Object.keys(_saltsStructs).map(function(name) { return { label: name } })
            console.log(JSON.stringify({ type: "structureResponse", reqId: "salts", data: JSON.stringify(sList) }))
            return
        }
        else if (cmd === 'getFunctionalGroupsList') {
            var fgList = Object.keys(_fgStructs).sort().map(function(name) {
                return { label: name, group: (_fgMeta[name] && _fgMeta[name].group) || 'Functional Groups' }
            })
            console.log(JSON.stringify({ type: "structureResponse", reqId: "fg_list", data: JSON.stringify(fgList) }))
            return
        }
        else if (cmd === 'getTemplateLibraryList') {
            var libList = Object.keys(_libraryStructs).map(function(name) {
                return { label: name, group: (_libraryMeta[name] && _libraryMeta[name].group) || 'Templates' }
            })
            console.log(JSON.stringify({ type: "structureResponse", reqId: "library_list", data: JSON.stringify(libList) }))
            return
        }
        else if (cmd === 'getTemplateThumbnail') {
            var name = args[0]
            var thumbReqId = args[1] || ("thumb_" + name)
            var tplStruct = _fgStructs[name] || _saltsStructs[name] || _libraryStructs[name]
            if (!tplStruct) {
                console.log(JSON.stringify({ type: "structureResponse", reqId: thumbReqId, data: "{}" }))
                return
            }
            var savedStruct = _struct
            var savedShowH = _showExplicitH
            _struct = tplStruct
            _showExplicitH = false
            var thumbState
            try {
                thumbState = buildRenderPrimitives(false)
            } catch (e) {
                thumbState = { atoms: [], bonds: [] }
            }
            _struct = savedStruct
            _showExplicitH = savedShowH

            // Normalize coordinates to 0..1 range
            var bbox = thumbState.bbox
            var atoms = []
            var bonds = []
            if (bbox) {
                var w = bbox.maxX - bbox.minX || 1
                var h = bbox.maxY - bbox.minY || 1
                var pad = 0.1
                var scale = (1 - 2 * pad) / Math.max(w, h)
                var offX = pad + (1 - 2 * pad - w * scale) / 2
                var offY = pad + (1 - 2 * pad - h * scale) / 2
                if (thumbState.atoms) {
                    thumbState.atoms.forEach(function(a) {
                        atoms.push({
                            x: offX + (a.x - bbox.minX) * scale,
                            y: offY + (a.y - bbox.minY) * scale,
                            label: a.element || ""
                        })
                    })
                }
                if (thumbState.bonds) {
                    thumbState.bonds.forEach(function(b) {
                        var a1 = thumbState.atomsById[b.begin.toString()]
                        var a2 = thumbState.atomsById[b.end.toString()]
                        if (a1 && a2) {
                            bonds.push({
                                x1: offX + (a1.x - bbox.minX) * scale,
                                y1: offY + (a1.y - bbox.minY) * scale,
                                x2: offX + (a2.x - bbox.minX) * scale,
                                y2: offY + (a2.y - bbox.minY) * scale,
                                type: b.type || 1,
                                stereo: b.stereo || 0
                            })
                        }
                    })
                }
            }
            console.log(JSON.stringify({ type: "structureResponse", reqId: thumbReqId, data: JSON.stringify({ atoms: atoms, bonds: bonds }) }))
            return
        }
        
        const state = buildRenderPrimitives(_showExplicitH);
        const selection = currentSelection();
        
        console.log(JSON.stringify({
            status: "ok",
            state: state,
            selection: selection,
            isDirty: isDirty(),
            canUndo: canUndo(),
            canRedo: canRedo(),
            result: result
        }));
    } catch (e) {
        console.log(JSON.stringify({ status: "error", message: e.toString() }))
    }
}

// ---- Transport -------------------------------------------------------------

if (_hasNative) {
    // The C++ bridge fetches this function off the global object and calls it
    // directly with a real JS array for args -- no JSON stringify/parse round-trip.
    globalThis.__dispatchCommand = _dispatchCommand;
} else {
    const readline = require("readline");
    const rl = readline.createInterface({
        input: process.stdin,
        output: process.stdout,
        terminal: false
    });
    rl.on('line', (line) => {
        if (!line.trim()) return;
        try {
            const msg = JSON.parse(line);
            _dispatchCommand(msg.cmd, msg.args || []);
        } catch (e) {
            console.log(JSON.stringify({ status: "error", message: e.toString() }))
        }
    });
}
