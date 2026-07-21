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


// ---- Worker Sub-modules Loader ----------------------------------------------
// Loaded in this order into the SAME global scope (matches how chem-core.js is
// already loaded above) -- order only matters for immediate top-level execution
// (30-templates.js's loadTemplates() IIFE, which needs CoreLib/fs/path already
// bootstrapped above, not anything cross-file at the function level, since no
// command actually dispatches until every file below has finished loading).
const _workerFiles = [
    "10-state.js",
    "20-edit.js",
    "30-templates.js",
    "40-serialize.js",
    "50-reactions.js",
    "60-analysis.js",
    "90-dispatch.js"
];

for (let i = 0; i < _workerFiles.length; i++) {
    const p = path.join(_workerDir, "worker", _workerFiles[i]);
    const code = fs.readFileSync(p, "utf8");
    eval(code);
}
