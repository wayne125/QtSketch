#include "DocumentManager.h"

DocumentManager::DocumentManager(QObject *parent) : QObject(parent) {
    addDocument();
}

QVariantList DocumentManager::docIds() const {
    QVariantList out;
    for (int id : m_order) out.append(id);
    return out;
}

void DocumentManager::setActiveDocId(int docId) {
    if (m_activeDocId == docId || !m_documents.contains(docId)) return;
    m_activeDocId = docId;
    emit activeDocIdChanged();
}

int DocumentManager::addDocument(bool cppEngine) {
    int docId = m_nextDocId++;
    auto *proc = new V8Process(this, cppEngine);
    proc->init(); // no-op on a C++-mode document: sendCommand's own guard (m_engine is null) warns and returns
    m_documents.insert(docId, proc);
    m_order.append(docId);
    emit docIdsChanged();
    setActiveDocId(docId);
    return docId;
}

void DocumentManager::closeDocument(int docId) {
    if (!m_documents.contains(docId)) return;
    V8Process *proc = m_documents.take(docId);
    m_order.removeAll(docId);
    proc->deleteLater();
    emit docIdsChanged();
    if (m_activeDocId == docId) {
        m_activeDocId = -1;
        if (!m_order.isEmpty()) setActiveDocId(m_order.last());
        else emit activeDocIdChanged();
    }
}

QObject* DocumentManager::documentFor(int docId) const {
    return m_documents.value(docId, nullptr);
}
