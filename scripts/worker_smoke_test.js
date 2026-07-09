#!/usr/bin/env node
// Drives src/v8_worker.js over its real stdin/stdout JSON protocol and asserts on
// the resulting state — no GUI required. Run after any v8_worker.js change:
//   node scripts/worker_smoke_test.js
"use strict";
const { spawn } = require("child_process");
const path = require("path");

const workerPath = path.join(__dirname, "..", "src", "v8_worker.js");
const proc = spawn(process.execPath, [workerPath]);

let buffer = "";
const responses = [];
proc.stdout.on("data", (chunk) => {
    buffer += chunk.toString();
    let idx;
    while ((idx = buffer.indexOf("\n")) >= 0) {
        const line = buffer.slice(0, idx);
        buffer = buffer.slice(idx + 1);
        if (line.trim()) {
            try { responses.push(JSON.parse(line)); }
            catch (e) { console.error("UNPARSEABLE LINE:", line); }
        }
    }
});
proc.stderr.on("data", (d) => process.stderr.write(d));

let failures = 0;
function assert(cond, label) {
    if (cond) console.log("  PASS  " + label);
    else { console.log("  FAIL  " + label); failures++; }
}

function send(cmd, args) {
    proc.stdin.write(JSON.stringify({ cmd, args: args || [] }) + "\n");
}

// Fire the whole command sequence; responses[] accumulates in order since the
// worker is single-threaded and processes stdin lines synchronously per line.
send("init");
send("addAtom", ["C", 0, 0, 0]);
send("addBondAndAtom", [0, "R1", 1, 0, 1, 0]);                       // 2 atoms, 1 bond
send("addRing", [[3, 3, 4, 3, 4.5, 3.87, 4, 4.73, 3, 4.73, 2.5, 3.87], true]);  // aromatic hexagon
send("addRing", [[6, 3, 7, 3, 7.5, 3.87, 7, 4.73, 6, 4.73, 5.5, 3.87], false]); // saturated hexagon
send("addChain", [0, -2, 3, -2]);
send("selectAll");
send("transformSelection", ["rotate_cw"]);
send("transformSelection", ["flip_h"]);
send("undo");
send("undo");
send("addText", ["reflux, 2h", 5, -2]);
send("getStructure", ["ket", "smoke_ket"]);
send("updateText", [0, "reflux, 4h"]);
send("deleteText", [0]);
send("undo");
send("undo"); send("undo"); send("undo"); send("undo"); send("undo"); send("undo"); send("undo"); send("undo");

setTimeout(() => {
    proc.stdin.end();
    proc.kill();

    console.log("\n=== worker_smoke_test.js ===\n");

    const states = responses.filter((r) => r.state);
    const ketResponses = responses.filter((r) => r.type === "structureResponse");

    assert(states.length >= 10, "received state updates for each mutating command");

    // states[0]=init states[1]=addAtom states[2]=addBondAndAtom
    // states[3]=addRing#1 states[4]=addRing#2(both rings now present)
    const afterRings = states[4]; // after both addRing calls
    if (afterRings && afterRings.state.rings) {
        const rings = afterRings.state.rings;
        assert(rings.length === 2, "two rings detected (" + rings.length + " found)");
        const aromatic = rings.find((r) => r.hasBondType4 === false && r.isAromatic === true);
        const saturated = rings.find((r) => r.isAromatic === false);
        assert(!!aromatic, "aromatic ring flagged isAromatic=true, hasBondType4=false (Kekulized)");
        assert(!!saturated, "saturated ring flagged isAromatic=false");
    } else {
        assert(false, "rings array present in state");
    }

    const afterChain = states[5];
    // Chain from (0,-2) to (3,-2): dist 3 / StandardBondLength 1.5 = 2 bonds = 3 atoms.
    assert(afterChain && afterChain.state.atoms.length === 2 + 6 + 6 + 3,
        "chain added expected atom count (2 core + 6 + 6 ring + 3 chain = 17, got " +
        (afterChain ? afterChain.state.atoms.length : "?") + ")");

    const ketResp = ketResponses.find((r) => r.reqId === "smoke_ket");
    let ketOk = false, textNodeOk = false;
    if (ketResp) {
        try {
            const ketObj = JSON.parse(ketResp.data);
            ketOk = !!ketObj.root && Array.isArray(ketObj.root.nodes);
            // Most node types are $ref pointers into top-level keys; text nodes are
            // inlined directly in root.nodes instead.
            const textNode = (ketObj.root.nodes || [])
                .map((n) => (n.$ref ? ketObj[n.$ref] : n))
                .find((n) => n && n.type === "text");
            textNodeOk = !!textNode;
        } catch (e) { /* ketOk stays false */ }
    }
    assert(ketOk, "getStructure('ket') returned parseable KET with root.nodes");
    assert(textNodeOk, "KET contains a structured text node (not empty data:\"\")");

    const afterFullUndo = states[states.length - 1];
    assert(afterFullUndo && afterFullUndo.state.atoms.length === 0 && !afterFullUndo.state.texts.length,
        "full undo chain returns to an empty structure");

    console.log("\n" + (failures === 0 ? "ALL PASS" : failures + " FAILURE(S)") + "\n");
    process.exit(failures === 0 ? 0 : 1);
}, 2000);
