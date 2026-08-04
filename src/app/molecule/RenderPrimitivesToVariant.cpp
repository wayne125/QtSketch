// src/app/molecule/RenderPrimitivesToVariant.cpp
#include "RenderPrimitivesToVariant.h"
#include <QVariantList>

QVariantMap selectionStateToVariant(const SelectionState& sel) {
    QVariantMap m;
    QVariantList atomIds;
    for (AtomId id : sel.atoms) atomIds.append(id);
    QVariantList bondIds;
    for (BondId id : sel.bonds) bondIds.append(id);
    QVariantList rxnArrowIds;
    for (RxnArrowId id : sel.rxnArrows) rxnArrowIds.append(id);
    QVariantList rxnPlusIds;
    for (RxnPlusId id : sel.rxnPluses) rxnPlusIds.append(id);
    QVariantList multitailArrowIds;
    for (MultitailArrowId id : sel.multitailArrows) multitailArrowIds.append(id);

    m["atom_ids"] = atomIds;
    m["bond_ids"] = bondIds;
    m["rxnArrow_ids"] = rxnArrowIds;
    m["rxnPlus_ids"] = rxnPlusIds;
    m["multitailArrow_ids"] = multitailArrowIds;
    m["bbox"] = QVariant(); // always null in every real code path
    return m;
}
