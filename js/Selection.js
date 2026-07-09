.pragma library

// Shared selection-count accessors. `sketch` is a V8Process (or equivalent)
// exposing a `selection` property shaped {atom_ids, bond_ids, ...} that may
// not yet be populated (worker hasn't replied) — every accessor here always
// returns a number, never undefined, so callers can bind directly without
// re-deriving the null-coercion chain themselves.

function atomCount(sketch) {
    return (sketch && sketch.selection && sketch.selection.atom_ids && sketch.selection.atom_ids.length) || 0
}

function bondCount(sketch) {
    return (sketch && sketch.selection && sketch.selection.bond_ids && sketch.selection.bond_ids.length) || 0
}

function totalCount(sketch) {
    return atomCount(sketch) + bondCount(sketch)
}

function hasAtoms(sketch, n) {
    return atomCount(sketch) >= n
}
