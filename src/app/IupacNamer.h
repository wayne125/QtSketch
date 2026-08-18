#ifndef IUPACNAMER_H
#define IUPACNAMER_H

#include <QString>
#include <QChar>
#include <map>
#include <utility>

struct IupacResult {
    bool success = false;
    QString name;
    QString error;
};

class IupacNamer {
public:
    // Generates IUPAC name for an already-loaded Indigo molecule handle.
    // Must be called within an active Indigo session.
    static IupacResult generateName(int mol);
};

// Maps each stereo-defined double bond (keyed by its two atom indices, min first)
// to its E/Z CIP descriptor, read directly from Indigo's KET JSON "cip" bond field.
// Shared between IupacNamer.cpp (name generation) and
// IndigoService::calcStereoDescriptors (UI bond-label display) -- see
// IUPAC Blue Book Coverage.md item 8. Must be called within an active Indigo session.
std::map<std::pair<int,int>, QChar> computeIndigoBondCIP(int mol);

#endif // IUPACNAMER_H
