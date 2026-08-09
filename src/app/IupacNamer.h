#ifndef IUPACNAMER_H
#define IUPACNAMER_H

#include <QString>

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

#endif // IUPACNAMER_H
