#ifndef TEMPLATELIBRARY_H
#define TEMPLATELIBRARY_H

// Loads chem-core.js's three bundled SDF template files once (functional
// groups, library rings, salts/solvents) and exposes them by name -- the C++
// replacement for 30-templates.js's module-level _fgStructs/_libraryStructs/
// _saltsStructs + the <bondid>/<group> halves of _fgMeta/_libraryMeta
// (sub-project 3d of the chem-core.js migration; see
// docs/superpowers/specs/2026-08-02-template-insertion-cpp-design.md).
//
// SCOPE CORRECTION (not ported): the real _fgMeta/_libraryMeta also parse an
// <atomid> field (parsed and validated by the real code but never actually
// read by insertFunctionalGroup or insertLibraryTemplateFused anywhere --
// dead data even in the source) -- not parsed here.
//
// LOADING RECIPE (empirically confirmed against the real indigo.dll and the
// real bundled files, not assumed): indigoIterateSDFile's indigoNext yields
// an item that is NOT a usable molecule handle (indigoCountAtoms returns -1
// on it) -- it must be reparsed via indigoRawData + indigoLoadMoleculeFromString.
// Every one of fg.sdf's 62 records carries a Ketcher-specific "G    1  0"
// line positioned right after the bond block; indigoLoadMoleculeFromString
// fails outright on it ("array: invalid index 0 (size=0)", confirmed by
// direct reparse of the real "Ac" record with and without that one line --
// removing it alone was the fix). No truncation at "M  END" is needed:
// trailing SDF ">  <field>" data-item blocks parse fine as-is once the
// G-line is gone. Separately (found by sub-project 14, when its new
// enumeration accessors were the first callers to ever touch a record past
// the first one in any of these files): every record but the first also
// carries a stray leading blank line, left over from the previous record's
// "$$$$" delimiter line (confirmed directly in fg.sdf -- line 25, right
// after the first "$$$$" at line 24, is blank, before the second record's
// title "Bn" on line 26). Since an MDL molfile's first 4 lines are
// fixed-position (title/program-line/comment/counts), this shifts
// everything by one line and made indigoLoadMoleculeFromString fail
// ("scanner: BufferScanner::read() error") on every record except the
// first for as long as this class has existed -- invisible until now
// because every prior caller only ever looked up a first-record name ("Ac",
// "alpha-D-Allopyranose"). Stripped by loadSdf before the G-line removal.
// indigoGetProperty(mol, "bondid"), called on the
// successfully-REPARSED molecule (calling it on the raw pre-parse item
// returns a bogus value, confirmed by direct probe), directly returns the
// real <bondid> value as a string -- no custom text-scanning parser is
// needed for this field. The same mechanism (indigoGetProperty on the
// reparsed handle) is reused for <group> (sub-project 14).

#include <QString>
#include <QStringList>
#include <QHash>

class TemplateLibrary {
public:
    TemplateLibrary(const QString& fgSdfPath, const QString& librarySdfPath, const QString& saltsSdfPath);
    ~TemplateLibrary();

    TemplateLibrary(const TemplateLibrary&) = delete;
    TemplateLibrary& operator=(const TemplateLibrary&) = delete;

    int functionalGroup(const QString& name) const;   // Indigo molecule handle, -1 if absent
    int libraryTemplate(const QString& name) const;
    int saltOrSolvent(const QString& name) const;
    int libraryTemplateFusionBondIdx(const QString& name) const;   // -1 if the template has no fusion metadata

    // Enumeration accessors for the picker-listing commands (sub-project 14). Each
    // returns names in FILE INSERTION ORDER (the order records appear in the SDF file) --
    // QHash does not preserve insertion order, so a separate ordered list is tracked
    // alongside each hash specifically for this. Sorting (where the real JS does it, e.g.
    // getFunctionalGroupsList) is the CALLER's job, not this class's -- matches the real
    // 90-dispatch.js/30-templates.js layering exactly (this class only ever stores, never
    // sorts).
    QStringList functionalGroupNames() const;
    QStringList libraryTemplateNames() const;
    QStringList saltOrSolventNames() const;

    // <group> SDF metadata, parsed the same way <bondid> already is (loadLibraryBondIdx,
    // below). Real JS's own fallback values (30-templates.js's '' + the dispatch-side
    // `||` default) are baked directly into these two accessors for when no <group> was
    // found for that name. No salts equivalent: the real getSaltsAndSolventsList never
    // reads or emits a group field at all, and the real parseGroupMeta is never called on
    // salts-and-solvents.sdf either -- do not add one.
    QString functionalGroupGroup(const QString& name) const;
    QString libraryTemplateGroup(const QString& name) const;

    // Serializes a handle returned by this class (from ITS OWN session) to
    // Molfile text -- the only safe way for a caller in a DIFFERENT Indigo
    // session (e.g. an EditableMolecule, which activates its own session
    // before every call) to consume this structure. Confirmed by direct
    // probe: a raw handle cannot cross sessions -- reusing its integer id in
    // a foreign session either fails or, worse, silently aliases a different
    // object there -- while a Molfile round-trip (indigoMolfile here,
    // indigoLoadMoleculeFromString on the other side) preserves atoms,
    // bonds, coordinates, and SUP sgroups with their attachment points
    // correctly. Empty string if the handle is invalid.
    QString molfileText(int handle) const;

private:
    // Indigo molecule handles are small integers scoped to the session active
    // when they were created -- switching sessions (as every EditableMolecule
    // does before its own indigo calls) can make the SAME integer alias a
    // completely different object in the new session, silently corrupting
    // data instead of failing loudly (confirmed by direct probe). Every
    // stored handle here belongs to m_session; activateSession() must run
    // before any indigo call that touches m_session's state, mirroring
    // EditableMolecule's own activateSession() pattern.
    unsigned long long m_session = 0;
    QHash<QString, int> m_fgStructs;
    QHash<QString, int> m_libraryStructs;
    QHash<QString, int> m_saltsStructs;
    QHash<QString, int> m_libraryBondIdx;

    // Insertion-order companions to the 3 QHashes above (sub-project 14) -- QHash
    // iteration order is hash-bucket order, unrelated to SDF-file record order, so these
    // are needed to expose the real file order via functionalGroupNames() etc.
    QList<QString> m_fgNamesInOrder;
    QList<QString> m_libraryNamesInOrder;
    QList<QString> m_saltsNamesInOrder;

    // <group> metadata, keyed the same way m_libraryBondIdx is (sub-project 14).
    QHash<QString, QString> m_fgGroups;
    QHash<QString, QString> m_libraryGroups;

    void activateSession() const;
    void loadSdf(const QString& path, QHash<QString, int>& target, QList<QString>& order);
    void loadLibraryBondIdx(const QHash<QString, int>& libraryStructs, QHash<QString, int>& target);
    void loadGroupMeta(const QHash<QString, int>& structs, QHash<QString, QString>& target);
};

#endif // TEMPLATELIBRARY_H
