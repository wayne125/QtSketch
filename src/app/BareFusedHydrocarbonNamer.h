#pragma once

#include <QString>
#include "IupacNamer.h"
#include "GraphTypes.h"
#include "FusedRingOrientation.h"

enum class BareFusedDisposition { NotMatched, Named, Rejected };

struct BareFusedHydrocarbonResult {
    BareFusedDisposition disposition = BareFusedDisposition::NotMatched;
    IupacResult result;   // valid when Named
    QString message;      // valid when Rejected
};

BareFusedHydrocarbonResult tryNameBareFusedHydrocarbon(
    int mol, const Graph &g, const FusedRingSystemInput &input);
