import sys

def process_file(file_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        c = f.read()

    t1 = """                } else if (!doubleO.empty() && !halogens.empty() && singleO.empty() && singleN.empty()) {
                    carbonGroup[i] = GroupType::ACYL_HALIDE;
                    acylHalideHalogen[i] = g.nodes[halogens[0]].atomicNumber;
                } else if (!doubleO.empty() && !singleN.empty()) {
                    bool isHydrazide = false;"""

    r1 = """                } else if (!doubleO.empty() && !halogens.empty() && singleO.empty() && singleN.empty()) {
                    if (halogens.size() > 1) {
                        return {false, "", "Carbonic acid halides with multiple halogens are not supported in this phase."};
                    }
                    int singleC = 0;
                    for (int nei : node.neighbors) {
                        if (g.nodes[nei].atomicNumber == 6) singleC++;
                    }
                    if (singleC == 0) {
                        return {false, "", "Carbonic/carbamic acid derivatives (rootless acyl carbons) are not supported in this phase."};
                    }
                    carbonGroup[i] = GroupType::ACYL_HALIDE;
                    acylHalideHalogen[i] = g.nodes[halogens[0]].atomicNumber;
                } else if (!doubleO.empty() && !singleN.empty()) {
                    int singleC = 0;
                    for (int nei : node.neighbors) {
                        if (g.nodes[nei].atomicNumber == 6) singleC++;
                    }
                    if (singleC == 0) {
                        return {false, "", "Carbonic/carbamic acid derivatives (rootless acyl carbons) are not supported in this phase."};
                    }
                    if (!halogens.empty()) {
                        return {false, "", "Amides with coexisting halogens on the acyl carbon are not supported."};
                    }
                    bool isHydrazide = false;"""

    t2 = """            } else if (!doubleO.empty() && !halogens.empty() && singleO.empty() && singleN.empty()) {
                carbonGroup[i] = GroupType::ACYL_HALIDE;
                acylHalideHalogen[i] = g.nodes[halogens[0]].atomicNumber;
            } else if (!doubleO.empty() && !singleN.empty()) {
                bool isHydrazide = false;"""

    r2 = """            } else if (!doubleO.empty() && !halogens.empty() && singleO.empty() && singleN.empty()) {
                if (halogens.size() > 1) {
                    return {false, "", "Carbonic acid halides with multiple halogens are not supported in this phase."};
                }
                int singleC = 0;
                for (int nei : node.neighbors) {
                    if (g.nodes[nei].atomicNumber == 6) singleC++;
                }
                if (singleC == 0) {
                    return {false, "", "Carbonic/carbamic acid derivatives (rootless acyl carbons) are not supported in this phase."};
                }
                carbonGroup[i] = GroupType::ACYL_HALIDE;
                acylHalideHalogen[i] = g.nodes[halogens[0]].atomicNumber;
            } else if (!doubleO.empty() && !singleN.empty()) {
                int singleC = 0;
                for (int nei : node.neighbors) {
                    if (g.nodes[nei].atomicNumber == 6) singleC++;
                }
                if (singleC == 0) {
                    return {false, "", "Carbonic/carbamic acid derivatives (rootless acyl carbons) are not supported in this phase."};
                }
                if (!halogens.empty()) {
                    return {false, "", "Amides with coexisting halogens on the acyl carbon are not supported."};
                }
                bool isHydrazide = false;"""

    print('t1 found:', c.count(t1))
    print('t2 found:', c.count(t2))

    c = c.replace(t1, r1)
    c = c.replace(t2, r2)
    with open(file_path, 'w', encoding='utf-8') as f:
        f.write(c)
    print("Done")

if __name__ == "__main__":
    process_file('src/app/IupacNamer.cpp')
