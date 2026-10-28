#ifndef BONDSTEREOPERCEPTION_H
#define BONDSTEREOPERCEPTION_H

#include <QChar>
#include <map>
#include <utility>

// Maps each stereo-defined double bond (keyed by its two atom indices, min first)
// to its E/Z CIP descriptor, read directly from Indigo's KET JSON "cip" bond field.
// Shared between IupacNamer.cpp (name generation), IndigoService::calcStereoDescriptors
// (UI bond-label display), and EditableMolecule::allBondCipLabels (canvas rendering) --
// see IUPAC Blue Book Coverage.md item 8. Must be called within an active Indigo session.
//
// Lives in its own small translation unit (mirroring BondAngleSuggester.h/.cpp's
// pattern) rather than in IupacNamer.cpp, specifically so molecule_model (which
// EditableMolecule.cpp is part of) can link it without also pulling in the whole
// naming engine -- IupacNamer.cpp is separately compiled directly into both sketch
// and iupac_namer_test, so adding it to molecule_model's own sources would produce
// duplicate-symbol link errors for sketch.exe (which links both).
std::map<std::pair<int,int>, QChar> computeIndigoBondCIP(int mol);

#endif // BONDSTEREOPERCEPTION_H
