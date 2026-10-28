#pragma once

#include "GraphTypes.h"
#include "FusedRingOrientation.h"
#include "NamingTrace.h"

#include <map>
#include <set>
#include <vector>
#include <QString>

std::map<int, QString> computePeripheralNumberingGeneral(
    const Graph &g,
    const FusedRingSystemInput &input,
    const std::set<int> &substituentBearingNodes = {},
    const std::set<int> &hydroLocantBearingNodes = {},
    std::vector<std::map<int, QString>> *tiedMapsOut = nullptr,
    bool restrictStartToPredecessorBridgehead = false,
    const std::set<int> &principalGroupBearingNodes = {});
