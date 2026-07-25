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
    // Defensive `|| []` on every field: every _selection writer is supposed to
    // always populate all 5 fields, but this unconditional .slice() is what
    // actually surfaced it the one time a writer didn't (a stale
    // selectByRect/selectByLasso/selectAll/selectSubstructureMatches result
    // missing multitailArrow_ids threw "cannot read property 'slice' of
    // undefined" here) -- kept defensive even after fixing every writer, so a
    // future writer bug degrades to "selection not preserved across undo"
    // instead of crashing the whole command.
    var oldSelection = {
        atom_ids: (_selection.atom_ids || []).slice(),
        bond_ids: (_selection.bond_ids || []).slice(),
        rxnArrow_ids: (_selection.rxnArrow_ids || []).slice(),
        rxnPlus_ids: (_selection.rxnPlus_ids || []).slice(),
        multitailArrow_ids: (_selection.multitailArrow_ids || []).slice(),
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

