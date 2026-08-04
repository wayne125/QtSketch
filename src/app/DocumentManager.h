#pragma once
#include <QObject>
#include <QMap>
#include <QList>
#include <QVariantList>
#include <QtQml/qqml.h>
#include "../v8_process.h"

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

    Q_INVOKABLE int addDocument(bool cppEngine = false);
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
};
