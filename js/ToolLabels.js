.pragma library

// Maps a currentTool id (an internal string like "TEMPLATE_BENZENE" or
// "BOND_UP") to the same user-facing label already shown as its tip/tooltip
// in ToolPanel.qml/MainToolbar.qml/AppMenus.qml -- kept here as the single
// source of truth instead of duplicating each string a second time next to
// its tip, so the two can't drift out of sync.
const TOOL_LABELS = {
    SELECT: "Selection",
    SELECT_FRAGMENT: "Fragment Selection",
    SELECT_LASSO: "Lasso Select",
    HAND: "Pan",
    ERASE: "Erase",
    TEXT: "Text Annotation",
    IMAGE: "Insert Image",
    BOND_1: "Single Bond",
    BOND_2: "Double Bond",
    BOND_3: "Triple Bond",
    BOND_UP: "Wedge Bond",
    BOND_DOWN: "Dash Bond",
    BOND_UPDOWN: "Wavy Bond",
    CHAIN: "Chain",
    TEMPLATE_BENZENE: "Benzene",
    LIB_Pyridine: "Pyridine",
    TEMPLATE_6: "Cyclohexane",
    TEMPLATE_5: "Cyclopentane",
    TEMPLATE_4: "Cyclobutane",
    TEMPLATE_3: "Cyclopropane",
    TEMPLATE_7: "Cycloheptane",
    TEMPLATE_8: "Cyclooctane",
    RXN_ARROW: "Reaction Arrow",
    MULTITAIL_ARROW: "Multi-tail Arrow",
    RXN_PLUS: "Reaction Plus",
    AAM: "Atom-Atom Mapping",
    ATOM_A: "Any Atom",
    ATOM_AH: "Any Atom or H",
    ATOM_Q: "Any Heteroatom",
    ATOM_QH: "Heteroatom or H",
    ATOM_M: "Any Metal",
    ATOM_MH: "Metal or H",
    ATOM_X: "Any Halogen",
    ATOM_XH: "Halogen or H",
    ATOM_C: "Carbon",
    ATOM_N: "Nitrogen",
    ATOM_O: "Oxygen",
    ATOM_S: "Sulfur",
    ATOM_F: "Fluorine",
    ATOM_Cl: "Chlorine",
    ATOM_ANY: "Periodic Table",
    CHARGE_PLUS: "Charge Plus",
    CHARGE_MINUS: "Charge Minus"
}

function label(toolId) {
    if (!toolId) return ""
    if (TOOL_LABELS[toolId] !== undefined) return TOOL_LABELS[toolId]
    // Periodic-table element picks and R-group labels are set dynamically as
    // "ATOM_" + <element symbol or "R1".."R8"> and can't be enumerated ahead
    // of time -- fall back to just the suffix rather than the raw "ATOM_Na".
    if (toolId.indexOf("ATOM_") === 0) return toolId.substring(5)
    return toolId
}
