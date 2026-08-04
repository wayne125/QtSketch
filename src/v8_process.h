#pragma once
#include <QObject>
#include <QVariantMap>
#include <QVariantList>
#include <QList>
#include <QByteArray>
#include <QtQml/qqml.h>
#include <memory>
#include "qjs_engine.h"
#include "app/molecule/DocumentState.h"

class V8Process : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(Sketch)

    Q_PROPERTY(QVariantMap primitives READ primitives NOTIFY primitivesChanged)
    Q_PROPERTY(QVariantMap selection READ selection NOTIFY selectionChanged)
    Q_PROPERTY(QVariantMap overlayState READ overlayState WRITE setOverlayState NOTIFY overlayStateChanged)

public:
    // cppEngine=true creates a document that routes the in-scope gestures (see
    // applyLocalState's own comment) through DocumentState instead of the JS engine, and never
    // creates m_engine at all. Defaulted so every existing call site (which never passes this
    // argument) is completely unaffected.
    explicit V8Process(QObject *parent = nullptr, bool cppEngine = false);
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
    Q_INVOKABLE void requestSelectionStructure(const QString& reqId);
    Q_INVOKABLE void requestSerialize(const QString& reqId);
    Q_INVOKABLE QString getStructure(const QString& fmt);
    Q_INVOKABLE QString serializeMol();
    Q_INVOKABLE void loadStructure(const QString& format, const QString& data, bool centerOnPage = false);
    Q_INVOKABLE void insertFunctionalGroup(const QString& fgName, double cx, double cy, int targetAtomId = -1, bool fullStructure = true);
    Q_INVOKABLE void insertLibraryTemplateFused(const QString& fgName, double cx, double cy, int targetBondId);
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
    Q_INVOKABLE void setAttachmentPoint(int id, int order);
    Q_INVOKABLE void changeAtomIsotope(int id, int isotope);
    Q_INVOKABLE void changeAtomRadical(int id, int radical);
    Q_INVOKABLE void changeAtomValence(int id, int valence);
    Q_INVOKABLE void requestAtomProperties(int id);
    Q_INVOKABLE void selectByRect(double x1, double y1, double x2, double y2);
    Q_INVOKABLE void addSelectionByRect(double x1, double y1, double x2, double y2);
    Q_INVOKABLE void selectByLasso(const QVariantList& pointsFlat);
    Q_INVOKABLE void selectItem(const QVariant& atomId, const QVariant& bondId, const QVariant& rxnArrowId = QVariant(), const QVariant& rxnPlusId = QVariant(), const QVariant& multitailArrowId = QVariant());
    Q_INVOKABLE void addItemToSelection(const QVariant& atomId, const QVariant& bondId);
    Q_INVOKABLE void removeItemFromSelection(const QVariant& atomId, const QVariant& bondId);
    Q_INVOKABLE void selectFragment(const QVariant& atomId, const QVariant& bondId);
    Q_INVOKABLE void moveSelection(double dx, double dy);
    Q_INVOKABLE void commitMove();
    Q_INVOKABLE void rotateSelectionLive(double angleDelta);
    Q_INVOKABLE void commitRotate();
    Q_INVOKABLE void scaleSelectionLive(double factor, double anchorX, double anchorY);
    Q_INVOKABLE void commitScale();
    Q_INVOKABLE void centerStructure();
    Q_INVOKABLE void normalizeStructure();

    Q_INVOKABLE void alignAtoms(const QString& direction);
    Q_INVOKABLE void distributeAtoms(const QString& direction);
    Q_INVOKABLE void transformSelection(const QString& mode);
    Q_INVOKABLE void addChain(double x1, double y1, double x2, double y2);
    Q_INVOKABLE void addText(const QString& content, double x, double y, bool bold = false, bool italic = false);
    Q_INVOKABLE void updateText(int id, const QString& content, bool bold = false, bool italic = false);
    Q_INVOKABLE void deleteText(int id);
    Q_INVOKABLE void addImage(const QString& base64DataUri, double cx, double cy, double halfW, double halfH);
    Q_INVOKABLE void deleteImage(int id);

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

private:
    // Replaces the old QProcess-based onReadyReadStandardOutput: called once per
    // JSON line the worker emits via console.log, whether that line came from
    // QjsEngine's synchronous native_log callback (see qjs_engine.cpp).
    void handleWorkerLine(const QString &line);

    // Turns m_docState's current molecule/selection into the exact QVariantMap shape and
    // signal set QML already consumes from the JS path -- the single place that makes a
    // C++-mode document indistinguishable from a JS-mode one at the QML boundary. Always
    // renders with showExplicitH=false; the setShowExplicitH toggle is out of scope for this
    // pilot (see the approved spec's Scope section).
    void applyLocalState();

    bool m_cppEngine = false;
    std::unique_ptr<DocumentState> m_docState;   // non-null only when m_cppEngine
    QString m_docClipboardMol;   // C++-engine-only clipboard cache: set by copySelection/cutSelection,
                                  // read by pasteSelection. Mirrors the JS worker's module-level
                                  // _clipboard (90-dispatch.js:133-141, 40-serialize.js:120-123).

    std::unique_ptr<QjsEngine> m_engine;
    QVariantMap m_primitives;
    QVariantMap m_selection;
    QVariantMap m_overlayState;
    double getLargestEmptyAngle(const QVariant& atomId, const QVariantMap& primitives);
};
