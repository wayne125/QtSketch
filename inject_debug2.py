import re

with open('src/app/IupacNamer.cpp', 'r') as f:
    content = f.read()

detect_old = """                    bool isHydrazide = false;
                    for (int sN : singleN) {
                        for (size_t k = 0; k < g.nodes[sN].neighbors.size(); ++k) {
                            int nei = g.nodes[sN].neighbors[k];
                            if (nei != static_cast<int>(i) && g.nodes[nei].atomicNumber == 7 && g.nodes[sN].bondOrders[k] == 1) {
                                if (g.nodes[nei].neighbors.size() == 1) {
                                    isHydrazide = true;
                                    break;
                                }
                            }
                        }
                        if (isHydrazide) break;
                    }"""

detect_new = """                    bool isHydrazide = false;
                    for (int sN : singleN) {
                        for (size_t k = 0; k < g.nodes[sN].neighbors.size(); ++k) {
                            int nei = g.nodes[sN].neighbors[k];
                            if (nei != static_cast<int>(i) && g.nodes[nei].atomicNumber == 7 && g.nodes[sN].bondOrders[k] == 1) {
                                std::cout << "N1 has N2 neighbor with size: " << g.nodes[nei].neighbors.size() << "\n";
                                if (g.nodes[nei].neighbors.size() == 1) {
                                    isHydrazide = true;
                                    break;
                                }
                            }
                        }
                        if (isHydrazide) break;
                    }"""

content = content.replace(detect_old, detect_new)

with open('src/app/IupacNamer.cpp', 'w') as f:
    f.write(content)
