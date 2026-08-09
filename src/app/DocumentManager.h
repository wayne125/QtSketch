#pragma once
#include <QObject>
#include <QMap>
#include <QList>
#include <QVariantList>
#include <QtQml/qqml.h>
#include <memory>
#include "../v8_process.h"
#include "app/molecule/TemplateLibrary.h"

class DocumentManager : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(DocumentManager)
    QML_SINGLETON

    Q_PROPERTY(QVariantList docIds READ docIds NOTIFY docIdsChanged)
    Q_PROPERTY(int activeDocId READ activeDocId WRITE setActiveDocId NOTIFY activeDocIdChanged)

public:
    explicit DocumentManager(QObject *parent = nullptr);

    QVariantList docIds() const;
    int activeDocId() const { return m_activeDocId; }
    void setActiveDocId(int docId);

    Q_INVOKABLE int addDocument();
    Q_INVOKABLE void closeDocument(int docId);
    Q_INVOKABLE QObject* documentFor(int docId) const;

signals:
    void docIdsChanged();
    void activeDocIdChanged();

private:
    QMap<int, V8Process*> m_documents;
    QList<int> m_order;
    int m_nextDocId = 1;
    int m_activeDocId = -1;

    // Lazily constructed the first time a C++-engine document is created (addDocument's
    // own body below) -- a JS-only session never pays the cost of parsing ~473 SDF
    // records through Indigo. Shared across every C++-engine document/tab rather than
    // one per document, since TemplateLibrary(const TemplateLibrary&) is deleted (not
    // copyable) and reparsing the same 3 files per tab would be wasteful. Never
    // constructed eagerly, never destroyed early -- lives for the process lifetime once
    // it exists.
    std::unique_ptr<TemplateLibrary> m_templateLibrary;
};
