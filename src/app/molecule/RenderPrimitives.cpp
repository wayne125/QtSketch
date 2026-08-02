// src/app/molecule/RenderPrimitives.cpp
#include "RenderPrimitives.h"
#include "ElementData.h"
#include <QRegularExpression>
#include <QStringList>
#include <QSet>
#include <QHash>
#include <cmath>

namespace {

// Ported from chem-core.js:12-18's isRGroupLabel. A cosmetic per-atom label check, NOT the
// deferred r-group data model (fragment membership, range/resth/ifthen policy).
bool isRGroupLabel(const QString& label) {
    static const QRegularExpression kPattern(QStringLiteral("^R[1-8]$"));
    return kPattern.match(label).hasMatch();
}

// Ported from chem-core.js:9082-9103's isCorrectStereoCenter -- ~20 lines of pure
// neighbor-count/implicit-H-parity logic with no Indigo dependency. Takes plain values already
// available via EditableMolecule's accessors, rather than re-deriving anything from a raw handle.
//
// endOtherNeighborCount is the neighbor-count of end's OTHER neighbor (the one that isn't begin),
// only meaningful (and only read) when endNeighborCount == 2; pass 0 when not applicable.
bool isCorrectStereoCenter(int bondStereo, int beginNeighborCount, int endNeighborCount,
                           int beginImplicitH, int endOtherNeighborCount) {
    if (bondStereo <= 0) return false;
    if (endNeighborCount == 1 && beginNeighborCount == 2 && beginImplicitH % 2 == 0) return false;
    if (endNeighborCount == 2 && beginNeighborCount == 2 && beginImplicitH % 2 == 0
        && endOtherNeighborCount == 1) return false;
    if (beginNeighborCount == 1) return false;
    return true;
}

// Display-string mapping over (stereocenterType, stereocenterGroup), matching chem-core's own
// "abs"/"&N"/"orN" convention. INDIGO_ABS=1, INDIGO_OR=2, INDIGO_AND=3, INDIGO_EITHER=4
// (indigo.h:435-438).
QString stereoLabelFor(int type, int group) {
    switch (type) {
        case 1: return QStringLiteral("abs");
        case 3: return QStringLiteral("&") + QString::number(group);
        case 2: return QStringLiteral("or") + QString::number(group);
        default: return QString();   // INDIGO_EITHER or not a stereocenter
    }
}

// Maps Indigo's CIPDesc enum (NONE=0, UNKNOWN=1, s=2, r=3, S=4, R=5, E=6, Z=7 --
// molecule_cip_calculator.h:38-48) to its display string.
QString cipDescriptorLabel(int cip) {
    switch (cip) {
        case 2: return QStringLiteral("s");
        case 3: return QStringLiteral("r");
        case 4: return QStringLiteral("S");
        case 5: return QStringLiteral("R");
        case 6: return QStringLiteral("E");
        case 7: return QStringLiteral("Z");
        default: return QString();   // NONE or UNKNOWN
    }
}

} // namespace

RenderPrimitives RenderPrimitiveBuilder::build(const EditableMolecule& mol, bool showExplicitH) {
    RenderPrimitives result;

    // ---- Sgroup contraction: build the atom->sgroup map and synthetic SgroupPrim list first ----
    // sgroupIds() enumerates every currently-existing sgroup; sgroupMemberAtomIds() gives each
    // one's FULL member-atom set (not just its attach atom), so contraction hides every member
    // atom exactly like chem-core's real sg.atoms-based contraction. A HASH (atom -> owning
    // sgroup id), not just a boolean set, is needed so Task 6's bond-primitive step can
    // distinguish "both endpoints hidden by the SAME sgroup" (skip the bond entirely) from
    // "both endpoints hidden by DIFFERENT sgroups" or "exactly one endpoint hidden" (substitute
    // the hidden endpoint(s) with their sgroup's synthetic id, matching buildRenderPrimitives'
    // own beginSgId/endSgId substitution exactly).
    QHash<AtomId, SGroupId> atomToSgroup;
    for (SGroupId sgId : mol.sgroupIds()) {
        if (mol.sgroupExpanded(sgId)) continue;   // expanded: render its atoms normally, no contraction

        AtomId attach = -1;
        mol.superatomAttachAtom(sgId, attach);   // -1 if none resolves; position falls back below

        SgroupPrim prim;
        prim.id = sgId;
        prim.attachAtomId = attach;
        prim.label = QString();   // chem-core reads sg.data.name; this port has no stored sgroup
                                   // label anywhere (sub-project 3d's insertStructure discovers
                                   // sgroups from indigoMerge but never captures a display name) --
                                   // documented gap, empty string rather than a guess.

        QList<AtomId> members = mol.sgroupMemberAtomIds(sgId);
        double px = 0, py = 0;
        if (attach >= 0 && mol.atomPos(attach, px, py)) {
            prim.x = px; prim.y = py;
        } else if (!members.isEmpty()) {
            // No attachment point resolved: fall back to the member centroid, matching
            // buildRenderPrimitives' own attachAtomId-else-centroid fallback.
            double sx = 0, sy = 0; int n = 0;
            for (AtomId aid : members) {
                double ax = 0, ay = 0;
                if (mol.atomPos(aid, ax, ay)) { sx += ax; sy += ay; ++n; }
            }
            if (n > 0) { prim.x = sx / n; prim.y = sy / n; }
        }
        result.sgroups.append(prim);

        AtomPrim synthetic;
        synthetic.id = sgId;
        synthetic.x = prim.x; synthetic.y = prim.y;
        synthetic.label = prim.label;
        synthetic.isSgroup = true;
        result.atoms.append(synthetic);

        for (AtomId aid : members) atomToSgroup.insert(aid, sgId);
    }

    // ---- Atom primitives ----
    double minX = 0, maxX = 0, minY = 0, maxY = 0;
    bool haveBBox = false;
    for (const AtomPrim& sg : result.atoms) {
        if (!haveBBox) { minX = maxX = sg.x; minY = maxY = sg.y; haveBBox = true; }
        else {
            if (sg.x < minX) minX = sg.x; if (sg.x > maxX) maxX = sg.x;
            if (sg.y < minY) minY = sg.y; if (sg.y > maxY) maxY = sg.y;
        }
    }

    for (AtomId id : mol.atomIds()) {
        if (atomToSgroup.contains(id)) continue;

        AtomPrim prim;
        prim.id = id;
        double x = 0, y = 0;
        mol.atomPos(id, x, y);
        prim.x = x; prim.y = y;

        QString symbol = mol.atomSymbol(id);
        prim.element = symbol;
        prim.isAtomList = mol.hasAtomQueryList(id);

        bool rgroupFlag = isRGroupLabel(symbol);
        prim.isRGroup = rgroupFlag;

        QList<AtomId> neighbors = mol.neighborAtomIds(id);
        bool isHetero = (symbol != QStringLiteral("C") && symbol != QStringLiteral("H"));
        bool isTerminal = neighbors.size() <= 1;
        if (rgroupFlag || prim.isAtomList) { isHetero = false; isTerminal = false; }

        QString renderLabel = symbol;
        int implicitH = mol.implicitHydrogenCount(id);
        if (prim.isAtomList) {
            QList<int> nums = mol.atomQueryListNumbers(id);
            QStringList labels;
            for (int n : nums) {
                QString sym = ElementData::symbolForNumber(n);
                labels.append(sym.isEmpty() ? QStringLiteral("?") : sym);
            }
            prim.atomListElements = labels.join(QStringLiteral(","));
            prim.atomListNot = mol.atomQueryListIsNotList(id);
            renderLabel = (prim.atomListNot ? QStringLiteral("!") : QString())
                          + QStringLiteral("[") + prim.atomListElements + QStringLiteral("]");
        } else if ((showExplicitH || isHetero || isTerminal) && implicitH > 0) {
            renderLabel += (implicitH == 1) ? QStringLiteral("H")
                                             : (QStringLiteral("H") + QString::number(implicitH));
        }
        prim.label = renderLabel;
        prim.implicitHCount = implicitH > 0 ? implicitH : 0;

        const ElementData::ElementInfo* info = ElementData::infoFor(symbol);
        prim.atomicNum = info ? info->number : 0;
        prim.atomicTitle = info ? info->title : QString();
        prim.atomicMass = info ? (std::round(info->mass * 1000.0) / 1000.0) : 0.0;
        prim.color = rgroupFlag ? QStringLiteral("#7B68EE") : ElementData::colorFor(symbol);

        prim.charge = mol.atomCharge(id);
        prim.isotope = mol.atomIsotope(id);
        prim.radical = mol.atomRadical(id);
        prim.explicitValence = mol.atomExplicitValence(id);
        prim.aam = mol.atomAAM(id);
        prim.checkWarning = mol.atomCheckWarningText(id);

        int stType = mol.stereocenterType(id);
        int stGroup = mol.stereocenterGroup(id);
        prim.stereoType = stType;
        prim.stereoGroup = stGroup;
        prim.stereoLabel = stereoLabelFor(stType, stGroup);
        prim.cipLabel = cipDescriptorLabel(mol.atomCipDescriptor(id));

        double nbXSum = 0; int nbCount = 0;
        for (AtomId nb : neighbors) {
            double nx = 0, ny = 0;
            if (mol.atomPos(nb, nx, ny)) { nbXSum += nx; ++nbCount; }
        }
        prim.hOnLeft = nbCount > 0 && (nbXSum / nbCount) > x;

        result.atoms.append(prim);

        if (!haveBBox) { minX = maxX = x; minY = maxY = y; haveBBox = true; }
        else {
            if (x < minX) minX = x; if (x > maxX) maxX = x;
            if (y < minY) minY = y; if (y > maxY) maxY = y;
        }
    }

    result.bbox.valid = haveBBox;
    if (haveBBox) { result.bbox.minX = minX; result.bbox.minY = minY; result.bbox.maxX = maxX; result.bbox.maxY = maxY; }

    result.stereoFlagsType = mol.stereoFlagsType();
    result.stereoFlagsGroupId = mol.stereoFlagsGroupId();

    return result;
}
