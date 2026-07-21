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

// Each entry: fn(args) performs the command. broadcast:true (default) means the
// dispatcher builds and emits the standard state snapshot after fn runs, using
// fn's return value as `result` only when captureResult is set (only addAtom
// uses its return value today). custom:true means fn emits its own
// structureResponse (or nothing) and the dispatcher must not also broadcast.
