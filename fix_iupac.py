import re

with open("src/app/IupacNamer.cpp", "r") as f:
    content = f.read()

replacement = """        } else {
            // Check for partially saturated hydro-prefix case
            std::vector<int> spareValence(ringSize);
            int zeroSpareCount = 0;
            for (int i = 0; i < ringSize; ++i) {
                int z = g.nodes[bestSig.ringChain[i]].atomicNumber;
                if (z == 8 || z == 16 || z == 34 || z == 52) {
                    spareValence[i] = 0;
                    zeroSpareCount++;
                } else {
                    spareValence[i] = 1;
                }
            }
            bool parityCheckOk = true;
            if (zeroSpareCount > 0) {
                int startIdx = 0;
                while (spareValence[startIdx] != 0) startIdx++;
                int runLength = 0;
                for (int i = 1; i <= ringSize; ++i) {
                    int idx = (startIdx + i) % ringSize;
                    if (spareValence[idx] == 1) {
                        runLength++;
                    } else {
                        if (runLength > 0 && runLength % 2 != 0) { parityCheckOk = false; break; }
                        runLength = 0;
                    }
                }
            } else {
                if (ringSize % 2 != 0) parityCheckOk = false;
            }

            std::vector<int> hydroLocs;
            if (parityCheckOk) {
                for (int i = 0; i < ringSize; ++i) {
                    if (spareValence[i] == 1) {
                        int nodeIdx = bestSig.ringChain[i];
                        const GraphNode &node = g.nodes[nodeIdx];
                        int prevIdx = bestSig.ringChain[(i - 1 + ringSize) % ringSize];
                        int nextIdx = bestSig.ringChain[(i + 1) % ringSize];
                        int orderToPrev = -1, orderToNext = -1;
                        for (size_t j = 0; j < node.neighbors.size(); ++j) {
                            if (node.neighbors[j] == prevIdx) orderToPrev = node.bondOrders[j];
                            else if (node.neighbors[j] == nextIdx) orderToNext = node.bondOrders[j];
                        }
                        if (orderToPrev == 1 && orderToNext == 1) {
                            hydroLocs.push_back(i + 1);
                        }
                    }
                }
            }

            if (parityCheckOk && hydroLocs.size() > 0 && hydroLocs.size() % 2 == 0) {
                QStringList hStrs;
                for (int l : hydroLocs) hStrs.append(QString::number(l));
                QString hydroPrefix = QString("%1-%2hydro-").arg(hStrs.join(","), multiPrefix(static_cast<int>(hydroLocs.size())));
                parentNameRoot = hydroPrefix + locantPrefix + elemPrefixes + stem;
            } else {
                // P-14.7.1: Check for indicated hydrogen (saturated ring position)
                int indicatedH = findIndicatedHydrogenLocant(g, bestSig.ringChain);
                if (indicatedH == -2) {
                    // Multiple indicated hydrogen positions - not supported for general heterocycles
                    return {false, "", "Multiple indicated hydrogen positions found; this general heterocycle case is not yet supported."};
                } else if (indicatedH > 0) {
                    // Exactly one indicated hydrogen position - prepend "<locant>H-"
                    parentNameRoot = QString("%1H-").arg(indicatedH) + locantPrefix + elemPrefixes + stem;
                } else {
                    // No indicated hydrogen needed
                    parentNameRoot = locantPrefix + elemPrefixes + stem;
                }
            }
        }"""

target = """        } else {
        // P-14.7.1: Check for indicated hydrogen (saturated ring position)
        int indicatedH = findIndicatedHydrogenLocant(g, bestSig.ringChain);
        if (indicatedH == -2) {
            // Multiple indicated hydrogen positions - not supported for general heterocycles
            return {false, "", "Multiple indicated hydrogen positions found; this general heterocycle case is not yet supported."};
        } else if (indicatedH > 0) {
            // Exactly one indicated hydrogen position - prepend "<locant>H-"
            parentNameRoot = QString("%1H-").arg(indicatedH) + locantPrefix + elemPrefixes + stem;
        } else {
            // No indicated hydrogen needed
            parentNameRoot = locantPrefix + elemPrefixes + stem;
        }
        }"""

if target in content:
    content = content.replace(target, replacement)
    with open("src/app/IupacNamer.cpp", "w") as f:
        f.write(content)
    print("Replaced successfully")
else:
    print("Target not found")
