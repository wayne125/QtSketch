#ifndef IUPACNAMER_H
#define IUPACNAMER_H

#include <QString>

// Naming result classification (Task C)
// SystematicIupacName as the default for a success path means
// "generated systematically, not independently verified" -- not a claim
// of correctness beyond what the existing tests already check.
enum class NamingStatus {
    Failed,
    KnownGapFallback,
    SystematicIupacName,
    PreferredIupacName,
};

struct IupacResult {
    bool success = false;
    QString name;
    QString error;
    NamingStatus status = NamingStatus::Failed;
};

class IupacNamer {
public:
    // Generates IUPAC name for an already-loaded Indigo molecule handle.
    // Must be called within an active Indigo session.
    static IupacResult generateName(int mol);
private:
    // Implementation - renamed from generateName to support status normalization wrapper
    static IupacResult generateNameImpl(int mol);
};

#endif // IUPACNAMER_H
