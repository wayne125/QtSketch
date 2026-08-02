// src/app/molecule/TemplateLibrary.h
#ifndef TEMPLATELIBRARY_H
#define TEMPLATELIBRARY_H

// Loads chem-core.js's three bundled SDF template files once (functional
// groups, library rings, salts/solvents) and exposes them by name -- the C++
// replacement for 30-templates.js's module-level _fgStructs/_libraryStructs/
// _saltsStructs + the <bondid> half of _libraryMeta (sub-project 3d of the
// chem-core.js migration; see
// docs/superpowers/specs/2026-08-02-template-insertion-cpp-design.md).
//
// SCOPE CORRECTION (not ported): the real _fgMeta/_libraryMeta also parse a
// <group> field (consumed only by 90-dispatch.js's picker-category-listing,
// dispatch plumbing out of scope here) and an <atomid> field (parsed and
// validated by the real code but never actually read by insertFunctionalGroup
// or insertLibraryTemplateFused anywhere -- dead data even in the source).
// Only <bondid>, the one field insertLibraryTemplateFused actually uses, is
// parsed here.
//
// LOADING RECIPE (empirically confirmed against the real indigo.dll and the
// real bundled files, not assumed): indigoIterateSDFile's indigoNext yields
// an item that is NOT a usable molecule handle (indigoCountAtoms returns -1
// on it) -- it must be reparsed via indigoRawData + indigoLoadMoleculeFromString.
// Every one of fg.sdf's 62 records (confirmed: 62/62, and 0/276 library.sdf,
// 0/135 salts-and-solvents.sdf) carries a Ketcher-specific "G    1  0" line
// positioned right after the bond block; indigoLoadMoleculeFromString fails
// outright on it ("array: invalid index 0 (size=0)", confirmed by direct
// reparse of the real "Ac" record with and without that one line -- removing
// it alone was the fix). No truncation at "M  END" is needed: trailing SDF
// ">  <field>" data-item blocks parse fine as-is once the G-line is gone.
// indigoGetProperty(mol, "bondid"), called on the successfully-REPARSED
// molecule (calling it on the raw pre-parse item returns a bogus value,
// confirmed by direct probe), directly returns the real <bondid> value as a
// string -- no custom text-scanning parser is needed for this field.

#include <QString>
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

    void activateSession() const;
    void loadSdf(const QString& path, QHash<QString, int>& target);
    void loadLibraryBondIdx(const QHash<QString, int>& libraryStructs, QHash<QString, int>& target);
};

#endif // TEMPLATELIBRARY_H
