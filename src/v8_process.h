#pragma once
#include <QObject>
#include <QProcess>
#include <QVariantMap>
#include <QVariantList>
#include <QList>
#include <QByteArray>
#include <QtQml/qqml.h>

class V8Process : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(Sketch)

    Q_PROPERTY(QVariantMap primitives READ primitives NOTIFY primitivesChanged)
    Q_PROPERTY(QVariantMap selection READ selection NOTIFY selectionChanged)
    Q_PROPERTY(QVariantMap overlayState READ overlayState WRITE setOverlayState NOTIFY overlayStateChanged)

public:
    explicit V8Process(QObject *parent = nullptr);
    ~V8Process();

    QVariantMap primitives() const { return m_primitives; }
    QVariantMap selection() const { return m_selection; }
    QVariantMap overlayState() const { return m_overlayState; }
    void setOverlayState(const QVariantMap& state);

    Q_INVOKABLE void init();
    Q_INVOKABLE void loadMol(const QString& molfile);
    Q_INVOKABLE void addAtom(const QString& label, double x, double y, int charge);
    Q_INVOKABLE void addBondAndAtom(int beginAtomId, const QString& endAtomLabel, double x, double y, int bondType, int stereo);
    Q_INVOKABLE void addBondBetweenCoords(double x1, double y1, double x2, double y2, int type, int stereo);
    Q_INVOKABLE void addBond(int beginAtomId, int endAtomId, int bondType, int stereoDir = 0);
    Q_INVOKABLE void addRing(const QVariantList& coords, bool aromatic = true);
    Q_INVOKABLE QVariantList getRingPreviewCoords(int n, double cx, double cy, const QVariant& hoverAtomId, const QVariant& hoverBondId);
    Q_INVOKABLE void deleteAtomById(int id);
    Q_INVOKABLE void deleteBondById(int id);
    Q_INVOKABLE void deleteSelection();
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    Q_INVOKABLE void copySelection();
    Q_INVOKABLE void cutSelection();
    Q_INVOKABLE void pasteSelection(double cx, double cy);
    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void clearCanvas();
    Q_INVOKABLE void loadBenzene();
    Q_INVOKABLE void deserializeMol(const QString& data);
    Q_INVOKABLE void requestStructure(const QString& fmt, const QString& reqId);
    Q_INVOKABLE void requestSerialize(const QString& reqId);
    Q_INVOKABLE QString getStructure(const QString& fmt);
    Q_INVOKABLE QString serializeMol();
    Q_INVOKABLE void loadStructure(const QString& format, const QString& data);
    Q_INVOKABLE void insertFunctionalGroup(const QString& fgName, double cx, double cy, int targetAtomId = -1);
    Q_INVOKABLE void requestSaltsAndSolventsList();
    Q_INVOKABLE void requestFunctionalGroupsList();
    Q_INVOKABLE void requestTemplateLibraryList();
    Q_INVOKABLE void requestTemplateThumbnail(const QString& name, const QString& reqId);
    Q_INVOKABLE void addRxnArrow(double x, double y, const QString& mode = QStringLiteral("filled-triangle"));
    Q_INVOKABLE void addRxnPlus(double x, double y);
    Q_INVOKABLE void addCurvedArrow(double x1, double y1, double ctrlX, double ctrlY, double x2, double y2);
    Q_INVOKABLE void setRxnArrowMode(int id, const QString& mode);
    Q_INVOKABLE void setRxnArrowConditions(int id, const QString& above, const QString& below);
    Q_INVOKABLE void setStereoFlags(const QString& type, int groupId);
    Q_INVOKABLE void addRGroup(int rgroupNumber);
    Q_INVOKABLE void deleteRGroup(int rgroupNumber);
    Q_INVOKABLE void setRGroupLogic(int rgroupNumber, const QString& range, bool resth, int ifthen);
    Q_INVOKABLE void addRGroupMember(int rgroupNumber);
    Q_INVOKABLE void removeRGroupMember(int rgroupNumber, int fragId);
    Q_INVOKABLE void setAtomQueryList(int atomId, const QString& elementsCsv, bool notList);
    Q_INVOKABLE void clearAtomQueryList(int atomId, const QString& fallbackLabel = QStringLiteral("C"));
    Q_INVOKABLE void addMultitailArrow(double x, double y);
    Q_INVOKABLE void deleteMultitailArrow(int id);
    Q_INVOKABLE void addMultitailArrowTail(int id);
    Q_INVOKABLE void changeAtomLabel(int id, const QString& label);
    Q_INVOKABLE void setAtomMapping(int id, int mapping);
    Q_INVOKABLE void changeBondType(int id, int type, int stereo = 0);
    Q_INVOKABLE void changeAtomCharge(int id, int charge);
    Q_INVOKABLE void changeAtomIsotope(int id, int isotope);
    Q_INVOKABLE void changeAtomRadical(int id, int radical);
    Q_INVOKABLE void changeAtomValence(int id, int valence);
    Q_INVOKABLE void requestAtomProperties(int id);
    Q_INVOKABLE void selectByRect(double x1, double y1, double x2, double y2);
    Q_INVOKABLE void addSelectionByRect(double x1, double y1, double x2, double y2);
    Q_INVOKABLE void selectItem(const QVariant& atomId, const QVariant& bondId, const QVariant& rxnArrowId = QVariant(), const QVariant& rxnPlusId = QVariant(), const QVariant& multitailArrowId = QVariant());
    Q_INVOKABLE void addItemToSelection(const QVariant& atomId, const QVariant& bondId);
    Q_INVOKABLE void removeItemFromSelection(const QVariant& atomId, const QVariant& bondId);
    Q_INVOKABLE void selectFragment(const QVariant& atomId, const QVariant& bondId);
    Q_INVOKABLE void moveSelection(double dx, double dy);
    Q_INVOKABLE void commitMove();
    Q_INVOKABLE void centerStructure();
    Q_INVOKABLE void normalizeStructure();

    Q_INVOKABLE void alignAtoms(const QString& direction);
    Q_INVOKABLE void distributeAtoms(const QString& direction);
    Q_INVOKABLE void transformSelection(const QString& mode);
    Q_INVOKABLE void addChain(double x1, double y1, double x2, double y2);
    Q_INVOKABLE void addText(const QString& content, double x, double y);
    Q_INVOKABLE void updateText(int id, const QString& content);
    Q_INVOKABLE void deleteText(int id);

    Q_INVOKABLE void setStereoDescriptors(const QString& jsonMap);

    Q_INVOKABLE void setCheckIssues(const QString& jsonMap);

    Q_INVOKABLE void sendCommand(const QString& cmd, const QVariantList& args = QVariantList());

    // OS clipboard helpers (synchronous, no IPC)
    Q_INVOKABLE QString getOsClipboardText() const;
    Q_INVOKABLE void setOsClipboardText(const QString& text);

    // Worker clipboard → OS clipboard
    Q_INVOKABLE void requestClipboardKet();
    // Import KET text as a paste at (cx, cy)
    Q_INVOKABLE void importKetAtPosition(const QString& ket, double cx, double cy);

signals:
    void stateUpdated(const QVariantMap& state, const QVariantMap& selection, bool isDirty, bool canUndo, bool canRedo, const QVariant& result);
    void primitivesChanged();
    void selectionChanged();
    void overlayStateChanged();
    void structureReady(const QString& reqId, const QString& data);
    void errorOccurred(const QString& error);

private slots:
    void onReadyReadStandardOutput();
    void onProcessError(QProcess::ProcessError error);
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessStarted();

private:
    QProcess *m_process;
    QList<QByteArray> m_pendingCommands;
    QVariantMap m_primitives;
    QVariantMap m_selection;
    QVariantMap m_overlayState;
    double getLargestEmptyAngle(const QVariant& atomId, const QVariantMap& primitives);
};
