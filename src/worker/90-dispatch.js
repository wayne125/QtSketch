const COMMANDS = {
    init:            { fn: () => init() },
    loadMol:         { fn: (args) => loadMolfile(args[0]) },
    addAtom:         { fn: (args) => addAtom(args[0], args[1], args[2], args[3]), captureResult: true },
    addBondAndAtom:  { fn: (args) => addBondAndAtom(args[0], args[1], args[2], args[3], args[4], args[5]) },
    addBondBetweenCoords: { fn: (args) => addBondBetweenCoords(args[0], args[1], args[2], args[3], args[4], args[5]) },
    addBond:         { fn: (args) => addBond(args[0], args[1], args[2], args[3]) },
    addRing:         { fn: (args) => addRing(args[0], args[1]) },
    deleteAtomById:  { fn: (args) => deleteAtomById(args[0]) },
    deleteBondById:  { fn: (args) => deleteBondById(args[0]) },
    deleteSelection: { fn: () => deleteSelection() },
    undo:            { fn: () => undo() },
    redo:            { fn: () => redo() },
    changeAtomLabel: { fn: (args) => changeAtomLabel(args[0], args[1]) },
    setAtomMapping:  { fn: (args) => setAtomMapping(args[0], args[1]) },
    changeBondType:  { fn: (args) => changeBondType(args[0], args[1], args[2]) },
    changeAtomCharge:{ fn: (args) => changeAtomCharge(args[0], args[1]) },
    setAttachmentPoint: { fn: (args) => setAttachmentPoint(args[0], args[1]) },
    changeAtomIsotope: { fn: (args) => changeAtomIsotope(args[0], args[1]) },
    changeAtomRadical: { fn: (args) => changeAtomRadical(args[0], args[1]) },
    changeAtomValence: { fn: (args) => changeAtomValence(args[0], args[1]) },
    getAtomProperties: {
        custom: true,
        fn: (args) => {
            const props = getAtomProperties(args[0]);
            console.log(JSON.stringify({ type: "structureResponse", reqId: "atom_props", data: JSON.stringify(props || {}) }));
        }
    },
    selectByRect:    { fn: (args) => selectByRect(args[0], args[1], args[2], args[3]) },
    addSelectionByRect: { fn: (args) => addSelectionByRect(args[0], args[1], args[2], args[3]) },
    selectByLasso:   { fn: (args) => selectByLasso(args[0]) },
    selectItem:      { fn: (args) => selectItem(args[0], args[1], args[2], args[3], args[4]) },
    addItemToSelection: { fn: (args) => addItemToSelection(args[0], args[1]) },
    removeItemFromSelection: { fn: (args) => removeItemFromSelection(args[0], args[1]) },
    selectFragment:  { fn: (args) => selectFragment(args[0], args[1]) },
    moveSelection:   { fn: (args) => moveSelection(args[0], args[1]) },
    commitMove:      { fn: () => commitMove() },
    rotateSelectionLive: { fn: (args) => rotateSelectionLive(args[0]) },
    commitRotate:    { fn: () => commitRotate() },
    scaleSelectionLive: { fn: (args) => scaleSelectionLive(args[0], args[1], args[2]) },
    commitScale:     { fn: () => commitScale() },
    centerStructure: { fn: () => centerStructure() },
    normalizeStructure: { fn: () => normalizeStructure() },
    alignAtoms:      { fn: (args) => alignAtoms(args[0]) },
    distributeAtoms: { fn: (args) => distributeAtoms(args[0]) },
    setStereoDescriptors: { custom: true, fn: (args) => setStereoDescriptors(args[0]) },
    setCheckIssues:  { custom: true, fn: (args) => setCheckIssues(args[0]) },
    copySelection:   { fn: () => copySelection() },
    cutSelection:    { fn: () => cutSelection() },
    pasteSelection:  { fn: (args) => pasteSelection(args[0], args[1]) },
    insertRecognizedStructure: { fn: (args) => insertRecognizedStructure(args[0], args[1], args[2]) },
    getClipboardAsKet: { custom: true, fn: () => getClipboardAsKet() },
    getClipboardPreview: { custom: true, fn: () => getClipboardPreview() },
    importKetAtPosition: { fn: (args) => importKetAtPosition(args[0], args[1], args[2]) },
    selectAll:       { fn: () => selectAll() },
    clearCanvas:     { fn: () => clearCanvas() },
    loadBenzene:     { fn: () => loadBenzene() },
    deserializeMol:  { fn: (args) => deserializeMol(args[0]) },
    deserializeSdf:  { fn: (args) => deserializeSdf(args[0]) },
    deserializeRdfBatch: { fn: (args) => deserializeRdfBatch(args[0]) },
    deserializeIndigoBatch: { fn: (args) => deserializeIndigoBatch(args[0]) },
    deserializeSdfBatch: { fn: (args) => deserializeSdfBatch(args[0]) },
    loadSdfBatchRecord: { fn: (args) => loadSdfBatchRecord(args[0]) },
    getSdfBatchMolfiles: { fn: () => getSdfBatchMolfiles() },
    realignSdfBatch: { fn: (args) => realignSdfBatch(args[0]) },
    deserializeKet:  { fn: (args) => deserializeKet(args[0]) },
    addRxnArrow:     { fn: (args) => addRxnArrow(args[0], args[1], args[2]) },
    addCurvedArrow:  { fn: (args) => addCurvedArrow(args[0], args[1], args[2], args[3], args[4], args[5]) },
    setRxnArrowMode: { fn: (args) => setRxnArrowMode(args[0], args[1]) },
    setRxnArrowConditions: { fn: (args) => setRxnArrowConditions(args[0], args[1], args[2]) },
    setStereoFlags:  { fn: (args) => setStereoFlags(args[0], args[1]) },
    addRGroup:       { fn: (args) => addRGroup(args[0]) },
    deleteRGroup:    { fn: (args) => deleteRGroup(args[0]) },
    setRGroupLogic:  { fn: (args) => setRGroupLogic(args[0], args[1], args[2], args[3]) },
    addRGroupMember: { fn: (args) => addRGroupMember(args[0]) },
    removeRGroupMember: { fn: (args) => removeRGroupMember(args[0], args[1]) },
    transformSelection: { fn: (args) => transformSelection(args[0]) },
    addChain:        { fn: (args) => addChain(args[0], args[1], args[2], args[3]) },
    addText:         { fn: (args) => addText(args[0], args[1], args[2]) },
    updateText:      { fn: (args) => updateText(args[0], args[1]) },
    deleteText:      { fn: (args) => deleteText(args[0]) },
    addImage:        { fn: (args) => addImage(args[0], args[1], args[2], args[3], args[4]) },
    deleteImage:     { fn: (args) => deleteImage(args[0]) },
    moveImage:       { fn: (args) => moveImage(args[0], args[1], args[2]) },
    resizeImage:     { fn: (args) => resizeImage(args[0], args[1]) },
    setAtomQueryList: { fn: (args) => setAtomQueryList(args[0], args[1], args[2]) },
    clearAtomQueryList: { fn: (args) => clearAtomQueryList(args[0], args[1]) },
    addRxnPlus:      { fn: (args) => addRxnPlus(args[0], args[1]) },
    deleteRxnArrow:  { fn: (args) => deleteRxnArrow(args[0]) },
    deleteRxnPlus:   { fn: (args) => deleteRxnPlus(args[0]) },
    addMultitailArrow: { fn: (args) => addMultitailArrow(args[0], args[1]) },
    deleteMultitailArrow: { fn: (args) => deleteMultitailArrow(args[0]) },
    addMultitailArrowTail: { fn: (args) => addMultitailArrowTail(args[0]) },
    layoutSelectedChain: { fn: () => layoutSelectedChain() },
    getMoleculeName: { custom: true, fn: () => getMoleculeName() },
    setMoleculeName: { fn: (args) => setMoleculeName(args[0]) },
    selectSubstructureMatches: { fn: (args) => selectSubstructureMatches(args[0]) },
    getSdfProps:     { custom: true, fn: () => getSdfProps() },
    bioBuildSequenceView: {
        custom: true,
        fn: (args) => {
            bioBuildSequenceView(args[0], args[1]);
            bioGetSequenceViewSnapshot("biopolymer_seq_view");
        }
    },
    bioAddMonomer: {
        custom: true,
        fn: (args) => {
            bioAddMonomer(args[0], args[1]);
            bioGetSequenceViewSnapshot("biopolymer_seq_view");
        }
    },
    bioDeleteMonomer: {
        custom: true,
        fn: (args) => {
            bioDeleteMonomer(args[0]);
            bioGetSequenceViewSnapshot("biopolymer_seq_view");
        }
    },
    getStructure: {
        custom: true,
        fn: (args) => {
            const structStr = getStructure(args[0]);
            console.log(JSON.stringify({ type: "structureResponse", reqId: args[1], data: structStr }));
        }
    },
    setShowExplicitH: { fn: (args) => { _showExplicitH = args[0]; } },
    insertFunctionalGroup: { fn: (args) => insertFunctionalGroup(args[0], args[1], args[2], args[3]) },
    insertLibraryTemplateFused: { fn: (args) => insertLibraryTemplateFused(args[0], args[1], args[2], args[3]) },
    toggleSgroupExpanded: { fn: (args) => toggleSgroupExpanded(args[0]) },
    getGenericsList: {
        custom: true,
        fn: () => {
            var gList = CoreLib.ChemCore.genericsList.map(function(label) { return { label: label } })
            console.log(JSON.stringify({ type: "structureResponse", reqId: "generics", data: JSON.stringify(gList) }))
        }
    },
    getGenericsDetails: {
        custom: true,
        fn: () => {
            console.log(JSON.stringify({ type: "structureResponse", reqId: "generics_details", data: JSON.stringify(CoreLib.ChemCore.Generics) }))
        }
    },
    getSaltsAndSolventsList: {
        custom: true,
        fn: () => {
            var sList = Object.keys(_saltsStructs).map(function(name) { return { label: name } })
            console.log(JSON.stringify({ type: "structureResponse", reqId: "salts", data: JSON.stringify(sList) }))
        }
    },
    getFunctionalGroupsList: {
        custom: true,
        fn: () => {
            var fgList = Object.keys(_fgStructs).sort().map(function(name) {
                return { label: name, group: (_fgMeta[name] && _fgMeta[name].group) || 'Functional Groups' }
            })
            console.log(JSON.stringify({ type: "structureResponse", reqId: "fg_list", data: JSON.stringify(fgList) }))
        }
    },
    getTemplateLibraryList: {
        custom: true,
        fn: () => {
            var libList = Object.keys(_libraryStructs).map(function(name) {
                return { label: name, group: (_libraryMeta[name] && _libraryMeta[name].group) || 'Templates' }
            })
            console.log(JSON.stringify({ type: "structureResponse", reqId: "library_list", data: JSON.stringify(libList) }))
        }
    },
    getTemplateThumbnail: {
        custom: true,
        fn: (args) => {
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
        }
    }
};

function _dispatchCommand(cmd, args) {
    try {
        const entry = COMMANDS[cmd];
        if (!entry) {
            console.log(JSON.stringify({ status: "error", message: "unknown command: " + cmd }));
            return;
        }

        let result = null;
        const ret = entry.fn(args);
        if (entry.captureResult) result = ret;
        if (entry.custom) return;

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
