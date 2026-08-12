#ifndef INDIGOSERVICE_H
#define INDIGOSERVICE_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QtQml/qqml.h>

class IndigoService : public QObject {
    Q_OBJECT
    QML_ELEMENT
public:
    explicit IndigoService(QObject *parent = nullptr);
    ~IndigoService();

    Q_INVOKABLE void layout(const QString &molfile);
    Q_INVOKABLE void clean2d(const QString &molfile);
    Q_INVOKABLE void aromatize(const QString &molfile);
    Q_INVOKABLE void dearomatize(const QString &molfile);
    Q_INVOKABLE void unfoldHydrogens(const QString &molfile);
    Q_INVOKABLE void foldHydrogens(const QString &molfile);
    Q_INVOKABLE void smiles(const QString &molfile);
    Q_INVOKABLE void canonicalSmiles(const QString &molfile);
    Q_INVOKABLE void inchi(const QString &molfile);
    Q_INVOKABLE void inchiKey(const QString &molfile);
    Q_INVOKABLE void hash(const QString &molfile);
    Q_INVOKABLE void similarity(const QString &molfile, const QString &refSmiles);
    Q_INVOKABLE void massComposition(const QString &molfile);
    Q_INVOKABLE void pkaValues(const QString &molfile);
    Q_INVOKABLE void renderToFile(const QString &molfile, const QUrl &fileUrl, const QString &format);
    Q_INVOKABLE void renderReactionGridToFile(const QString &molfile, const QUrl &fileUrl, const QString &format);
    Q_INVOKABLE void exportBatchGridToFile(const QStringList &molfiles, const QUrl &fileUrl, const QString &format);
    Q_INVOKABLE void parseRdfBatch(const QUrl &fileUrl);
    Q_INVOKABLE void parseIndigoBatchFile(const QUrl &fileUrl, const QString &format);
    Q_INVOKABLE void exportBatchToFile(const QStringList &molfiles, const QUrl &fileUrl, const QString &format);
    Q_INVOKABLE void autoMapReaction(const QString &molfile);
    Q_INVOKABLE void clearReactionMapping(const QString &molfile);
    Q_INVOKABLE void correctReactingCenters(const QString &molfile);
    Q_INVOKABLE void normalize(const QString &molfile);
    Q_INVOKABLE void standardize(const QString &molfile);
    Q_INVOKABLE void ionizeAtPh(const QString &molfile, double pH);
    Q_INVOKABLE void calcProperties(const QString &molfile);
    Q_INVOKABLE void calcStereoDescriptors(const QString &molfile);
    Q_INVOKABLE void checkStructure(const QString &molfile);
    Q_INVOKABLE void generateIupacName(const QString &molfile);
    Q_INVOKABLE void substructureSearch(const QString &molfile, const QString &smarts);
    Q_INVOKABLE void findCommonScaffold(const QStringList &molfiles);
    Q_INVOKABLE void decomposeToRGroups(const QStringList &molfiles);
    Q_INVOKABLE void decomposeToRGroupsPerMolecule(const QStringList &molfiles, const QStringList &labels);
    Q_INVOKABLE void rankBySimilarity(const QString &refMolfile, const QStringList &molfiles);
    Q_INVOKABLE void alignBatchToScaffold(const QStringList &molfiles);

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
    void layoutFinished(const QString &result, const QString &error);
    void reactionMappingFinished(const QString &result, const QString &error);
    void commonScaffoldFinished(const QString &result, const QString &error);
    void rgroupDecompositionFinished(const QString &result, const QString &error);
    void rgroupPerMoleculeDecompositionFinished(const QString &resultsJson, const QString &error);
    void similarityRankFinished(const QString &resultJson, const QString &error);
    void batchAlignFinished(const QString &resultJson, const QString &error);
    void rdfBatchParsed(const QString &recordsJson, const QString &error);
    void indigoBatchParsed(const QString &recordsJson, const QString &error);
    void clean2dFinished(const QString &result);
    void aromatizeFinished(const QString &result);
    void dearomatizeFinished(const QString &result);
    void unfoldHydrogensFinished(const QString &result);
    void foldHydrogensFinished(const QString &result);
    void smilesFinished(const QString &result);
    void canonicalSmilesFinished(const QString &result);
    void inchiFinished(const QString &result);
    void inchiKeyFinished(const QString &result);
    void hashFinished(const QString &result);
    void similarityFinished(const QString &result);
    void massCompositionFinished(const QString &result);
    void pkaValuesFinished(const QString &result);
    void renderFinished(bool success, const QString &error);
    void normalizeFinished(const QString &result);
    void standardizeFinished(const QString &result);
    void ionizeFinished(const QString &result, const QString &error);
    void propertiesReady(double mw, double mono, const QString &mf,
                         int atoms, int bonds,
                         double tpsa, double logp, int hba, int hbd, int rotBonds,
                         double molarRefractivity, double pka,
                         int heavyAtoms, bool isChiral, double mostAbundantMass,
                         int fragmentCount, int ringCount);
    void stereoDescriptorsReady(const QString &jsonMap);
    void checkFinished(const QString &report);
    void checkIssuesReady(const QString &structuredJson);
    void substructureSearchFinished(const QString &result);
    void iupacNameReady(const QString &name, const QString &error);

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
