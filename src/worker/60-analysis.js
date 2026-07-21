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

