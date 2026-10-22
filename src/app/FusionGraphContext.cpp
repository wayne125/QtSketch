#include "FusionGraphContext.h"
#include "indigo.h"

FusionGraphContext::FusionGraphContext(const Graph &g, int mol,
                                       const std::vector<std::set<int>> *ringSubsetOverride)
    : m_graph(g), m_allDisjoint(true)
{
    if (ringSubsetOverride) {
        m_sssrRings = *ringSubsetOverride;
        m_allDisjoint = true;
        for (size_t i = 0; i < m_sssrRings.size() && m_allDisjoint; ++i) {
            for (size_t j = i + 1; j < m_sssrRings.size(); ++j) {
                for (int a : m_sssrRings[i]) {
                    if (m_sssrRings[j].count(a)) {
                        m_allDisjoint = false;
                        break;
                    }
                }
                if (!m_allDisjoint) break;
            }
        }
    } else {
        int sssrIter = indigoIterateSSSR(mol);
        m_allDisjoint = true;
        if (sssrIter >= 0) {
            int subMol = 0;
            while ((subMol = indigoNext(sssrIter)) != 0) {
                std::set<int> rAtoms;
                int ringAtomIter = indigoIterateAtoms(subMol);
                if (ringAtomIter >= 0) {
                    int ringAtomHandle = 0;
                    while ((ringAtomHandle = indigoNext(ringAtomIter)) != 0) {
                        rAtoms.insert(indigoIndex(ringAtomHandle));
                        indigoFree(ringAtomHandle);
                    }
                    indigoFree(ringAtomIter);
                }
                m_sssrRings.push_back(rAtoms);
                indigoFree(subMol);
            }
            indigoFree(sssrIter);
            for (size_t i = 0; i < m_sssrRings.size() && m_allDisjoint; ++i) {
                for (size_t j = i + 1; j < m_sssrRings.size(); ++j) {
                    for (int a : m_sssrRings[i]) {
                        if (m_sssrRings[j].count(a)) {
                            m_allDisjoint = false;
                            break;
                        }
                    }
                    if (!m_allDisjoint) break;
                }
            }
        } else {
            m_allDisjoint = false;
        }
    }
}

const Graph &FusionGraphContext::graph() const { return m_graph; }

const std::vector<std::set<int>> &FusionGraphContext::sssrRings() const { return m_sssrRings; }

bool FusionGraphContext::allDisjoint() const { return m_allDisjoint; }
