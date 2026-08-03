// src/app/molecule/SdfBatch.cpp
#include "SdfBatch.h"
#include "EditableMolecule.h"
#include "RenderPrimitives.h"
#include "indigo.h"
#include <algorithm>

SdfBatch::SdfBatch() {
    m_session = indigoAllocSessionId();
}

SdfBatch::~SdfBatch() {
    indigoReleaseSessionId(m_session);
}

void SdfBatch::activateSession() const {
    indigoSetSessionId(m_session);
}

static SdfBatch::Thumbnail buildThumbnail(const QString& molfileText) {
    SdfBatch::Thumbnail thumb;
    EditableMolecule mol(molfileText);
    if (!mol.isValid()) return thumb;
    RenderPrimitives rp = RenderPrimitiveBuilder::build(mol, /*showExplicitH=*/false);
    if (!rp.bbox.valid) return thumb;

    double w = rp.bbox.maxX - rp.bbox.minX; if (w == 0.0) w = 1.0;
    double h = rp.bbox.maxY - rp.bbox.minY; if (h == 0.0) h = 1.0;
    const double pad = 0.1;
    double scale = (1.0 - 2.0 * pad) / std::max(w, h);
    double offX = pad + (1.0 - 2.0 * pad - w * scale) / 2.0;
    double offY = pad + (1.0 - 2.0 * pad - h * scale) / 2.0;

    QHash<AtomId, QPointF> normalizedPos;
    for (const AtomPrim& a : rp.atoms) {
        double nx = offX + (a.x - rp.bbox.minX) * scale;
        double ny = offY + (a.y - rp.bbox.minY) * scale;
        normalizedPos.insert(a.id, QPointF(nx, ny));
        thumb.atoms.append(SdfBatch::ThumbnailAtom{nx, ny, a.element});
    }
    for (const BondPrim& b : rp.bonds) {
        if (!normalizedPos.contains(b.begin) || !normalizedPos.contains(b.end)) continue;
        QPointF p1 = normalizedPos.value(b.begin);
        QPointF p2 = normalizedPos.value(b.end);
        thumb.bonds.append(SdfBatch::ThumbnailBond{p1.x(), p1.y(), p2.x(), p2.y(), b.type});
    }
    return thumb;
}

SdfBatch::BatchRecord SdfBatch::ingestFromHandle(int handle, int indexForLabel, bool withProps, bool computeThumbnail) const {
    BatchRecord rec;
    const char* mf = indigoMolfile(handle);
    rec.molfileText = mf ? QString::fromUtf8(mf) : QString();

    const char* name = indigoName(handle);
    QString qname = name ? QString::fromUtf8(name).trimmed() : QString();
    rec.label = !qname.isEmpty() ? qname : (QStringLiteral("Record ") + QString::number(indexForLabel + 1));

    if (withProps) {
        int propIter = indigoIterateProperties(handle);
        if (propIter >= 0) {
            int prop;
            while ((prop = indigoNext(propIter)) > 0) {
                const char* key = indigoName(prop);
                if (key) {
                    const char* val = indigoGetProperty(handle, key);
                    rec.props.insert(QString::fromUtf8(key), val ? QString::fromUtf8(val) : QString());
                }
                indigoFree(prop);
            }
            indigoFree(propIter);
        }
    }

    if (computeThumbnail) {
        rec.thumbnail = buildThumbnail(rec.molfileText);
    }

    return rec;
}

std::optional<SdfBatch::BatchRecord> SdfBatch::ingestFromText(const QString& text, int indexForLabel, bool computeThumbnail) const {
    activateSession();
    int handle = indigoLoadMoleculeFromString(text.toUtf8().constData());
    if (handle < 0) return std::nullopt;
    BatchRecord rec = ingestFromHandle(handle, indexForLabel, /*withProps=*/false, computeThumbnail);
    indigoFree(handle);
    return rec;
}

bool SdfBatch::loadFromMolfileList(const QStringList& molfiles) {
    if (molfiles.isEmpty()) return false;
    activateSession();

    QList<BatchRecord> parsed;
    int index = 0;
    for (const QString& text : molfiles) {
        std::optional<BatchRecord> rec = ingestFromText(text, index, index < 500);
        if (rec) { parsed.append(*rec); ++index; }
    }
    m_records = parsed;
    return true;
}

int SdfBatch::recordCount() const { return m_records.size(); }

QString SdfBatch::molfileAt(int index) const {
    if (index < 0 || index >= m_records.size()) return QString();
    return m_records.at(index).molfileText;
}

QString SdfBatch::labelAt(int index) const {
    if (index < 0 || index >= m_records.size()) return QString();
    return m_records.at(index).label;
}

QHash<QString, QString> SdfBatch::propsAt(int index) const {
    if (index < 0 || index >= m_records.size()) return {};
    return m_records.at(index).props;
}

SdfBatch::Thumbnail SdfBatch::thumbnailAt(int index) const {
    if (index < 0 || index >= m_records.size()) return {};
    return m_records.at(index).thumbnail;
}
