// src/app/molecule/TemplateLibrary.cpp
#include "TemplateLibrary.h"
#include "indigo.h"
#include <QRegularExpression>

TemplateLibrary::TemplateLibrary(const QString& fgSdfPath, const QString& librarySdfPath, const QString& saltsSdfPath) {
    m_session = indigoAllocSessionId();
    activateSession();
    loadSdf(fgSdfPath, m_fgStructs);
    loadSdf(librarySdfPath, m_libraryStructs);
    loadSdf(saltsSdfPath, m_saltsStructs);
    loadLibraryBondIdx(m_libraryStructs, m_libraryBondIdx);
}

TemplateLibrary::~TemplateLibrary() {
    activateSession();
    for (int h : m_fgStructs) indigoFree(h);
    for (int h : m_libraryStructs) indigoFree(h);
    for (int h : m_saltsStructs) indigoFree(h);
    indigoReleaseSessionId(m_session);
}

void TemplateLibrary::activateSession() const {
    indigoSetSessionId(m_session);
}

void TemplateLibrary::loadSdf(const QString& path, QHash<QString, int>& target) {
    static const QRegularExpression kGLine(QStringLiteral("^G\\s+\\d+\\s+\\d+\\s*$"),
                                            QRegularExpression::MultilineOption);

    activateSession();
    int iter = indigoIterateSDFile(path.toUtf8().constData());
    if (iter < 0) return;   // missing/unreadable file: leave target empty

    int item;
    while ((item = indigoNext(iter)) > 0) {
        const char* raw = indigoRawData(item);
        if (raw) {
            QString text = QString::fromUtf8(raw);
            text.remove(kGLine);
            int mol = indigoLoadMoleculeFromString(text.toUtf8().constData());
            if (mol >= 0) {
                const char* name = indigoName(mol);
                if (name && *name) {
                    QString qname = QString::fromUtf8(name);
                    if (!target.contains(qname)) {
                        target.insert(qname, mol);
                    } else {
                        indigoFree(mol);   // duplicate name in the file: keep the first
                    }
                } else {
                    indigoFree(mol);
                }
            }
            // mol < 0: record failed to reparse even after the G-line strip --
            // skipped, matching the real try/catch swallow.
        }
        indigoFree(item);
    }
    indigoFree(iter);
}

void TemplateLibrary::loadLibraryBondIdx(const QHash<QString, int>& libraryStructs, QHash<QString, int>& target) {
    activateSession();
    for (auto it = libraryStructs.constBegin(); it != libraryStructs.constEnd(); ++it) {
        int mol = it.value();
        if (!indigoHasProperty(mol, "bondid")) continue;
        const char* raw = indigoGetProperty(mol, "bondid");
        if (!raw) continue;
        bool ok = false;
        int bondIdx = QString::fromUtf8(raw).toInt(&ok);
        if (!ok || bondIdx < 0 || bondIdx >= indigoCountBonds(mol)) continue;   // defensive validation
        target.insert(it.key(), bondIdx);
    }
}

int TemplateLibrary::functionalGroup(const QString& name) const {
    activateSession();
    return m_fgStructs.value(name, -1);
}
int TemplateLibrary::libraryTemplate(const QString& name) const {
    activateSession();
    return m_libraryStructs.value(name, -1);
}
int TemplateLibrary::saltOrSolvent(const QString& name) const {
    activateSession();
    return m_saltsStructs.value(name, -1);
}
int TemplateLibrary::libraryTemplateFusionBondIdx(const QString& name) const { return m_libraryBondIdx.value(name, -1); }

QString TemplateLibrary::molfileText(int handle) const {
    if (handle < 0) return QString();
    activateSession();
    const char* mf = indigoMolfile(handle);
    return mf ? QString::fromUtf8(mf) : QString();
}
