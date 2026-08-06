#include "TemplateLibrary.h"
#include "indigo.h"
#include <QRegularExpression>
#include <QFile>

TemplateLibrary::TemplateLibrary(const QString& fgSdfPath, const QString& librarySdfPath, const QString& saltsSdfPath) {
    m_session = indigoAllocSessionId();
    activateSession();
    loadSdf(fgSdfPath, m_fgStructs, m_fgNamesInOrder);
    loadSdf(librarySdfPath, m_libraryStructs, m_libraryNamesInOrder);
    loadSdf(saltsSdfPath, m_saltsStructs, m_saltsNamesInOrder);
    loadLibraryBondIdx(m_libraryStructs, m_libraryBondIdx);
    loadGroupMeta(m_fgStructs, m_fgGroups);
    loadGroupMeta(m_libraryStructs, m_libraryGroups);
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

void TemplateLibrary::loadSdf(const QString& path, QHash<QString, int>& target, QList<QString>& order) {
    static const QRegularExpression kGLine(QStringLiteral("^G\\s+\\d+\\s+\\d+\\s*$"),
                                            QRegularExpression::MultilineOption);

    activateSession();

    // Records are split MANUALLY on the "$$$$" delimiter line, rather than via
    // indigoIterateSDFile/indigoNext, because that Indigo API pair was found (sub-project
    // 14, while adding the enumeration accessors that were the first code to ever check
    // an SDF file's record count end-to-end) to silently DROP records for reasons internal
    // to Indigo's own SDF scanner: fg.sdf has 62 real records but indigoNext only ever
    // yielded 59 of them, permanently skipping "Bz", "CCl3", and "CF3" every single run.
    // Confirmed by direct isolation: extracting one of the dropped records' exact text
    // (blank-line-stripped, G-line intact) and feeding it straight to
    // indigoLoadMoleculeFromString parses it successfully -- the record itself is fine,
    // Indigo's iterator just never hands it to the caller. Splitting the file ourselves
    // sidesteps that scanner entirely and is immune to it. Previously invisible because
    // every prior caller only ever looked up a name from the front of a file (which the
    // buggy iterator does yield correctly).
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;   // missing/unreadable file: leave target empty
    const QString content = QString::fromUtf8(file.readAll());
    file.close();

    const QStringList rawRecords = content.split(QStringLiteral("$$$$"));
    for (const QString& rawRecord : rawRecords) {
        QString text = rawRecord;
        // Every record but the first is preceded by a blank line left over from the
        // previous record's "$$$$" delimiter (confirmed directly: fg.sdf line 25, right
        // after the first "$$$$" at line 24, is blank, before the second record's title
        // "Bn" on line 26). An MDL molfile's first 4 lines are fixed-position
        // (title/program-line/comment/counts), so this stray leading blank line shifts
        // everything by one and makes indigoLoadMoleculeFromString fail ("scanner:
        // BufferScanner::read() error") if left in place.
        while (text.startsWith(QLatin1Char('\n'))) {
            text.remove(0, 1);
        }
        if (text.trimmed().isEmpty()) continue;   // trailing empty chunk after the last "$$$$"
        text.remove(kGLine);
        int mol = indigoLoadMoleculeFromString(text.toUtf8().constData());
        if (mol < 0) continue;   // record failed to reparse: skipped, matching the real try/catch swallow
        const char* name = indigoName(mol);
        if (name && *name) {
            QString qname = QString::fromUtf8(name);
            if (!target.contains(qname)) {
                target.insert(qname, mol);
                order.append(qname);
            } else {
                indigoFree(mol);   // duplicate name in the file: keep the first
            }
        } else {
            indigoFree(mol);
        }
    }
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

void TemplateLibrary::loadGroupMeta(const QHash<QString, int>& structs, QHash<QString, QString>& target) {
    activateSession();
    for (auto it = structs.constBegin(); it != structs.constEnd(); ++it) {
        int mol = it.value();
        if (!indigoHasProperty(mol, "group")) continue;
        const char* raw = indigoGetProperty(mol, "group");
        if (!raw) continue;
        QString group = QString::fromUtf8(raw).trimmed();
        if (group.isEmpty()) continue;
        target.insert(it.key(), group);
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

QStringList TemplateLibrary::functionalGroupNames() const { return m_fgNamesInOrder; }
QStringList TemplateLibrary::libraryTemplateNames() const { return m_libraryNamesInOrder; }
QStringList TemplateLibrary::saltOrSolventNames() const { return m_saltsNamesInOrder; }

QString TemplateLibrary::functionalGroupGroup(const QString& name) const {
    return m_fgGroups.value(name, QStringLiteral("Functional Groups"));
}
QString TemplateLibrary::libraryTemplateGroup(const QString& name) const {
    return m_libraryGroups.value(name, QStringLiteral("Templates"));
}

QString TemplateLibrary::molfileText(int handle) const {
    if (handle < 0) return QString();
    activateSession();
    const char* mf = indigoMolfile(handle);
    return mf ? QString::fromUtf8(mf) : QString();
}
