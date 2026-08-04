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

static QVariantMap atomPrimToVariant(const AtomPrim& a) {
    QVariantMap m;
    m["id"] = a.id;
    m["x"] = a.x;
    m["y"] = a.y;
    m["label"] = a.label;
    m["element"] = a.element;
    m["charge"] = a.charge;
    m["stereoLabel"] = a.stereoLabel;
    m["cipLabel"] = a.cipLabel;
    m["stereoType"] = a.stereoType;
    m["stereoGroup"] = a.stereoGroup;
    m["color"] = a.color;
    m["isRGroup"] = a.isRGroup;
    m["atomicNum"] = a.atomicNum;
    m["atomicTitle"] = a.atomicTitle;
    m["atomicMass"] = a.atomicMass;
    m["implicitHCount"] = a.implicitHCount;
    m["hOnLeft"] = a.hOnLeft;
    m["isotope"] = a.isotope;
    m["radical"] = a.radical;
    m["explicitValence"] = a.explicitValence;
    m["aam"] = a.aam;
    m["attachmentPoints"] = a.attachmentPoints;
    m["checkWarning"] = a.checkWarning;
    m["isAtomList"] = a.isAtomList;
    m["atomListElements"] = a.atomListElements;
    m["atomListNot"] = a.atomListNot;
    m["isSgroup"] = a.isSgroup;
    return m;
}

static QVariantMap bondPrimToVariant(const BondPrim& b) {
    QVariantMap m;
    m["id"] = b.id;
    m["begin"] = b.begin;
    m["end"] = b.end;
    m["type"] = b.type;
    m["stereo"] = b.stereo;
    m["inAromaticRing"] = b.inAromaticRing;
    m["invalidStereo"] = b.invalidStereo;
    m["checkWarning"] = b.checkWarning;
    m["beginIsSgroup"] = b.beginIsSgroup;
    m["endIsSgroup"] = b.endIsSgroup;
    m["cipLabel"] = b.cipLabel;
    m["reactingCenterStatus"] = b.reactingCenterStatus;
    if (b.hasRingCenter) {
        m["ringCenterX"] = b.ringCenterX;
        m["ringCenterY"] = b.ringCenterY;
    }
    return m;
}

static QVariantMap ringPrimToVariant(const RingPrim& r) {
    QVariantMap m;
    m["id"] = r.id;
    QVariantList atomIds;
    for (AtomId id : r.atoms) atomIds.append(id);
    m["atoms"] = atomIds;
    m["x"] = r.x;
    m["y"] = r.y;
    m["radius"] = r.radius;
    m["isAromatic"] = r.isAromatic;
    m["hasBondType4"] = r.hasBondType4;
    return m;
}

static QVariantMap sgroupPrimToVariant(const SgroupPrim& s) {
    QVariantMap m;
    m["id"] = s.id;
    m["label"] = s.label;
    m["x"] = s.x;
    m["y"] = s.y;
    m["attachAtomId"] = s.attachAtomId;
    return m;
}

static QVariantMap textPrimToVariant(const TextPrim& t) {
    QVariantMap m;
    m["id"] = t.id;
    m["x"] = t.x;
    m["y"] = t.y;
    m["content"] = t.content;
    m["bold"] = t.bold;
    m["italic"] = t.italic;
    return m;
}

static QVariantMap imagePrimToVariant(const ImagePrim& img) {
    QVariantMap m;
    m["id"] = img.id;
    m["x"] = img.x;
    m["y"] = img.y;
    m["w"] = img.w;
    m["h"] = img.h;
    m["bitmap"] = QString::fromUtf8(img.bitmap);
    return m;
}

static QVariantMap rxnArrowPrimToVariant(const RxnArrowPrim& r) {
    QVariantMap m;
    m["id"] = r.id;
    QVariantMap p1; p1["x"] = r.p1x; p1["y"] = r.p1y;
    QVariantMap p2; p2["x"] = r.p2x; p2["y"] = r.p2y;
    m["p1"] = p1;
    m["p2"] = p2;
    m["mode"] = r.mode;
    QVariantMap conditionsText;
    conditionsText["above"] = r.conditionsAbove;
    conditionsText["below"] = r.conditionsBelow;
    m["conditionsText"] = conditionsText;
    if (r.hasCurvature) {
        QVariantMap curv; curv["x"] = r.curvatureX; curv["y"] = r.curvatureY;
        m["curvature"] = curv;
    } else {
        m["curvature"] = QVariant();
    }
    return m;
}

static QVariantMap rxnPlusPrimToVariant(const RxnPlusPrim& p) {
    QVariantMap m;
    m["id"] = p.id;
    m["x"] = p.x;
    m["y"] = p.y;
    return m;
}

static QVariantMap multitailArrowPrimToVariant(const MultitailArrowPrim& mta) {
    QVariantMap m;
    m["id"] = mta.id;
    m["spineTopX"] = mta.headX;
    m["spineTopY"] = mta.headY;
    QVariantList tails;
    for (const QPointF& t : mta.tails) {
        QVariantMap tm; tm["x"] = t.x(); tm["y"] = t.y();
        tails.append(tm);
    }
    m["tails"] = tails;
    return m;
}

static QVariantMap rgroupPrimToVariant(const RGroupPrim& rg) {
    QVariantMap m;
    m["number"] = rg.number;
    m["range"] = rg.range;
    m["resth"] = rg.resth;
    m["ifthen"] = rg.ifthen;
    QVariantList members;
    for (const RGroupMemberPrim& mem : rg.members) {
        QVariantMap mm;
        mm["fragId"] = mem.fragId;
        QVariantList atomIds;
        for (AtomId id : mem.atomIds) atomIds.append(id);
        mm["atomIds"] = atomIds;
        members.append(mm);
    }
    m["members"] = members;
    return m;
}

static QVariantMap bracketPrimToVariant(const BracketPrim& b) {
    QVariantMap m;
    m["minX"] = b.minX;
    m["minY"] = b.minY;
    m["maxX"] = b.maxX;
    m["maxY"] = b.maxY;
    return m;
}

QVariantMap renderPrimitivesToVariant(const RenderPrimitives& prims) {
    QVariantMap root;

    // A contracted-sgroup pseudo-atom (AtomPrim::isSgroup) and its corresponding SgroupPrim
    // share the same id (RenderPrimitives.cpp:84,108: both assigned from the same sgId) --
    // AtomPrim itself has no attachAtomId field, but the real JSON's equivalent object is
    // literally the SAME object pushed into both atomsArray and sgroupsArray
    // (10-state.js:1162-1164), so its attachAtomId is present on the atoms-list entry too.
    // Synthesize that here so a QML consumer reading attachAtomId off an atomsById lookup
    // (e.g. ChemCanvas.qml:460) sees the same value it would from the real JS.
    QHash<int, AtomId> sgroupAttachAtomById;
    for (const SgroupPrim& s : prims.sgroups) sgroupAttachAtomById.insert(s.id, s.attachAtomId);

    QVariantList atoms;
    QVariantMap atomsById;
    for (const AtomPrim& a : prims.atoms) {
        QVariantMap av = atomPrimToVariant(a);
        if (a.isSgroup && sgroupAttachAtomById.contains(a.id)) {
            av["attachAtomId"] = sgroupAttachAtomById.value(a.id);
        }
        atoms.append(av);
        atomsById[QString::number(a.id)] = av;
    }
    root["atoms"] = atoms;
    root["atomsById"] = atomsById;

    QVariantList bonds;
    for (const BondPrim& b : prims.bonds) bonds.append(bondPrimToVariant(b));
    root["bonds"] = bonds;

    QVariantList rings;
    for (const RingPrim& r : prims.rings) rings.append(ringPrimToVariant(r));
    root["rings"] = rings;

    QVariantList sgroups;
    for (const SgroupPrim& s : prims.sgroups) sgroups.append(sgroupPrimToVariant(s));
    root["sgroups"] = sgroups;

    QVariantList rxnArrows;
    for (const RxnArrowPrim& r : prims.rxnArrows) rxnArrows.append(rxnArrowPrimToVariant(r));
    root["rxnArrows"] = rxnArrows;

    QVariantList rxnPluses;
    for (const RxnPlusPrim& p : prims.rxnPluses) rxnPluses.append(rxnPlusPrimToVariant(p));
    root["rxnPluses"] = rxnPluses;

    QVariantList multitailArrows;
    for (const MultitailArrowPrim& m : prims.multitailArrows) multitailArrows.append(multitailArrowPrimToVariant(m));
    root["multitailArrows"] = multitailArrows;

    if (prims.bbox.valid) {
        QVariantMap bbox;
        bbox["minX"] = prims.bbox.minX;
        bbox["minY"] = prims.bbox.minY;
        bbox["maxX"] = prims.bbox.maxX;
        bbox["maxY"] = prims.bbox.maxY;
        root["bbox"] = bbox;
    } else {
        root["bbox"] = QVariant();
    }

    QVariantMap stereoFlags;
    stereoFlags["type"] = prims.stereoFlagsType;
    stereoFlags["groupId"] = prims.stereoFlagsGroupId;
    root["stereoFlags"] = stereoFlags;

    QVariantList texts;
    for (const TextPrim& t : prims.texts) texts.append(textPrimToVariant(t));
    root["texts"] = texts;

    QVariantList images;
    for (const ImagePrim& img : prims.images) images.append(imagePrimToVariant(img));
    root["images"] = images;

    QVariantList rgroups;
    for (const RGroupPrim& rg : prims.rgroups) rgroups.append(rgroupPrimToVariant(rg));
    root["rgroups"] = rgroups;

    QVariantList brackets;
    for (const BracketPrim& b : prims.brackets) brackets.append(bracketPrimToVariant(b));
    root["brackets"] = brackets;

    return root;
}
