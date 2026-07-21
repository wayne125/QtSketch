var _sdfBatchRecords = [];
var _sdfProps = {};

// ============================================================================
// ---- Functional Group Library (loaded once at startup) ---------------------

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

