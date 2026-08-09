// src/app/molecule/ClipboardPreview.cpp
#include "ClipboardPreview.h"
#include "EditableMolecule.h"

ClipboardPreview buildClipboardPreview(const QString& molfileText) {
    ClipboardPreview preview;
    if (molfileText.isEmpty()) return preview;

    EditableMolecule mol(molfileText);
    if (!mol.isValid() || mol.atomCount() == 0) return preview;

    QHash<AtomId, QPointF> positions;
    double minX = 0, maxX = 0, minY = 0, maxY = 0;
    bool any = false;
    for (AtomId id : mol.atomIds()) {
        double x = 0, y = 0;
        if (!mol.atomPos(id, x, y)) continue;
        positions.insert(id, QPointF(x, y));
        preview.atoms.append(ClipboardPreviewAtom{x, y, mol.atomSymbol(id)});
        if (!any) { minX = maxX = x; minY = maxY = y; any = true; }
        else {
            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;
        }
    }

    for (BondId id : mol.bondIds()) {
        AtomId a = -1, b = -1;
        if (!mol.bondEndpoints(id, a, b)) continue;
        if (!positions.contains(a) || !positions.contains(b)) continue;
        QPointF p1 = positions.value(a);
        QPointF p2 = positions.value(b);
        preview.bonds.append(ClipboardPreviewBond{p1.x(), p1.y(), p2.x(), p2.y()});
    }

    if (any) {
        preview.cx = (minX + maxX) / 2.0;
        preview.cy = (minY + maxY) / 2.0;
    }
    return preview;
}
