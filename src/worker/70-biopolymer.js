// ---- Biopolymer Sequence View (hand-rolled, no chem-core.js dependency) ----
// v1 of this feature tried to reuse chem-core.js's own DrawingEntitiesManager and was
// abandoned: its real mutation methods throw, because several submodules they depend on
// were bundled as stub-empty Proxies (see Features To Be Implemented.md / Architecture
// Map.md, 2026-07-23 entries). This version needs none of that -- just plain position/
// label/class/connectivity data for a read-only display.

var _bioNextId = 1;
var _bioMonomers = [];
var _bioBonds = [];
var _bioSeqType = "PEPTIDE";

var _STANDARD_MONOMER_TEMPLATES = {
    PEPTIDE: {
        "A": { alias: "Ala", name: "Alanine", class: "AminoAcid" },
        "C": { alias: "Cys", name: "Cysteine", class: "AminoAcid" },
        "D": { alias: "Asp", name: "Aspartic Acid", class: "AminoAcid" },
        "E": { alias: "Glu", name: "Glutamic Acid", class: "AminoAcid" },
        "F": { alias: "Phe", name: "Phenylalanine", class: "AminoAcid" },
        "G": { alias: "Gly", name: "Glycine", class: "AminoAcid" },
        "H": { alias: "His", name: "Histidine", class: "AminoAcid" },
        "I": { alias: "Ile", name: "Isoleucine", class: "AminoAcid" },
        "K": { alias: "Lys", name: "Lysine", class: "AminoAcid" },
        "L": { alias: "Leu", name: "Leucine", class: "AminoAcid" },
        "M": { alias: "Met", name: "Methionine", class: "AminoAcid" },
        "N": { alias: "Asn", name: "Asparagine", class: "AminoAcid" },
        "P": { alias: "Pro", name: "Proline", class: "AminoAcid" },
        "Q": { alias: "Gln", name: "Glutamine", class: "AminoAcid" },
        "R": { alias: "Arg", name: "Arginine", class: "AminoAcid" },
        "S": { alias: "Ser", name: "Serine", class: "AminoAcid" },
        "T": { alias: "Thr", name: "Threonine", class: "AminoAcid" },
        "V": { alias: "Val", name: "Valine", class: "AminoAcid" },
        "W": { alias: "Trp", name: "Tryptophan", class: "AminoAcid" },
        "Y": { alias: "Tyr", name: "Tyrosine", class: "AminoAcid" },
        "B": { alias: "Asx", name: "Asx (Asp or Asn)", class: "AminoAcid", ambiguous: true },
        "Z": { alias: "Glx", name: "Glx (Glu or Gln)", class: "AminoAcid", ambiguous: true },
        "J": { alias: "Xle", name: "Xle (Leu or Ile)", class: "AminoAcid", ambiguous: true },
        "X": { alias: "Xaa", name: "Xaa (any amino acid)", class: "AminoAcid", ambiguous: true }
    },
    RNA: {
        "A": { alias: "A", name: "Adenine", class: "Base", monomerType: "RNA" },
        "C": { alias: "C", name: "Cytosine", class: "Base", monomerType: "RNA" },
        "G": { alias: "G", name: "Guanine", class: "Base", monomerType: "RNA" },
        "U": { alias: "U", name: "Uracil", class: "Base", monomerType: "RNA" },
        "N": { alias: "N", name: "any base",             class: "Base", monomerType: "RNA", ambiguous: true },
        "B": { alias: "B", name: "not A (C/G/T or C/G/U)", class: "Base", monomerType: "RNA", ambiguous: true },
        "V": { alias: "V", name: "not T/U (A/C/G)",       class: "Base", monomerType: "RNA", ambiguous: true },
        "D": { alias: "D", name: "not C (A/G/T or A/G/U)", class: "Base", monomerType: "RNA", ambiguous: true },
        "H": { alias: "H", name: "not G (A/C/T or A/C/U)", class: "Base", monomerType: "RNA", ambiguous: true },
        "K": { alias: "K", name: "keto (G/T or G/U)",     class: "Base", monomerType: "RNA", ambiguous: true },
        "M": { alias: "M", name: "amino (A/C)",           class: "Base", monomerType: "RNA", ambiguous: true },
        "W": { alias: "W", name: "weak (A/T or A/U)",     class: "Base", monomerType: "RNA", ambiguous: true },
        "Y": { alias: "Y", name: "pyrimidine (C/T or C/U)", class: "Base", monomerType: "RNA", ambiguous: true },
        "R": { alias: "R", name: "purine (A/G)",          class: "Base", monomerType: "RNA", ambiguous: true },
        "S": { alias: "S", name: "strong (C/G)",          class: "Base", monomerType: "RNA", ambiguous: true }
    },
    DNA: {
        "A": { alias: "dA", name: "Adenine", class: "Base", monomerType: "DNA" },
        "C": { alias: "dC", name: "Cytosine", class: "Base", monomerType: "DNA" },
        "G": { alias: "dG", name: "Guanine", class: "Base", monomerType: "DNA" },
        "T": { alias: "dT", name: "Thymine", class: "Base", monomerType: "DNA" },
        "N": { alias: "N", name: "any base",             class: "Base", monomerType: "DNA", ambiguous: true },
        "B": { alias: "B", name: "not A (C/G/T or C/G/U)", class: "Base", monomerType: "DNA", ambiguous: true },
        "V": { alias: "V", name: "not T/U (A/C/G)",       class: "Base", monomerType: "DNA", ambiguous: true },
        "D": { alias: "D", name: "not C (A/G/T or A/G/U)", class: "Base", monomerType: "DNA", ambiguous: true },
        "H": { alias: "H", name: "not G (A/C/T or A/C/U)", class: "Base", monomerType: "DNA", ambiguous: true },
        "K": { alias: "K", name: "keto (G/T or G/U)",     class: "Base", monomerType: "DNA", ambiguous: true },
        "M": { alias: "M", name: "amino (A/C)",           class: "Base", monomerType: "DNA", ambiguous: true },
        "W": { alias: "W", name: "weak (A/T or A/U)",     class: "Base", monomerType: "DNA", ambiguous: true },
        "Y": { alias: "Y", name: "pyrimidine (C/T or C/U)", class: "Base", monomerType: "DNA", ambiguous: true },
        "R": { alias: "R", name: "purine (A/G)",          class: "Base", monomerType: "DNA", ambiguous: true },
        "S": { alias: "S", name: "strong (C/G)",          class: "Base", monomerType: "DNA", ambiguous: true }
    }
};

var _BIO_CELL_WIDTH = 60;
var _BIO_ROW_LEN = 20;

function bioBuildSequenceView(sequenceText, seqType) {
    _bioMonomers = [];
    _bioBonds = [];
    if (!sequenceText || typeof sequenceText !== "string") return;

    var type = (seqType || "PEPTIDE").toUpperCase();
    if (type === "PROTEIN") type = "PEPTIDE";
    _bioSeqType = type;
    var table = _STANDARD_MONOMER_TEMPLATES[type] || _STANDARD_MONOMER_TEMPLATES.PEPTIDE;

    var lines = sequenceText.split("\n").filter(function(l) { return l.trim().indexOf(">") !== 0; });
    var cleaned = lines.join("").replace(/[^A-Za-z]/g, "").toUpperCase();

    var prevId = null;
    for (var i = 0; i < cleaned.length; i++) {
        var ch = cleaned[i];
        var tpl = table[ch];
        if (!tpl) {
            console.warn("bioBuildSequenceView: unknown residue symbol: " + ch);
            continue;
        }
        var idx = _bioMonomers.length;
        var id = _bioNextId++;
        _bioMonomers.push({
            id: id,
            label: ch,
            alias: tpl.alias,
            x: (idx % _BIO_ROW_LEN) * _BIO_CELL_WIDTH,
            y: Math.floor(idx / _BIO_ROW_LEN) * _BIO_CELL_WIDTH * 2,
            monomerClass: tpl.class,
            ambiguous: !!tpl.ambiguous
        });
        if (prevId !== null) _bioBonds.push({ fromId: prevId, toId: id });
        prevId = id;
    }
}

function bioAddMonomer(symbol, seqType) {
    if (!symbol || typeof symbol !== "string") return;
    var type = (seqType || _bioSeqType || "PEPTIDE").toUpperCase();
    if (type === "PROTEIN") type = "PEPTIDE";
    var table = _STANDARD_MONOMER_TEMPLATES[type] || _STANDARD_MONOMER_TEMPLATES.PEPTIDE;
    var ch = symbol.trim().toUpperCase().charAt(0);
    var tpl = table[ch];
    if (!tpl) {
        console.warn("bioAddMonomer: unknown residue symbol: " + ch);
        return;
    }
    var prevId = _bioMonomers.length > 0 ? _bioMonomers[_bioMonomers.length - 1].id : null;
    var idx = _bioMonomers.length;
    var id = _bioNextId++;
    _bioMonomers.push({
        id: id,
        label: ch,
        alias: tpl.alias,
        x: (idx % _BIO_ROW_LEN) * _BIO_CELL_WIDTH,
        y: Math.floor(idx / _BIO_ROW_LEN) * _BIO_CELL_WIDTH * 2,
        monomerClass: tpl.class,
        ambiguous: !!tpl.ambiguous
    });
    if (prevId !== null) _bioBonds.push({ fromId: prevId, toId: id });
    _bioSeqType = type;
}

function bioDeleteMonomer(id) {
    var idx = -1;
    for (var i = 0; i < _bioMonomers.length; i++) {
        if (_bioMonomers[i].id === id) { idx = i; break; }
    }
    if (idx === -1) return;
    _bioMonomers.splice(idx, 1);

    var prevNeighbor = null, nextNeighbor = null;
    _bioBonds = _bioBonds.filter(function(b) {
        if (b.toId === id) { prevNeighbor = b.fromId; return false; }
        if (b.fromId === id) { nextNeighbor = b.toId; return false; }
        return true;
    });
    if (prevNeighbor !== null && nextNeighbor !== null) {
        _bioBonds.push({ fromId: prevNeighbor, toId: nextNeighbor });
    }

    // Recompact remaining boxes back into sequence order (same snake-layout formula
    // bioBuildSequenceView uses) so a middle deletion doesn't leave a visual gap.
    for (var j = 0; j < _bioMonomers.length; j++) {
        _bioMonomers[j].x = (j % _BIO_ROW_LEN) * _BIO_CELL_WIDTH;
        _bioMonomers[j].y = Math.floor(j / _BIO_ROW_LEN) * _BIO_CELL_WIDTH * 2;
    }
}

function bioGetSequenceViewSnapshot(reqId) {
    var snapshot = { monomers: _bioMonomers, bonds: _bioBonds, seqType: _bioSeqType };
    console.log(JSON.stringify({
        type: "structureResponse",
        reqId: reqId || "biopolymer_seq_view",
        data: JSON.stringify(snapshot)
    }));
    return snapshot;
}
