// src/app/molecule/SdfBatch.cpp
#include "SdfBatch.h"
#include "EditableMolecule.h"
#include "RenderPrimitives.h"
#include "indigo.h"
#include <algorithm>
#include <QRegularExpression>

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
    // ingestFromHandle's thumbnail step may have silently stolen the active session (see
    // loadFromSdfText's loop comment for the full explanation) -- re-activate before freeing
    // `handle`, which belongs to m_session, not whatever session is active now.
    activateSession();
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

bool SdfBatch::loadFromSdfText(const QString& sdfText) {
    if (sdfText.trimmed().isEmpty()) return false;
    activateSession();

    static const QRegularExpression kGLine(QStringLiteral("^G\\s+\\d+\\s+\\d+\\s*$"),
                                            QRegularExpression::MultilineOption);

    QByteArray utf8 = sdfText.toUtf8();   // must stay alive for the whole loop below --
                                           // indigoReadBuffer does not copy the buffer
    int reader = indigoReadBuffer(utf8.constData(), utf8.size());
    if (reader < 0) return false;
    int iter = indigoIterateSDF(reader);
    if (iter < 0) { indigoFree(reader); return false; }

    QList<BatchRecord> parsed;
    int index = 0;
    int item;
    while ((item = indigoNext(iter)) > 0) {
        int molHandle = item;
        bool ownsFallbackHandle = false;
        if (indigoCountAtoms(item) < 0) {
            const char* raw = indigoRawData(item);
            QString text = raw ? QString::fromUtf8(raw) : QString();
            text.remove(kGLine);
            molHandle = text.isEmpty() ? -1 : indigoLoadMoleculeFromString(text.toUtf8().constData());
            ownsFallbackHandle = true;
        }
        if (molHandle >= 0) {
            parsed.append(ingestFromHandle(molHandle, index, /*withProps=*/true, index < 500));
            ++index;
            // ingestFromHandle's thumbnail step constructs a transient EditableMolecule, whose
            // constructor allocates and activates ITS OWN Indigo session -- silently stealing the
            // active session away from this loop's `reader`/`iter` (which belong to m_session).
            // Re-activate before touching `item`/`iter`/`reader` again, or the next indigoNext(iter)
            // call operates under the wrong (or no) session. Confirmed by direct probe: without
            // this, only the first record's thumbnail computation succeeds and subsequent records
            // are silently lost.
            activateSession();
        }
        if (ownsFallbackHandle && molHandle >= 0) indigoFree(molHandle);
        indigoFree(item);
    }
    indigoFree(iter);
    indigoFree(reader);

    m_records = parsed;
    return true;
}

bool SdfBatch::realign(const QStringList& alignedMolfiles) {
    if (alignedMolfiles.size() != m_records.size()) return false;
    activateSession();

    QList<BatchRecord> result;
    for (int i = 0; i < alignedMolfiles.size(); ++i) {
        std::optional<BatchRecord> rec = ingestFromText(alignedMolfiles.at(i), i, i < 500);
        result.append(rec ? *rec : m_records.at(i));
    }
    m_records = result;
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
