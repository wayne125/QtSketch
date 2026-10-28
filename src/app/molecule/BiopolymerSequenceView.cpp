// src/app/molecule/BiopolymerSequenceView.cpp
#include "BiopolymerSequenceView.h"
#include <QRegularExpression>
#include <QStringList>

QString BiopolymerSequenceView::normalizeSeqType(const QString& seqType, const QString& fallback) {
    QString type = seqType.isEmpty() ? fallback : seqType.toUpper();
    if (type == QStringLiteral("PROTEIN")) type = QStringLiteral("PEPTIDE");
    return type;
}

const QHash<QString, BiopolymerSequenceView::MonomerTemplate>&
BiopolymerSequenceView::tableFor(const QString& normalizedType) {
    static const QHash<QString, MonomerTemplate> peptide = {
        {QStringLiteral("A"), {QStringLiteral("Ala"), QStringLiteral("AminoAcid"), false}},
        {QStringLiteral("C"), {QStringLiteral("Cys"), QStringLiteral("AminoAcid"), false}},
        {QStringLiteral("D"), {QStringLiteral("Asp"), QStringLiteral("AminoAcid"), false}},
        {QStringLiteral("E"), {QStringLiteral("Glu"), QStringLiteral("AminoAcid"), false}},
        {QStringLiteral("F"), {QStringLiteral("Phe"), QStringLiteral("AminoAcid"), false}},
        {QStringLiteral("G"), {QStringLiteral("Gly"), QStringLiteral("AminoAcid"), false}},
        {QStringLiteral("H"), {QStringLiteral("His"), QStringLiteral("AminoAcid"), false}},
        {QStringLiteral("I"), {QStringLiteral("Ile"), QStringLiteral("AminoAcid"), false}},
        {QStringLiteral("K"), {QStringLiteral("Lys"), QStringLiteral("AminoAcid"), false}},
        {QStringLiteral("L"), {QStringLiteral("Leu"), QStringLiteral("AminoAcid"), false}},
        {QStringLiteral("M"), {QStringLiteral("Met"), QStringLiteral("AminoAcid"), false}},
        {QStringLiteral("N"), {QStringLiteral("Asn"), QStringLiteral("AminoAcid"), false}},
        {QStringLiteral("P"), {QStringLiteral("Pro"), QStringLiteral("AminoAcid"), false}},
        {QStringLiteral("Q"), {QStringLiteral("Gln"), QStringLiteral("AminoAcid"), false}},
        {QStringLiteral("R"), {QStringLiteral("Arg"), QStringLiteral("AminoAcid"), false}},
        {QStringLiteral("S"), {QStringLiteral("Ser"), QStringLiteral("AminoAcid"), false}},
        {QStringLiteral("T"), {QStringLiteral("Thr"), QStringLiteral("AminoAcid"), false}},
        {QStringLiteral("V"), {QStringLiteral("Val"), QStringLiteral("AminoAcid"), false}},
        {QStringLiteral("W"), {QStringLiteral("Trp"), QStringLiteral("AminoAcid"), false}},
        {QStringLiteral("Y"), {QStringLiteral("Tyr"), QStringLiteral("AminoAcid"), false}},
        {QStringLiteral("B"), {QStringLiteral("Asx"), QStringLiteral("AminoAcid"), true}},
        {QStringLiteral("Z"), {QStringLiteral("Glx"), QStringLiteral("AminoAcid"), true}},
        {QStringLiteral("J"), {QStringLiteral("Xle"), QStringLiteral("AminoAcid"), true}},
        {QStringLiteral("X"), {QStringLiteral("Xaa"), QStringLiteral("AminoAcid"), true}},
    };
    static const QHash<QString, MonomerTemplate> rna = {
        {QStringLiteral("A"), {QStringLiteral("A"), QStringLiteral("Base"), false}},
        {QStringLiteral("C"), {QStringLiteral("C"), QStringLiteral("Base"), false}},
        {QStringLiteral("G"), {QStringLiteral("G"), QStringLiteral("Base"), false}},
        {QStringLiteral("U"), {QStringLiteral("U"), QStringLiteral("Base"), false}},
        {QStringLiteral("N"), {QStringLiteral("N"), QStringLiteral("Base"), true}},
        {QStringLiteral("B"), {QStringLiteral("B"), QStringLiteral("Base"), true}},
        {QStringLiteral("V"), {QStringLiteral("V"), QStringLiteral("Base"), true}},
        {QStringLiteral("D"), {QStringLiteral("D"), QStringLiteral("Base"), true}},
        {QStringLiteral("H"), {QStringLiteral("H"), QStringLiteral("Base"), true}},
        {QStringLiteral("K"), {QStringLiteral("K"), QStringLiteral("Base"), true}},
        {QStringLiteral("M"), {QStringLiteral("M"), QStringLiteral("Base"), true}},
        {QStringLiteral("W"), {QStringLiteral("W"), QStringLiteral("Base"), true}},
        {QStringLiteral("Y"), {QStringLiteral("Y"), QStringLiteral("Base"), true}},
        {QStringLiteral("R"), {QStringLiteral("R"), QStringLiteral("Base"), true}},
        {QStringLiteral("S"), {QStringLiteral("S"), QStringLiteral("Base"), true}},
    };
    static const QHash<QString, MonomerTemplate> dna = {
        {QStringLiteral("A"), {QStringLiteral("dA"), QStringLiteral("Base"), false}},
        {QStringLiteral("C"), {QStringLiteral("dC"), QStringLiteral("Base"), false}},
        {QStringLiteral("G"), {QStringLiteral("dG"), QStringLiteral("Base"), false}},
        {QStringLiteral("T"), {QStringLiteral("dT"), QStringLiteral("Base"), false}},
        {QStringLiteral("N"), {QStringLiteral("N"), QStringLiteral("Base"), true}},
        {QStringLiteral("B"), {QStringLiteral("B"), QStringLiteral("Base"), true}},
        {QStringLiteral("V"), {QStringLiteral("V"), QStringLiteral("Base"), true}},
        {QStringLiteral("D"), {QStringLiteral("D"), QStringLiteral("Base"), true}},
        {QStringLiteral("H"), {QStringLiteral("H"), QStringLiteral("Base"), true}},
        {QStringLiteral("K"), {QStringLiteral("K"), QStringLiteral("Base"), true}},
        {QStringLiteral("M"), {QStringLiteral("M"), QStringLiteral("Base"), true}},
        {QStringLiteral("W"), {QStringLiteral("W"), QStringLiteral("Base"), true}},
        {QStringLiteral("Y"), {QStringLiteral("Y"), QStringLiteral("Base"), true}},
        {QStringLiteral("R"), {QStringLiteral("R"), QStringLiteral("Base"), true}},
        {QStringLiteral("S"), {QStringLiteral("S"), QStringLiteral("Base"), true}},
    };
    if (normalizedType == QStringLiteral("RNA")) return rna;
    if (normalizedType == QStringLiteral("DNA")) return dna;
    return peptide;
}

void BiopolymerSequenceView::buildSequenceView(const QString& sequenceText, const QString& seqType) {
    m_monomers.clear();
    m_bonds.clear();
    if (sequenceText.isEmpty()) return;

    QString type = normalizeSeqType(seqType, QStringLiteral("PEPTIDE"));
    m_seqType = type;
    const QHash<QString, MonomerTemplate>& table = tableFor(type);

    QStringList lines = sequenceText.split(QLatin1Char('\n'));
    QString cleaned;
    for (const QString& line : lines) {
        if (line.trimmed().startsWith(QLatin1Char('>'))) continue;
        cleaned += line;
    }
    cleaned.remove(QRegularExpression(QStringLiteral("[^A-Za-z]")));
    cleaned = cleaned.toUpper();

    int prevId = -1;
    for (int i = 0; i < cleaned.size(); ++i) {
        QString ch(cleaned[i]);
        if (!table.contains(ch)) continue;
        const MonomerTemplate& tpl = table.value(ch);
        int idx = m_monomers.size();
        int id = m_nextId++;
        BioMonomer m;
        m.id = id;
        m.label = ch;
        m.alias = tpl.alias;
        m.x = (idx % kRowLen) * kCellWidth;
        m.y = (idx / kRowLen) * kCellWidth * 2;
        m.monomerClass = tpl.monomerClass;
        m.ambiguous = tpl.ambiguous;
        m_monomers.append(m);
        if (prevId != -1) m_bonds.append({prevId, id});
        prevId = id;
    }
}

void BiopolymerSequenceView::addMonomer(const QString& symbol, const QString& seqType) {
    if (symbol.isEmpty()) return;
    QString type = normalizeSeqType(seqType, m_seqType.isEmpty() ? QStringLiteral("PEPTIDE") : m_seqType);
    const QHash<QString, MonomerTemplate>& table = tableFor(type);
    QString ch = symbol.trimmed().toUpper().left(1);
    if (!table.contains(ch)) return;
    const MonomerTemplate& tpl = table.value(ch);

    int prevId = m_monomers.isEmpty() ? -1 : m_monomers.last().id;
    int idx = m_monomers.size();
    int id = m_nextId++;
    BioMonomer m;
    m.id = id;
    m.label = ch;
    m.alias = tpl.alias;
    m.x = (idx % kRowLen) * kCellWidth;
    m.y = (idx / kRowLen) * kCellWidth * 2;
    m.monomerClass = tpl.monomerClass;
    m.ambiguous = tpl.ambiguous;
    m_monomers.append(m);
    if (prevId != -1) m_bonds.append({prevId, id});
    m_seqType = type;
}

void BiopolymerSequenceView::deleteMonomer(int id) {
    int idx = -1;
    for (int i = 0; i < m_monomers.size(); ++i) {
        if (m_monomers[i].id == id) { idx = i; break; }
    }
    if (idx == -1) return;
    m_monomers.removeAt(idx);

    int prevNeighbor = -1, nextNeighbor = -1;
    QList<BioBond> kept;
    for (const BioBond& b : m_bonds) {
        if (b.toId == id) { prevNeighbor = b.fromId; continue; }
        if (b.fromId == id) { nextNeighbor = b.toId; continue; }
        kept.append(b);
    }
    m_bonds = kept;
    if (prevNeighbor != -1 && nextNeighbor != -1) {
        m_bonds.append({prevNeighbor, nextNeighbor});
    }

    recompactLayout();
}

void BiopolymerSequenceView::recompactLayout() {
    for (int j = 0; j < m_monomers.size(); ++j) {
        m_monomers[j].x = (j % kRowLen) * kCellWidth;
        m_monomers[j].y = (j / kRowLen) * kCellWidth * 2;
    }
}

QList<BioMonomer> BiopolymerSequenceView::monomers() const { return m_monomers; }
QList<BioBond> BiopolymerSequenceView::bonds() const { return m_bonds; }
QString BiopolymerSequenceView::seqType() const { return m_seqType; }
