#ifndef INDIGOSERVICE_H
#define INDIGOSERVICE_H

#include <QObject>
#include <QString>
#include <QtQml/qqml.h>

class IndigoService : public QObject {
    Q_OBJECT
    QML_ELEMENT
public:
    explicit IndigoService(QObject *parent = nullptr);
    ~IndigoService();

    Q_INVOKABLE void layout(const QString &molfile);
    Q_INVOKABLE void aromatize(const QString &molfile);
    Q_INVOKABLE void dearomatize(const QString &molfile);
    Q_INVOKABLE void smiles(const QString &molfile);
    Q_INVOKABLE void canonicalSmiles(const QString &molfile);
    Q_INVOKABLE void normalize(const QString &molfile);
    Q_INVOKABLE void standardize(const QString &molfile);
    Q_INVOKABLE void calcProperties(const QString &molfile);
    Q_INVOKABLE void calcStereoDescriptors(const QString &molfile);
    Q_INVOKABLE void checkStructure(const QString &molfile);

    // Biopolymer loading (all emit biopolymerLoaded on success, biopolymerLoadError on failure)
    Q_INVOKABLE void loadBioSequence(const QString &text, const QString &seqType);
    Q_INVOKABLE void loadBioFasta(const QString &text, const QString &seqType);
    Q_INVOKABLE void loadBioHelm(const QString &text);
    Q_INVOKABLE void loadBioIdt(const QString &text);
    Q_INVOKABLE void loadBioAxoLabs(const QString &text);

    // Biopolymer export (from serialised molfile of current canvas)
    Q_INVOKABLE void exportBioSequence(const QString &molfile);
    Q_INVOKABLE void exportBioFasta(const QString &molfile);
    Q_INVOKABLE void exportBioHelm(const QString &molfile);
    Q_INVOKABLE void exportBioIdt(const QString &molfile);
    Q_INVOKABLE void exportBioAxoLabs(const QString &molfile);

Q_SIGNALS:
    void layoutFinished(const QString &result);
    void aromatizeFinished(const QString &result);
    void dearomatizeFinished(const QString &result);
    void smilesFinished(const QString &result);
    void canonicalSmilesFinished(const QString &result);
    void normalizeFinished(const QString &result);
    void standardizeFinished(const QString &result);
    void propertiesReady(double mw, double mono, const QString &mf,
                         int atoms, int bonds,
                         double tpsa, double logp, int hba, int hbd, int rotBonds);
    void stereoDescriptorsReady(const QString &jsonMap);
    void checkFinished(const QString &report);
    void checkIssuesReady(const QString &structuredJson);

    void biopolymerLoaded(const QString &molfile);
    void biopolymerLoadError(const QString &error);
    void bioSequenceReady(const QString &text);
    void bioFastaReady(const QString &text);
    void bioHelmReady(const QString &text);
    void bioIdtReady(const QString &text);
    void bioAxoLabsReady(const QString &text);

public:
    Q_INVOKABLE void copyToClipboard(const QString &text);
private:
    unsigned long long m_sessionId;
};

#endif // INDIGOSERVICE_H
