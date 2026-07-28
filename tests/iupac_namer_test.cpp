#include "IupacNamer.h"
#include "indigo.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <map>
#include <set>

struct GraphNode {
    int id;
    int indigoIdx;
    int atomicNumber;
    int totalH;
    std::vector<int> neighbors;
    std::vector<int> bondOrders;
};

struct GraphBond {
    int u, v;
    int order;
};

struct Graph {
    std::vector<GraphNode> nodes;
    std::vector<GraphBond> bonds;
};

std::map<int, QString> computePeripheralNumbering(
    const Graph &g,
    const std::set<int> &ring1Nodes,
    const std::set<int> &ring2Nodes,
    int bhA,
    int bhB,
    const std::set<int> &substituentBearingNodes = {},
    bool preferIndicatedHydrogenLocant = false);

std::map<int, QString> computePeripheralNumberingForMol(int mol);

struct TestCase {
    std::string smiles;
    std::string expectedName;
    bool shouldFail = false;
    std::string expectedErrorSubstring = "";
};

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);
    qword sid = indigoAllocSessionId();
    indigoSetSessionId(sid);

    std::vector<TestCase> tests = {
        // Straight-chain alkanes C1-C10
        {"C", "methane"},
        {"CC", "ethane"},
        {"CCC", "propane"},
        {"CCCC", "butane"},
        {"CCCCC", "pentane"},
        {"CCCCCC", "hexane"},
        {"CCCCCCC", "heptane"},
        {"CCCCCCCC", "octane"},
        {"CCCCCCCCC", "nonane"},
        {"CCCCCCCCCC", "decane"},

        // Branched alkane
        {"CC(C)C", "2-methylpropane"},

        // Alkene and Alkyne
        {"C=C", "ethene"},
        {"C#C", "ethyne"},
        {"CC=C", "prop-1-ene"},

        // 7 suffix classes individually
        {"CC(=O)O", "ethanoic acid"},
        {"CC(=O)N", "ethanamide"},
        {"CC#N", "ethanenitrile"},
        {"CC=O", "ethanal"},
        {"CC(=O)C", "propan-2-one"},
        {"CCO", "ethanol"},
        {"CCN", "ethanamine"},

        // Seniority / multi-group split
        {"CC(O)C(=O)O", "2-hydroxypropanoic acid"},
        {"NCCO", "2-aminoethanol"},

        // Halogen substituent
        {"CC(Cl)C", "2-chloropropane"},

        // Diacid (multiplying prefix suffix) -- HOOC-(CH2)4-COOH, 6 carbons total
        {"O=C(O)CCCCC(=O)O", "hexanedioic acid"},

        // Diamine (multiplying-prefix suffix on a non-acid class -- regression check for a
        // format-string bug that previously produced "didiamine" instead of "diamine")
        {"NCCN", "ethane-1,2-diamine"},

        // Longer branched substituent (3-methylpentane) -- exercises multi-carbon substituent
        // naming, not just a bare "methyl"
        {"CCC(C)CC", "3-methylpentane"},

        // Two different halogens on the same chain
        {"CC(F)C(Cl)C", "2-chloro-3-fluorobutane"},

        // Monocyclic ring Phase 2 success cases
        {"c1ccccc1", "benzene"},
        {"C1CCCCC1", "cyclohexane"},
        {"C1CCCCC1O", "cyclohexanol"},
        {"C1CCCCC1C(=O)O", "cyclohexanecarboxylic acid"},
        {"Cc1ccccc1", "methylbenzene"},
        {"Clc1ccccc1", "chlorobenzene"},
        {"C1=CCCCC1", "cyclohexene"},
        {"c1ccoc1", "furan"},

        // Remaining 3 heterocycles (only furan was in the shipped suite)
        {"c1ccsc1", "thiophene"},
        {"c1cc[nH]c1", "pyrrole"},
        {"c1ccncc1", "pyridine"},

        // Other ring sizes (only cyclohexane/-ol/-ene/-carboxylic acid were tested at size 6)
        {"C1CCC1", "cyclobutane"},
        {"C1CC1", "cyclopropane"},

        // Ketone and amine directly on a ring carbon (only alcohol and the exocyclic acid
        // suffix were tested; ketone/amine use the same on-ring suffix branch as alcohol)
        {"O=C1CCCCC1", "cyclohexanone"},
        {"NC1CCCCC1", "cyclohexanamine"},

        // Halogen substituent on a saturated ring (only benzene got a halogen test)
        {"ClC1CCCCC1", "chlorocyclohexane"},

        // Phase 3: Ortho-fused bicyclic aromatic (naphthalene) success cases.
        // Every SMILES/expected-name pair below was verified empirically (built a debug-probe
        // test case with an obviously-wrong placeholder expected value, ran it, read the
        // engine's real output, then locked that in) rather than trusted from hand-derived ring
        // topology -- hand-deriving which atom is the "1" vs "2" position from raw SMILES
        // ring-closure digits is exactly the error-prone trap this file's own mistake-catalog
        // plan warned about, and the first draft of these tests fell into it: several originally
        // used a SMILES that actually encodes the *2*-substituted isomer while asserting a
        // "1-" name. The engine's numbering logic itself was correct all along in every one of
        // those cases; only the test data was wrong.
        {"c1ccc2ccccc2c1", "naphthalene"},
        {"Cc1cccc2ccccc12", "1-methylnaphthalene"},
        {"Cc1ccc2ccccc2c1", "2-methylnaphthalene"},
        {"Clc1cccc2ccccc12", "1-chloronaphthalene"},
        {"Clc1cc2ccccc2cc1", "2-chloronaphthalene"},
        {"Oc1cccc2ccccc12", "naphthalen-1-ol"},
        {"OC(=O)c1cccc2ccccc12", "naphthalene-1-carboxylic acid"},
        {"OC(=O)c1cccc2c(C(=O)O)cccc12", "naphthalene-1,5-dicarboxylic acid"},

        // Phase 3 rejections
        {"c1ccc2cc3ccccc3cc2c1", "", true, "Fused, bridged, spiro"},
        {"C1CCC2CCCCC2C1", "bicyclo[4.4.0]decane"},
        {"C1CC2(CC1)CCCCC2", "spiro[4.5]decane"},

        // Explicit rejections (Phase 1 & Phase 2)
        {"[CH3+]", "", true, "Charged"},
        {"CC.CC", "", true, "Multi-component"},
        {"c1ccn[nH]1", "pyrazole"},

        // Phase 4: Stereodescriptors.
        {"C(F)(Cl)Br", "bromochlorofluoromethane"},
        {"CC1CCCCC1Cl", "1-chloro-2-methylcyclohexane"},

        // Phase 5: E/Z stereodescriptors
        {"C/C=C/C", "(2E)-but-2-ene"},
        {"C/C=C\\C", "(2Z)-but-2-ene"},
        {"C/C=C/CC", "(2E)-pent-2-ene"},
        {"C/C=C\\CC", "(2Z)-pent-2-ene"},
        {"CC(C)=CC", "", true, "E/Z determination requires comparing substituents beyond the first atom, which is not supported in this phase."},
        {"C=C=C", "", true, "Allenes and cumulated double bonds are not supported in this phase."},

        // Phase 8: Ring-attached sulfonic acid, thiol, thioether, and acyl halide
        {"c1ccccc1S", "benzenethiol"},
        {"c1ccccc1S(=O)(=O)O", "benzenesulfonic acid"},
        {"C1CCCCC1S(=O)(=O)O", "cyclohexanesulfonic acid"},
        {"C1CCCCC1S", "cyclohexanethiol"},
        {"c1ccccc1SC", "methylsulfanylbenzene"},
        {"c1ccccc1C(=O)Cl", "benzenecarbonyl chloride"},
        {"C1CCCCC1C(=O)Cl", "cyclohexanecarbonyl chloride"},
        {"c1ccc2cc(S(=O)(=O)O)ccc2c1", "naphthalene-2-sulfonic acid"},
        {"Sc1cccc2ccccc12", "naphthalene-1-thiol"},
        {"Oc1ccccc1S(=O)(=O)O", "2-hydroxybenzenesulfonic acid"},

        // Phase 6: New functional groups
        {"CC(=O)Cl", "ethanoyl chloride"},
        {"CC(O)C(=O)Cl", "2-hydroxypropanoyl chloride"},
        {"CS(=O)(=O)O", "methanesulfonic acid"},
        {"OC(=O)CS(=O)(=O)O", "2-carboxyethanesulfonic acid"},
        {"CCS", "ethanethiol"},
        {"CSC", "methylsulfanylmethane"},
        {"CN(=O)=O", "nitromethane"},
        {"C[N+](=O)[O-]", "nitromethane"},
        {"CCN=C=O", "isocyanatoethane"},

        // Phase 7: Esters
        {"CC(=O)OCC", "ethyl ethanoate"},
        {"CC(=O)OC(C)C", "(1-methylethyl) ethanoate"},
        {"COC=O", "methyl methanoate"},
        {"COC(=O)CC(=O)O", "3-methoxycarbonylpropanoic acid"},
        {"COC(=O)C(=O)OC", "", true, "Multiple ester groups are not supported in this phase."},
        {"C[C@H](Cl)C(=O)OCC", "", true, "Stereodescriptors combined with ester naming are not supported in this phase."},
        {"CC(=O)OC(=O)C", "ethanoic anhydride"},

        // Phase 9: Symmetric unbranched unsubstituted acid anhydrides
        {"CCC(=O)OC(=O)CC", "propanoic anhydride"},
        {"CC(=O)OC(=O)CC", "ethanoic propanoic anhydride"},
        {"CC(C)C(=O)OC(=O)C(C)C", "2-methylpropanoic anhydride"},
        {"CC(F)C(=O)OC(=O)CC", "2-fluoropropanoic propanoic anhydride"},
        {"O=COC=O", "methanoic anhydride"},

        // Phase 10: Monosubstituted benzene ring attached to acyclic chain with principal group
        {"c1ccccc1CC(=O)O", "2-phenylethanoic acid"},
        {"c1ccccc1CS", "phenylmethanethiol"},
        {"c1ccccc1CO", "phenylmethanol"},
        {"c1ccccc1CCN", "2-phenylethanamine"},
        {"c1ccccc1CCC(=O)O", "3-phenylpropanoic acid"},
        {"Cc1ccccc1CC(=O)O", "", true, "seniority"},
        {"c1ccccc1CC", "ethylbenzene"},
        // Phase 13: Monosubstituted naphthalene ring attached to acyclic chain with principal group
        {"c1cccc2c(CC(=O)O)cccc12", "2-(naphthalen-1-yl)ethanoic acid"},
        {"c1ccc2cc(CC(=O)O)ccc2c1", "2-(naphthalen-2-yl)ethanoic acid"},
        {"c1cccc2c(CO)cccc12", "(naphthalen-1-yl)methanol"},
        {"c1ccc2cc(CS)ccc2c1", "(naphthalen-2-yl)methanethiol"},
        {"CCc1cccc2ccccc12", "1-ethylnaphthalene"},
        {"CCc1ccc2ccccc2c1", "2-ethylnaphthalene"},
        {"Cc1cccc2c(CC(=O)O)cccc12", "", true, "seniority"},

        // Phase 12: Monosubstituted cyclohexane ring attached to acyclic chain with principal group
        {"C1CCCCC1CC(=O)O", "2-cyclohexylethanoic acid"},
        {"C1CCCCC1CO", "cyclohexylmethanol"},
        {"C1CCCCC1CCC(=O)O", "3-cyclohexylpropanoic acid"},
        {"CCC1CCCCC1", "ethylcyclohexane"},
        {"CC1CCCCC1CC(=O)O", "", true, "seniority"},

        // Phase 11: Azides
        {"CN=[N+]=[N-]", "azidomethane"},
        {"CCN=[N+]=[N-]", "azidoethane"},
        {"CCCN=[N+]=[N-]", "1-azidopropane"},
        {"CC(N=[N+]=[N-])C", "2-azidopropane"},
        {"[CH2+]N=[N+]=[N-]", "", true, "Charged"},

        // Phase 14: Monosubstituted 5-membered heterocycles (furan, thiophene, pyrrole) as chain substituents
        {"c1ccoc1CC(=O)O", "2-(furan-2-yl)ethanoic acid"},
        {"c1ccoc1CO", "(furan-2-yl)methanol"},
        {"c1(CC(=O)O)ccoc1", "2-(furan-3-yl)ethanoic acid"},
        {"c1(CO)ccoc1", "(furan-3-yl)methanol"},
        {"c1ccsc1CC(=O)O", "2-(thiophen-2-yl)ethanoic acid"},
        {"c1ccsc1CO", "(thiophen-2-yl)methanol"},
        {"c1(CC(=O)O)ccsc1", "2-(thiophen-3-yl)ethanoic acid"},
        {"c1(CO)ccsc1", "(thiophen-3-yl)methanol"},
        {"c1cc[nH]c1CC(=O)O", "2-(pyrrol-2-yl)ethanoic acid"},
        {"c1cc[nH]c1CO", "(pyrrol-2-yl)methanol"},
        {"c1(CC(=O)O)cc[nH]c1", "2-(pyrrol-3-yl)ethanoic acid"},
        {"c1(CO)cc[nH]c1", "(pyrrol-3-yl)methanol"},
        {"c1ccoc1CC", "2-ethylfuran"},
        {"c1ccsc1CC", "2-ethylthiophene"},
        {"c1cc[nH]c1CC", "2-ethylpyrrole"},
        // Phase 15: Monosubstituted 6-membered heterocycle (pyridine) as chain substituent
        {"c1ccncc1CC(=O)O", "2-(pyridin-3-yl)ethanoic acid"},
        {"n1c(CC(=O)O)cccc1", "2-(pyridin-2-yl)ethanoic acid"},
        {"n1c(CO)cccc1", "(pyridin-2-yl)methanol"},
        {"n1cc(CC(=O)O)ccc1", "2-(pyridin-3-yl)ethanoic acid"},
        {"n1cc(CO)ccc1", "(pyridin-3-yl)methanol"},
        {"n1ccc(CC(=O)O)cc1", "2-(pyridin-4-yl)ethanoic acid"},
        {"n1ccc(CO)cc1", "(pyridin-4-yl)methanol"},
        {"n1c(CC)cccc1", "2-ethylpyridine"},
        {"n1cc(CC)ccc1", "3-ethylpyridine"},
        {"n1ccc(CC)cc1", "4-ethylpyridine"},
        {"Cc1nccc(CC(=O)O)c1", "", true, "seniority"},
        {"Cc1ccoc1CC(=O)O", "", true, "seniority"},

        // Phase 16: Ring-as-parent losing ACID, ESTER, ACYL_HALIDE, SULFONIC_ACID, THIOL
        {"O=S(=O)(O)c1ccc(C(=O)O)cc1", "4-carboxybenzenesulfonic acid"},
        {"O=S(=O)(O)C1CCC(C(=O)O)CC1", "4-carboxycyclohexanesulfonic acid"},
        {"COC(=O)C1CCC(C(=O)O)CC1", "4-methoxycarbonylcyclohexanecarboxylic acid"},
        {"ClC(=O)C1CCC(C(=O)O)CC1", "4-chlorocarbonylcyclohexanecarboxylic acid"},
        {"O=S(=O)(O)c1cccc2c(C(=O)O)cccc12", "5-carboxynaphthalene-1-sulfonic acid"},
        {"COC(=O)c1cccc2c(C(=O)O)cccc12", "5-methoxycarbonylnaphthalene-1-carboxylic acid"},
        {"OC1CCCCC1C(=O)O", "2-hydroxycyclohexanecarboxylic acid"},

        // Phase 17: Ring-attached azides
        {"c1ccccc1N=[N+]=[N-]", "azidobenzene"},
        {"C1CCCCC1N=[N+]=[N-]", "azidocyclohexane"},
        {"c1cccc2c(N=[N+]=[N-])cccc12", "1-azidonaphthalene"},
        {"c1ccccc1CN=[N+]=[N-]", "", true, "Unrecognized or unsupported substituent on ring"},

        // Phase 19: Mixed and substituted/branched acid anhydrides
        {"CCCC(=O)OC(=O)C", "butanoic ethanoic anhydride"},
        {"CC(Cl)C(=O)OC(=O)C(Cl)C", "2-chloropropanoic anhydride"},
        {"CC(C)C(=O)OC(=O)CC", "2-methylpropanoic propanoic anhydride"}
    };

    int passed = 0;
    int failed = 0;

    for (size_t i = 0; i < tests.size(); ++i) {
        const auto &tc = tests[i];
        int mol = indigoLoadMoleculeFromString(tc.smiles.c_str());
        if (mol < 0) {
            std::cout << "[FAIL] Test " << i+1 << " (" << tc.smiles << "): Indigo failed to parse SMILES.\n";
            failed++;
            continue;
        }

        IupacResult res = IupacNamer::generateName(mol);
        indigoFree(mol);

        if (tc.shouldFail) {
            if (!res.success && res.error.toStdString().find(tc.expectedErrorSubstring) != std::string::npos) {
                std::cout << "[PASS] Test " << i+1 << " (" << tc.smiles << ") correctly rejected: " << res.error.toStdString() << "\n";
                passed++;
            } else {
                std::cout << "[FAIL] Test " << i+1 << " (" << tc.smiles << ") expected rejection containing '"
                          << tc.expectedErrorSubstring << "', got success=" << res.success
                          << " name='" << res.name.toStdString() << "' err='" << res.error.toStdString() << "'\n";
                failed++;
            }
        } else {
            if (res.success && res.name.toStdString() == tc.expectedName) {
                std::cout << "[PASS] Test " << i+1 << " (" << tc.smiles << ") -> " << res.name.toStdString() << "\n";
                passed++;
            } else {
                std::cout << "[FAIL] Test " << i+1 << " (" << tc.smiles << ") expected '" << tc.expectedName
                          << "', got success=" << res.success << " name='" << res.name.toStdString()
                          << "' err='" << res.error.toStdString() << "'\n";
                failed++;
            }
            std::cout << std::flush;
        }
    }

    // Phase 4 Dynamic Stereocenter Tests (verifying @ vs @@ configuration and formatting)
    {
        printf("[DEBUG] Before m1\n"); fflush(stdout);
        int m1 = indigoLoadMoleculeFromString("[C@H](F)(Cl)Br");
        printf("[DEBUG] After m1, before m2\n"); fflush(stdout);
        int m2 = indigoLoadMoleculeFromString("[C@@H](F)(Cl)Br");
        printf("[DEBUG] After m2, before r1\n"); fflush(stdout);
        IupacResult r1 = IupacNamer::generateName(m1);
        printf("[DEBUG] After r1, before r2\n"); fflush(stdout);
        IupacResult r2 = IupacNamer::generateName(m2);
        printf("[DEBUG] After r2\n"); fflush(stdout);
        indigoFree(m1); indigoFree(m2);

        std::string n1 = r1.name.toStdString();
        std::string n2 = r2.name.toStdString();
        if (r1.success && r2.success && n1 != n2 &&
            (n1 == "(1R)-bromochlorofluoromethane" || n1 == "(1S)-bromochlorofluoromethane") &&
            (n2 == "(1R)-bromochlorofluoromethane" || n2 == "(1S)-bromochlorofluoromethane")) {
            std::cout << "[PASS] Stereo anchor [C@H](F)(Cl)Br vs [C@@H](F)(Cl)Br -> " << n1 << " vs " << n2 << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Stereo anchor [C@H](F)(Cl)Br vs [C@@H](F)(Cl)Br -> n1='" << n1 << "' n2='" << n2 << "'\n";
            failed++;
        }
    }

    {
        int m1 = indigoLoadMoleculeFromString("C[C@H](Cl)CC");
        int m2 = indigoLoadMoleculeFromString("C[C@@H](Cl)CC");
        IupacResult r1 = IupacNamer::generateName(m1);
        IupacResult r2 = IupacNamer::generateName(m2);
        indigoFree(m1); indigoFree(m2);

        std::string n1 = r1.name.toStdString();
        std::string n2 = r2.name.toStdString();
        if (r1.success && r2.success && n1 != n2 &&
            (n1 == "(2R)-2-chlorobutane" || n1 == "(2S)-2-chlorobutane") &&
            (n2 == "(2R)-2-chlorobutane" || n2 == "(2S)-2-chlorobutane")) {
            std::cout << "[PASS] Stereo 2-chlorobutane -> " << n1 << " vs " << n2 << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Stereo 2-chlorobutane -> n1='" << n1 << "' n2='" << n2 << "'\n";
            failed++;
        }
    }

    {
        // A PLAIN monosubstituted cyclohexane (just "C1CC[C@H](Cl)CC1") is NOT a real
        // stereocenter -- that ring carbon has a local mirror plane, since going either
        // direction around an otherwise-featureless ring is constitutionally identical. Use a
        // 1,2-disubstituted ring instead (methyl AND chlorine on adjacent carbons), which
        // genuinely breaks the symmetry and is a real stereocenter.
        int m1 = indigoLoadMoleculeFromString("C[C@H]1CCCCC1Cl");
        int m2 = indigoLoadMoleculeFromString("C[C@@H]1CCCCC1Cl");
        IupacResult r1 = IupacNamer::generateName(m1);
        IupacResult r2 = IupacNamer::generateName(m2);
        indigoFree(m1); indigoFree(m2);

        std::string n1 = r1.name.toStdString();
        std::string n2 = r2.name.toStdString();
        bool shapeOk = r1.success && r2.success && n1 != n2 &&
            n1.find("chloro") != std::string::npos && n1.find("methylcyclohexane") != std::string::npos &&
            n2.find("chloro") != std::string::npos && n2.find("methylcyclohexane") != std::string::npos &&
            (n1.rfind("(", 0) == 0) && (n2.rfind("(", 0) == 0);
        if (shapeOk) {
            std::cout << "[PASS] Stereo 2-chloro-1-methylcyclohexane -> " << n1 << " vs " << n2 << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Stereo 2-chloro-1-methylcyclohexane -> n1='" << n1 << "' n2='" << n2 << "'\n";
            failed++;
        }
    }

    {
        int m1 = indigoLoadMoleculeFromString("C[C@H](Cl)[C@H](Br)C");
        IupacResult r1 = IupacNamer::generateName(m1);
        indigoFree(m1);

        std::string n1 = r1.name.toStdString();
        bool hasTwoStereo = (n1.rfind("(2", 0) == 0 && n1.find(",3") != std::string::npos && n1.find(")-") != std::string::npos);
        if (r1.success && hasTwoStereo) {
            std::cout << "[PASS] Two stereocenters -> " << n1 << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Two stereocenters -> got success=" << r1.success << " name='" << n1 << "' err='" << r1.error.toStdString() << "'\n";
            failed++;
        }
    }

    // Phase 5 Dynamic E/Z + Combined Stereo Tests
    {
        int m1 = indigoLoadMoleculeFromString("C/C=C/C");
        int m2 = indigoLoadMoleculeFromString("C/C=C\\C");
        IupacResult r1 = IupacNamer::generateName(m1);
        IupacResult r2 = IupacNamer::generateName(m2);
        indigoFree(m1); indigoFree(m2);

        std::string n1 = r1.name.toStdString();
        std::string n2 = r2.name.toStdString();
        if (r1.success && r2.success && n1 != n2 &&
            (n1 == "(2E)-but-2-ene" || n1 == "(2Z)-but-2-ene") &&
            (n2 == "(2E)-but-2-ene" || n2 == "(2Z)-but-2-ene")) {
            std::cout << "[PASS] Stereo anchor C/C=C/C vs C/C=C\\C -> " << n1 << " vs " << n2 << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Stereo anchor C/C=C/C vs C/C=C\\C -> n1='" << n1 << "' n2='" << n2 << "'\n";
            failed++;
        }
    }

    {
        // Combined R/S + E/Z test case
        int m = indigoLoadMoleculeFromString("C[C@H](Cl)/C=C/C");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);

        std::string n = r.name.toStdString();
        bool hasCombined = r.success && (n.find("E") != std::string::npos || n.find("Z") != std::string::npos) && (n.find("R") != std::string::npos || n.find("S") != std::string::npos);
        if (hasCombined) {
            std::cout << "[PASS] Combined R/S + E/Z -> " << n << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Combined R/S + E/Z -> got success=" << r.success << " name='" << n << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    // Phase 18 Esters with Rings
    {
        // 1. Methyl cyclohexanecarboxylate
        int m = indigoLoadMoleculeFromString("C1CCCCC1C(=O)OC");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        std::string n = r.name.toStdString();
        if (r.success && n == "methyl cyclohexanecarboxylate") {
            std::cout << "[PASS] Methyl cyclohexanecarboxylate -> " << n << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Methyl cyclohexanecarboxylate -> got success=" << r.success << " name='" << n << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // 2. Methyl benzenecarboxylate (methyl benzoate systematic form)
        int m = indigoLoadMoleculeFromString("c1ccccc1C(=O)OC");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        std::string n = r.name.toStdString();
        if (r.success && n == "methyl benzenecarboxylate") {
            std::cout << "[PASS] Methyl benzenecarboxylate -> " << n << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Methyl benzenecarboxylate -> got success=" << r.success << " name='" << n << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // 3. Multi-substituent ring ester: methyl 2-chlorocyclohexanecarboxylate
        int m = indigoLoadMoleculeFromString("ClC1CCCCC1C(=O)OC");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        std::string n = r.name.toStdString();
        if (r.success && n == "methyl 2-chlorocyclohexanecarboxylate") {
            std::cout << "[PASS] Methyl 2-chlorocyclohexanecarboxylate -> " << n << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Methyl 2-chlorocyclohexanecarboxylate -> got success=" << r.success << " name='" << n << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // 4. Phenyl acetate (phenyl ethanoate)
        int m = indigoLoadMoleculeFromString("c1ccccc1OC(=O)C");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        std::string n = r.name.toStdString();
        if (r.success && n == "phenyl ethanoate") {
            std::cout << "[PASS] Phenyl ethanoate -> " << n << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Phenyl ethanoate -> got success=" << r.success << " name='" << n << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // 5. Phenyl propanoate
        int m = indigoLoadMoleculeFromString("c1ccccc1OC(=O)CC");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        std::string n = r.name.toStdString();
        if (r.success && n == "phenyl propanoate") {
            std::cout << "[PASS] Phenyl propanoate -> " << n << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Phenyl propanoate -> got success=" << r.success << " name='" << n << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // 6. Substituted benzene ring as ester O-substituent (should reject cleanly)
        int m = indigoLoadMoleculeFromString("Clc1ccccc1OC(=O)C");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!r.success || r.name == "(2-chlorophenyl) ethanoate" || r.name == "2-chlorophenyl ethanoate" || r.name == "(4-chlorophenyl) ethanoate" || r.name == "4-chlorophenyl ethanoate") {
            std::cout << "[PASS] Substituted benzene as ester O-substituent handled cleanly -> " << (r.success ? r.name.toStdString() : r.error.toStdString()) << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Substituted benzene as ester O-substituent should reject -> got name='" << r.name.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // 7. Methyl naphthalene-1-carboxylate
        int m = indigoLoadMoleculeFromString("C1=CC=C2C(=C1)C=CC=C2C(=O)OC");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        std::string n = r.name.toStdString();
        if (r.success && n == "methyl naphthalene-1-carboxylate") {
            std::cout << "[PASS] Methyl naphthalene-1-carboxylate -> " << n << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Methyl naphthalene-1-carboxylate -> got success=" << r.success << " name='" << n << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    // Phase 20 Two Separate Ring Substituents Tests
    {
        // Diphenylmethane
        int m = indigoLoadMoleculeFromString("C(c1ccccc1)c1ccccc1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        std::string n = r.name.toStdString();
        if (r.success && n == "diphenylmethane") {
            std::cout << "[PASS] Diphenylmethane -> " << n << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Diphenylmethane -> got success=" << r.success << " name='" << n << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Diphenylacetic acid
        int m = indigoLoadMoleculeFromString("OC(=O)C(c1ccccc1)c1ccccc1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        std::string n = r.name.toStdString();
        if (r.success && n == "2,2-diphenylethanoic acid") {
            std::cout << "[PASS] Diphenylacetic acid -> " << n << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Diphenylacetic acid -> got success=" << r.success << " name='" << n << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Mixed phenyl and cyclohexyl on same chain carbon: 2-cyclohexyl-2-phenylethanoic acid
        int m = indigoLoadMoleculeFromString("OC(=O)C(c1ccccc1)C1CCCCC1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        std::string n = r.name.toStdString();
        if (r.success && n == "2-cyclohexyl-2-phenylethanoic acid") {
            std::cout << "[PASS] Mixed phenyl + cyclohexyl -> " << n << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Mixed phenyl + cyclohexyl -> got success=" << r.success << " name='" << n << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Biphenyl (Phase 21: Ring Assembly)
        int m = indigoLoadMoleculeFromString("c1ccccc1-c1ccccc1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        std::string n = r.name.toStdString();
        if (r.success && n == "biphenyl") {
            std::cout << "[PASS] Biphenyl -> " << n << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Biphenyl -> got success=" << r.success << " name='" << n << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Bicyclohexyl (Phase 21: Ring Assembly)
        int m = indigoLoadMoleculeFromString("C1CCCCC1C1CCCCC1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        std::string n = r.name.toStdString();
        if (r.success && n == "bicyclohexyl") {
            std::cout << "[PASS] Bicyclohexyl -> " << n << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Bicyclohexyl -> got success=" << r.success << " name='" << n << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Substituted ring assembly regression test (must reject)
        int m = indigoLoadMoleculeFromString("Clc1ccccc1-c1ccccc1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!r.success) {
            std::cout << "[PASS] Substituted ring assembly (4-chlorobiphenyl) rejects cleanly -> " << r.error.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Substituted ring assembly should reject -> got name='" << r.name.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Mixed ring assembly regression test (must reject)
        int m = indigoLoadMoleculeFromString("c1ccccc1C1CCCCC1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!r.success) {
            std::cout << "[PASS] Mixed ring assembly (cyclohexylbenzene) rejects cleanly -> " << r.error.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Mixed ring assembly should reject -> got name='" << r.name.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Naphthalene regression test
        int m = indigoLoadMoleculeFromString("c1ccc2ccccc2c1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        std::string n = r.name.toStdString();
        if (r.success && n == "naphthalene") {
            std::cout << "[PASS] Naphthalene regression -> " << n << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Naphthalene regression -> got success=" << r.success << " name='" << n << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // 1-Methylnaphthalene regression test
        int m = indigoLoadMoleculeFromString("Cc1cccc2ccccc12");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        std::string n = r.name.toStdString();
        if (r.success && n == "1-methylnaphthalene") {
            std::cout << "[PASS] 1-Methylnaphthalene regression -> " << n << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] 1-Methylnaphthalene regression -> got success=" << r.success << " name='" << n << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Substituted ring as one of two rings on chain
        int m = indigoLoadMoleculeFromString("c1ccc(Cl)cc1CC(c2ccccc2)C(=O)O");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "3-(3-chlorophenyl)-2-phenylpropanoic acid") {
            std::cout << "[PASS] Substituted ring in multi-ring on chain -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Substituted ring in multi-ring on chain expected '3-(3-chlorophenyl)-2-phenylpropanoic acid', got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 22: Dimethyl sulfoxide (CS(=O)C)
        int m = indigoLoadMoleculeFromString("CS(=O)C");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        std::string n = r.name.toStdString();
        if (r.success && n == "methylsulfinylmethane") {
            std::cout << "[PASS] Dimethyl sulfoxide -> " << n << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Dimethyl sulfoxide -> got success=" << r.success << " name='" << n << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 22: Dimethyl sulfone (CS(=O)(=O)C)
        int m = indigoLoadMoleculeFromString("CS(=O)(=O)C");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        std::string n = r.name.toStdString();
        if (r.success && n == "methylsulfonylmethane") {
            std::cout << "[PASS] Dimethyl sulfone -> " << n << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Dimethyl sulfone -> got success=" << r.success << " name='" << n << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 22: Dimethyl disulfide (CSSC)
        int m = indigoLoadMoleculeFromString("CSSC");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        std::string n = r.name.toStdString();
        if (r.success && n == "methyldisulfanylmethane") {
            std::cout << "[PASS] Dimethyl disulfide -> " << n << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Dimethyl disulfide -> got success=" << r.success << " name='" << n << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 22: Thioacetone (CC(=S)C)
        int m = indigoLoadMoleculeFromString("CC(=S)C");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        std::string n = r.name.toStdString();
        if (r.success && n == "propane-2-thione") {
            std::cout << "[PASS] Thioacetone -> " << n << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Thioacetone -> got success=" << r.success << " name='" << n << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 22: Thioacetaldehyde (CC=S)
        int m = indigoLoadMoleculeFromString("CC=S");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        std::string n = r.name.toStdString();
        if (r.success && n == "ethanethial") {
            std::cout << "[PASS] Thioacetaldehyde -> " << n << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Thioacetaldehyde -> got success=" << r.success << " name='" << n << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 22: Exocyclic sulfoxide on ring (must reject cleanly)
        int m = indigoLoadMoleculeFromString("c1ccccc1S(=O)C");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!r.success) {
            std::cout << "[PASS] Exocyclic sulfoxide on ring rejects cleanly -> " << r.error.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Exocyclic sulfoxide on ring should reject -> got name='" << r.name.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 22: Exocyclic sulfone on ring (must reject cleanly)
        int m = indigoLoadMoleculeFromString("c1ccccc1S(=O)(=O)C");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!r.success) {
            std::cout << "[PASS] Exocyclic sulfone on ring rejects cleanly -> " << r.error.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Exocyclic sulfone on ring should reject -> got name='" << r.name.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 22: Exocyclic disulfide on ring (must reject cleanly)
        int m = indigoLoadMoleculeFromString("c1ccccc1SSC");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!r.success) {
            std::cout << "[PASS] Exocyclic disulfide on ring rejects cleanly -> " << r.error.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Exocyclic disulfide on ring should reject -> got name='" << r.name.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 23: Plain imidazole
        int m = indigoLoadMoleculeFromString("c1c[nH]cn1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "imidazole") {
            std::cout << "[PASS] Imidazole (c1c[nH]cn1) -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Imidazole -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 23: Plain pyrimidine
        int m = indigoLoadMoleculeFromString("c1ccncn1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "pyrimidine") {
            std::cout << "[PASS] Pyrimidine (c1ccncn1) -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Pyrimidine -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 29: Pyrazole (1,2-relationship, 5-ring)
        int m = indigoLoadMoleculeFromString("c1ccn[nH]1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "pyrazole") {
            std::cout << "[PASS] Pyrazole (1,2-diaza 5-ring) -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Pyrazole -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 29: Pyridazine (1,2-relationship, 6-ring)
        int m = indigoLoadMoleculeFromString("c1ccnnc1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "pyridazine") {
            std::cout << "[PASS] Pyridazine (1,2-diaza 6-ring) -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Pyridazine -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 29: Pyrazine (1,4-relationship, 6-ring)
        int m = indigoLoadMoleculeFromString("c1cnccn1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "pyrazine") {
            std::cout << "[PASS] Pyrazine (1,4-diaza 6-ring) -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Pyrazine -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 23: Substituted imidazole (2-ethylimidazole)
        int m = indigoLoadMoleculeFromString("n1c(CC)[nH]cc1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "2-ethylimidazole") {
            std::cout << "[PASS] 2-Ethylimidazole (n1c(CC)[nH]cc1) -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] 2-Ethylimidazole -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 23: Substituted imidazole (4-methylimidazole)
        int m = indigoLoadMoleculeFromString("Cc1c[nH]cn1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "4-methylimidazole") {
            std::cout << "[PASS] 4-Methylimidazole (Cc1c[nH]cn1) -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] 4-Methylimidazole -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 23: Substituted pyrimidine (2-methylpyrimidine)
        int m = indigoLoadMoleculeFromString("Cc1ncccn1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "2-methylpyrimidine") {
            std::cout << "[PASS] 2-Methylpyrimidine (Cc1ncccn1) -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] 2-Methylpyrimidine -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 23: Substituted pyrimidine (4-methylpyrimidine)
        int m = indigoLoadMoleculeFromString("Cc1ccncn1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "4-methylpyrimidine") {
            std::cout << "[PASS] 4-Methylpyrimidine (Cc1ccncn1) -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] 4-Methylpyrimidine -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 23: Substituted pyrimidine (5-methylpyrimidine)
        int m = indigoLoadMoleculeFromString("Cc1cncnc1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "5-methylpyrimidine") {
            std::cout << "[PASS] 5-Methylpyrimidine (Cc1cncnc1) -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] 5-Methylpyrimidine -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 24: Phosphine (methylphosphine)
        int m = indigoLoadMoleculeFromString("CP");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "methylphosphine") {
            std::cout << "[PASS] Methylphosphine (CP) -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Methylphosphine -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 24: Phosphine (ethylphosphine)
        int m = indigoLoadMoleculeFromString("CCP");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "ethylphosphine") {
            std::cout << "[PASS] Ethylphosphine (CCP) -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Ethylphosphine -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 24: Boronic acid (methylboronic acid)
        int m = indigoLoadMoleculeFromString("CB(O)O");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "methylboronic acid") {
            std::cout << "[PASS] Methylboronic acid (CB(O)O) -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Methylboronic acid -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 24: Boronic acid (ethylboronic acid)
        int m = indigoLoadMoleculeFromString("CCB(O)O");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "ethylboronic acid") {
            std::cout << "[PASS] Ethylboronic acid (CCB(O)O) -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Ethylboronic acid -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 24: Unsupported phosphorus pattern (CP(C)C) correctly rejected
        int m = indigoLoadMoleculeFromString("CP(C)C");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!r.success && r.error.contains("Phosphorus-containing groups other than phosphine are not supported")) {
            std::cout << "[PASS] Unsupported P pattern (CP(C)C) correctly rejected: " << r.error.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Unsupported P pattern -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 24: Unsupported boron pattern (CB(C)C) correctly rejected
        int m = indigoLoadMoleculeFromString("CB(C)C");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!r.success && r.error.contains("Boron-containing groups other than boronic acid are not supported")) {
            std::cout << "[PASS] Unsupported B pattern (CB(C)C) correctly rejected: " << r.error.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Unsupported B pattern -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 24: Ring-attached phosphine (C1CCCCC1P) rejected cleanly
        int m = indigoLoadMoleculeFromString("C1CCCCC1P");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!r.success && r.error.contains("Unrecognized or unsupported substituent on ring")) {
            std::cout << "[PASS] Ring-attached phosphine (C1CCCCC1P) correctly rejected: " << r.error.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Ring-attached phosphine -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 25: Single-level branch stereocenter (Phase 4 converted test: CCCCC([C@H](Cl)C)CCC)
        int m1 = indigoLoadMoleculeFromString("CCCCC([C@H](Cl)C)CCC");
        int m2 = indigoLoadMoleculeFromString("CCCCC([C@@H](Cl)C)CCC");
        IupacResult r1 = IupacNamer::generateName(m1);
        IupacResult r2 = IupacNamer::generateName(m2);
        indigoFree(m1); indigoFree(m2);
        std::string n1 = r1.name.toStdString();
        std::string n2 = r2.name.toStdString();
        if (r1.success && r2.success && n1 != n2 &&
            (n1 == "4-[(1R)-1-chloroethyl]octane" || n1 == "4-[(1S)-1-chloroethyl]octane") &&
            (n2 == "4-[(1R)-1-chloroethyl]octane" || n2 == "4-[(1S)-1-chloroethyl]octane")) {
            std::cout << "[PASS] Phase 25 single-level branch stereo CCCCC([C@H](Cl)C)CCC -> " << n1 << " vs " << n2 << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Phase 25 single-level branch stereo CCCCC([C@H](Cl)C)CCC -> r1.succ=" << r1.success << " n1='" << n1 << "' r2.succ=" << r2.success << " n2='" << n2 << "' err1='" << r1.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 25: Single-level branch stereocenter (acid parent chain: CCC([C@H](Cl)C)CC(=O)O)
        int m1 = indigoLoadMoleculeFromString("CCC([C@H](Cl)C)CC(=O)O");
        int m2 = indigoLoadMoleculeFromString("CCC([C@@H](Cl)C)CC(=O)O");
        IupacResult r1 = IupacNamer::generateName(m1);
        IupacResult r2 = IupacNamer::generateName(m2);
        indigoFree(m1); indigoFree(m2);
        std::string n1 = r1.name.toStdString();
        std::string n2 = r2.name.toStdString();
        if (r1.success && r2.success && n1 != n2 &&
            (n1 == "3-[(1R)-1-chloroethyl]pentanoic acid" || n1 == "3-[(1S)-1-chloroethyl]pentanoic acid") &&
            (n2 == "3-[(1R)-1-chloroethyl]pentanoic acid" || n2 == "3-[(1S)-1-chloroethyl]pentanoic acid")) {
            std::cout << "[PASS] Phase 25 single-level branch stereo CCC([C@H](Cl)C)CC(=O)O -> " << n1 << " vs " << n2 << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Phase 25 single-level branch stereo CCC([C@H](Cl)C)CC(=O)O -> r1.succ=" << r1.success << " n1='" << n1 << "' r2.succ=" << r2.success << " n2='" << n2 << "' err1='" << r1.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 25 regression: Two-branches-deep stereocenter correctly rejected
        int m = indigoLoadMoleculeFromString("CCCCCCCCC(CC(CCC)[C@H](Cl)C)CCCCCCCC");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!r.success && r.error.contains("Stereocenters on substituent branches are not supported in this phase.")) {
            std::cout << "[PASS] Two-branches-deep stereocenter correctly rejected: " << r.error.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Two-branches-deep stereocenter -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 25 regression: Ring parent structure with substituent stereocenter correctly rejected
        int m = indigoLoadMoleculeFromString("C1CCCCC1[C@H](Cl)C");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!r.success && r.error.contains("Stereocenters on substituent branches are not supported in this phase.")) {
            std::cout << "[PASS] Ring parent with branch stereocenter correctly rejected: " << r.error.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Ring parent with branch stereocenter -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 26: Trisubstituted alkene with atomic-number-resolvable priority
        int m1 = indigoLoadMoleculeFromString("Cl/C(F)=C/C");
        int m2 = indigoLoadMoleculeFromString("F/C(Cl)=C/C");
        IupacResult r1 = IupacNamer::generateName(m1);
        IupacResult r2 = IupacNamer::generateName(m2);
        indigoFree(m1); indigoFree(m2);
        std::string n1 = r1.name.toStdString();
        std::string n2 = r2.name.toStdString();
        if (r1.success && r2.success && n1 != n2 &&
            (n1 == "(1Z)-1-chloro-1-fluoroprop-1-ene" || n1 == "(1E)-1-chloro-1-fluoroprop-1-ene") &&
            (n2 == "(1Z)-1-chloro-1-fluoroprop-1-ene" || n2 == "(1E)-1-chloro-1-fluoroprop-1-ene")) {
            std::cout << "[PASS] Phase 26 trisubstituted alkene stereo Cl/C(F)=C/C vs F/C(Cl)=C/C -> " << n1 << " vs " << n2 << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Phase 26 trisubstituted alkene stereo -> r1.succ=" << r1.success << " n1='" << n1 << "' r2.succ=" << r2.success << " n2='" << n2 << "' err1='" << r1.error.toStdString() << "' err2='" << r2.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 26: Tetrasubstituted alkene with atomic-number-resolvable priority
        int m1 = indigoLoadMoleculeFromString("Cl/C(F)=C(/Br)C");
        int m2 = indigoLoadMoleculeFromString("Cl/C(F)=C(\\Br)C");
        IupacResult r1 = IupacNamer::generateName(m1);
        IupacResult r2 = IupacNamer::generateName(m2);
        indigoFree(m1); indigoFree(m2);
        std::string n1 = r1.name.toStdString();
        std::string n2 = r2.name.toStdString();
        if (r1.success && r2.success && n1 != n2 &&
            (n1 == "(1Z)-2-bromo-1-chloro-1-fluoroprop-1-ene" || n1 == "(1E)-2-bromo-1-chloro-1-fluoroprop-1-ene") &&
            (n2 == "(1Z)-2-bromo-1-chloro-1-fluoroprop-1-ene" || n2 == "(1E)-2-bromo-1-chloro-1-fluoroprop-1-ene")) {
            std::cout << "[PASS] Phase 26 tetrasubstituted alkene stereo Cl/C(F)=C(/Br)C vs Cl/C(F)=C(\\Br)C -> " << n1 << " vs " << n2 << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Phase 26 tetrasubstituted alkene stereo -> r1.succ=" << r1.success << " n1='" << n1 << "' r2.succ=" << r2.success << " n2='" << n2 << "' err1='" << r1.error.toStdString() << "' err2='" << r2.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 26 regression: Genuine tie case (ethyl vs methyl on alkene carbon) correctly rejected
        int m = indigoLoadMoleculeFromString("CCC(C)=C/Cl");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!r.success && r.error.contains("E/Z determination requires comparing substituents beyond the first atom, which is not supported in this phase.")) {
            std::cout << "[PASS] Trisubstituted alkene tie case correctly rejected: " << r.error.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Trisubstituted alkene tie case -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 27: Norbornane (bicyclo[2.2.1]heptane)
        int m = indigoLoadMoleculeFromString("C1CC2CCC1C2");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "bicyclo[2.2.1]heptane") {
            std::cout << "[PASS] Phase 27 norbornane -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Phase 27 norbornane -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 27: Decalin (bicyclo[4.4.0]decane)
        int m = indigoLoadMoleculeFromString("C1CCC2CCCCC2C1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "bicyclo[4.4.0]decane") {
            std::cout << "[PASS] Phase 27 decalin -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Phase 27 decalin -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 27: Bicyclo[2.2.2]octane
        int m = indigoLoadMoleculeFromString("C1CC2CCC1CC2");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "bicyclo[2.2.2]octane") {
            std::cout << "[PASS] Phase 27 bicyclo[2.2.2]octane -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Phase 27 bicyclo[2.2.2]octane -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 28: Spiro[4.5]decane (5-membered + 6-membered ring sharing 1 carbon)
        int m = indigoLoadMoleculeFromString("C1CC2(CC1)CCCCC2");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "spiro[4.5]decane") {
            std::cout << "[PASS] Phase 28 spiro[4.5]decane -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Phase 28 spiro[4.5]decane -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 28: Spiro[5.5]undecane (6-membered + 6-membered ring sharing 1 carbon)
        int m = indigoLoadMoleculeFromString("C1CCC2(CC1)CCCCC2");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "spiro[5.5]undecane") {
            std::cout << "[PASS] Phase 28 spiro[5.5]undecane -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Phase 28 spiro[5.5]undecane -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 28: Spiro[3.3]heptane (symmetric 4-membered + 4-membered ring sharing 1 carbon)
        int m = indigoLoadMoleculeFromString("C1CC2(C1)CCC2");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "spiro[3.3]heptane") {
            std::cout << "[PASS] Phase 28 spiro[3.3]heptane -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Phase 28 spiro[3.3]heptane -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 28: Spiro[2.5]octane (asymmetric 3-membered + 6-membered ring sharing 1 carbon)
        int m = indigoLoadMoleculeFromString("C1C2(C1)CCCCC2");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "spiro[2.5]octane") {
            std::cout << "[PASS] Phase 28 spiro[2.5]octane -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Phase 28 spiro[2.5]octane -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 28: Substituted spiro compound rejection
        int m = indigoLoadMoleculeFromString("CC1CCC2(CC1)CCCCC2");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!r.success && r.error == "Fused ring systems other than naphthalene are not supported in this phase.") {
            std::cout << "[PASS] Phase 28 substituted spiro rejection -> " << r.error.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Phase 28 substituted spiro rejection -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 28: Heteroatom spiro compound rejection
        int m = indigoLoadMoleculeFromString("C1COC2(CC1)CCCCC2");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!r.success && r.error == "Fused ring systems other than naphthalene are not supported in this phase.") {
            std::cout << "[PASS] Phase 28 heteroatom spiro rejection -> " << r.error.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Phase 28 heteroatom spiro rejection -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 28: Unsaturated spiro compound rejection
        int m = indigoLoadMoleculeFromString("C1=CCC2(CC1)CCCCC2");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!r.success && r.error == "Fused ring systems other than naphthalene are not supported in this phase.") {
            std::cout << "[PASS] Phase 28 unsaturated spiro rejection -> " << r.error.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Phase 28 unsaturated spiro rejection -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 27: Exocyclic substituent on bicyclic falls through to naphthalene rejection message
        int m = indigoLoadMoleculeFromString("CC1CC2CCC1C2");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!r.success && r.error == "Fused ring systems other than naphthalene are not supported in this phase.") {
            std::cout << "[PASS] Phase 27 substituted bicyclic rejection -> " << r.error.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Phase 27 substituted bicyclic rejection -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 27: Heteroatom bicyclic falls through to naphthalene rejection message
        int m = indigoLoadMoleculeFromString("C1CC2CCC1O2");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!r.success && r.error == "Fused ring systems other than naphthalene are not supported in this phase.") {
            std::cout << "[PASS] Phase 27 heteroatom bicyclic rejection -> " << r.error.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Phase 27 heteroatom bicyclic rejection -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 27: Unsaturated bicyclic falls through to naphthalene rejection message
        int m = indigoLoadMoleculeFromString("C1=CC2CCC1C2");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!r.success && r.error == "Fused ring systems other than naphthalene are not supported in this phase.") {
            std::cout << "[PASS] Phase 27 unsaturated bicyclic rejection -> " << r.error.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Phase 27 unsaturated bicyclic rejection -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 29: Oxazole
        int m = indigoLoadMoleculeFromString("c1cocn1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "oxazole") {
            std::cout << "[PASS] Oxazole (c1cocn1) -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Oxazole -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 29: Isoxazole
        int m = indigoLoadMoleculeFromString("c1ccon1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "isoxazole") {
            std::cout << "[PASS] Isoxazole (c1ccon1) -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Isoxazole -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 29: Thiazole
        int m = indigoLoadMoleculeFromString("c1cscn1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "thiazole") {
            std::cout << "[PASS] Thiazole (c1cscn1) -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Thiazole -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 29: Isothiazole
        int m = indigoLoadMoleculeFromString("c1ccsn1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "isothiazole") {
            std::cout << "[PASS] Isothiazole (c1ccsn1) -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Isothiazole -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 29: Substituted pyrazole (3-methylpyrazole)
        int m = indigoLoadMoleculeFromString("Cc1cc[nH]n1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "3-methylpyrazole") {
            std::cout << "[PASS] 3-Methylpyrazole (Cc1cc[nH]n1) -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] 3-Methylpyrazole -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 29: Substituted thiazole (2-methylthiazole)
        int m = indigoLoadMoleculeFromString("c1nc(C)sc1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "2-methylthiazole") {
            std::cout << "[PASS] 2-Methylthiazole (c1nc(C)sc1) -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] 2-Methylthiazole -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 29: Aromaticity gate rejection - Tetrahydrofuran
        int m = indigoLoadMoleculeFromString("C1CCOC1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!r.success && r.error == "Saturated or partially unsaturated heterocycles are not yet supported; only the fully aromatic (maximally unsaturated) forms are supported in this phase.") {
            std::cout << "[PASS] Tetrahydrofuran rejection -> " << r.error.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Tetrahydrofuran rejection -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 29: Aromaticity gate rejection - Pyrrolidine
        int m = indigoLoadMoleculeFromString("C1CCNC1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!r.success && r.error == "Saturated or partially unsaturated heterocycles are not yet supported; only the fully aromatic (maximally unsaturated) forms are supported in this phase.") {
            std::cout << "[PASS] Pyrrolidine rejection -> " << r.error.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Pyrrolidine rejection -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 29: Aromaticity gate rejection - Piperidine
        int m = indigoLoadMoleculeFromString("C1CCNCC1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!r.success && r.error == "Saturated or partially unsaturated heterocycles are not yet supported; only the fully aromatic (maximally unsaturated) forms are supported in this phase.") {
            std::cout << "[PASS] Piperidine rejection -> " << r.error.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Piperidine rejection -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 30: 1,2,4-Triazole (3 N, 5-ring)
        int m = indigoLoadMoleculeFromString("c1nc[nH]n1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "1,2,4-triazole") {
            std::cout << "[PASS] 1,2,4-Triazole (c1nc[nH]n1) -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] 1,2,4-Triazole -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 30: 1,3,5-Triazine (3 N, 6-ring, fully symmetric)
        int m = indigoLoadMoleculeFromString("c1ncncn1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "1,3,5-triazine") {
            std::cout << "[PASS] 1,3,5-Triazine (c1ncncn1) -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] 1,3,5-Triazine -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 30: Tetrazole (4 N, 5-ring)
        int m = indigoLoadMoleculeFromString("c1nnn[nH]1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "1,2,3,4-tetrazole") {
            std::cout << "[PASS] Tetrazole (c1nnn[nH]1) -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Tetrazole -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 30: Mixed-element 1,3,4-Oxadiazole (1 O, 2 N, 5-ring)
        int m = indigoLoadMoleculeFromString("o1cnnc1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "1,3,4-oxadiazole") {
            std::cout << "[PASS] 1,3,4-Oxadiazole (o1cnnc1) -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] 1,3,4-Oxadiazole -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 30: Heteroatom-locant tie-break - 1,4-Oxazine (1 O, 1 N, 6-ring)
        int m = indigoLoadMoleculeFromString("c1nccoc1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "1,4-oxazine") {
            std::cout << "[PASS] 1,4-Oxazine tie-break (c1nccoc1) -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] 1,4-Oxazine tie-break -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 30: Substituted general-heterocycle (3-methyl-1,2,4-triazole)
        int m = indigoLoadMoleculeFromString("n1c(C)ncn1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "3-methyl-1,2,4-triazole") {
            std::cout << "[PASS] 3-Methyl-1,2,4-triazole (n1c(C)ncn1) -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] 3-Methyl-1,2,4-triazole -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 30: Non-O/S/N element rejection (P in ring)
        int m = indigoLoadMoleculeFromString("c1c[nH]cp1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!r.success) {
            std::cout << "[PASS] Non-O/S/N heterocycle rejection -> " << r.error.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Non-O/S/N heterocycle rejection -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 30: Ring-size rejection (7-ring with 3 N's)
        int m = indigoLoadMoleculeFromString("c1nncccn1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!r.success && r.error == "Heterocycles other than furan, thiophene, pyrrole, pyridine, pyrazole, oxazole, isoxazole, thiazole, isothiazole, imidazole, pyridazine, pyrimidine, and pyrazine are not supported in Phase 2.") {
            std::cout << "[PASS] 7-membered heterocycle rejection -> " << r.error.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] 7-membered heterocycle rejection -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 30: Saturated heterocycle rejection (1,2,4-triazolidine)
        int m = indigoLoadMoleculeFromString("C1NCNN1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!r.success && r.error == "Saturated or partially unsaturated heterocycles are not yet supported; only the fully aromatic (maximally unsaturated) forms are supported in this phase.") {
            std::cout << "[PASS] Saturated general heterocycle rejection -> " << r.error.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Saturated general heterocycle rejection -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }

    {
        // Phase 30: Imidazole retained name regression test
        int m = indigoLoadMoleculeFromString("c1c[nH]cn1");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (r.success && r.name == "imidazole") {
            std::cout << "[PASS] Imidazole retained name non-regression -> " << r.name.toStdString() << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] Imidazole retained name non-regression -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
        }
    }

    {
        // Phase 31: 13 Benzo-fused heterobicyclic ring systems and rejections
        struct Phase31Test {
            std::string smiles;
            std::string expectedName;
            bool shouldFail;
            std::string expectedError;
        };

        std::vector<Phase31Test> p31Tests = {
            {"c1ccc2[nH]ccc2c1", "indole", false, ""},
            {"c1ccc2c[nH]cc2c1", "isoindole", false, ""},
            {"c1ccc2occc2c1", "benzofuran", false, ""},
            {"c1ccc2cocc2c1", "isobenzofuran", false, ""},
            {"c1ccc2sccc2c1", "benzothiophene", false, ""},
            {"c1ccc2cscc2c1", "isobenzothiophene", false, ""},
            {"c1ccc2[nH]cnc2c1", "benzimidazole", false, ""},
            {"c1ccc2ncccc2c1", "quinoline", false, ""},
            {"c1ccc2cnccc2c1", "isoquinoline", false, ""},
            {"c1ccc2ncncc2c1", "quinazoline", false, ""},
            {"c1ccc2nnccc2c1", "cinnoline", false, ""},
            {"c1ccc2cnncc2c1", "phthalazine", false, ""},
            {"c1ccc2nccnc2c1", "quinoxaline", false, ""},
            // Rejection regressions (substituted indole tested in Phase 37)
            {"c1ccc2ncoc2c1", "", true, "Fused ring systems other than naphthalene are not supported in this phase."},
            {"c1ccc2nn[nH]c2c1", "", true, "Fused ring systems other than naphthalene are not supported in this phase."}
        };

        for (const auto &t : p31Tests) {
            int m = indigoLoadMoleculeFromString(t.smiles.c_str());
            IupacResult r = IupacNamer::generateName(m);
            indigoFree(m);
            if (!t.shouldFail) {
                if (r.success && r.name == QString::fromStdString(t.expectedName)) {
                    std::cout << "[PASS] Phase 31 " << t.expectedName << " (" << t.smiles << ") -> " << r.name.toStdString() << "\n";
                    passed++;
                } else {
                    std::cout << "[FAIL] Phase 31 " << t.expectedName << " (" << t.smiles << ") -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
                    failed++;
                }
            } else {
                if (!r.success && r.error == QString::fromStdString(t.expectedError)) {
                    std::cout << "[PASS] Phase 31 rejection (" << t.smiles << ") -> " << r.error.toStdString() << "\n";
                    passed++;
                } else {
                    std::cout << "[FAIL] Phase 31 rejection (" << t.smiles << ") -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
                    failed++;
                }
            }
        }
    }

    {
        // Phase 32: Purine (hetero-hetero bicyclic system) and rejections
        struct Phase32Test {
            std::string smiles;
            std::string expectedName;
            bool shouldFail;
            std::string expectedError;
        };

        std::vector<Phase32Test> p32Tests = {
            {"c1ncnc2[nH]cnc12", "purine", false, ""},
            {"c1ncc2[nH]cnc2n1", "purine", false, ""},
            // Rejection: substituted purine
            {"Cc1ncnc2[nH]cnc12", "", true, "Fused ring systems other than naphthalene are not supported in this phase."},
            // Rejection: non-purine hetero-hetero fusion (furo[3,2-b]pyridine)
            {"c1cc2cccnc2o1", "", true, "Fused ring systems other than naphthalene are not supported in this phase."},
            // Rejection: non-purine hetero-hetero fusion (thieno[2,3-d]imidazole)
            {"c1csc2[nH]cnc12", "", true, "Fused ring systems other than naphthalene are not supported in this phase."}
        };

        for (const auto &t : p32Tests) {
            int m = indigoLoadMoleculeFromString(t.smiles.c_str());
            IupacResult r = IupacNamer::generateName(m);
            indigoFree(m);
            if (!t.shouldFail) {
                if (r.success && r.name == QString::fromStdString(t.expectedName)) {
                    std::cout << "[PASS] Phase 32 " << t.expectedName << " (" << t.smiles << ") -> " << r.name.toStdString() << "\n";
                    passed++;
                } else {
                    std::cout << "[FAIL] Phase 32 " << t.expectedName << " (" << t.smiles << ") -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
                    failed++;
                }
            } else {
                if (!r.success && r.error == QString::fromStdString(t.expectedError)) {
                    std::cout << "[PASS] Phase 32 rejection (" << t.smiles << ") -> " << r.error.toStdString() << "\n";
                    passed++;
                } else {
                    std::cout << "[FAIL] Phase 32 rejection (" << t.smiles << ") -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
                    failed++;
                }
            }
        }
    }

    {
        // Phase 33: Pentalene (5-5 fused all-carbon bicyclic, fully conjugated) and rejections
        struct Phase33Test {
            std::string smiles;
            std::string expectedName;
            bool shouldFail;
            std::string expectedError;
        };

        std::vector<Phase33Test> p33Tests = {
            {"C1=CC=C2C=CC=C12", "pentalene", false, ""},
            // Rejection: substituted pentalene
            {"CC1=CC=C2C=CC=C12", "", true, "Fused ring systems other than naphthalene are not supported in this phase."},
            // Rejection: saturated 5-5 fused bicyclic still yields von Baeyer bicyclo[3.3.0]octane from Phase 27
            {"C1CCC2CCCC12", "bicyclo[3.3.0]octane", false, ""}
        };

        for (const auto &t : p33Tests) {
            int m = indigoLoadMoleculeFromString(t.smiles.c_str());
            IupacResult r = IupacNamer::generateName(m);
            indigoFree(m);
            if (!t.shouldFail) {
                if (r.success && r.name == QString::fromStdString(t.expectedName)) {
                    std::cout << "[PASS] Phase 33 " << t.expectedName << " (" << t.smiles << ") -> " << r.name.toStdString() << "\n";
                    passed++;
                } else {
                    std::cout << "[FAIL] Phase 33 " << t.expectedName << " (" << t.smiles << ") -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
                    failed++;
                }
            } else {
                if (!r.success && r.error == QString::fromStdString(t.expectedError)) {
                    std::cout << "[PASS] Phase 33 rejection (" << t.smiles << ") -> " << r.error.toStdString() << "\n";
                    passed++;
                } else {
                    std::cout << "[FAIL] Phase 33 rejection (" << t.smiles << ") -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
                    failed++;
                }
            }
        }
    }

    // Phase 34: 3H-Pyrrolizine (5-5 fused bicyclic with bridgehead nitrogen)
    {
        struct Phase34Test {
            std::string smiles;
            std::string expectedName;
            bool shouldFail;
            std::string expectedError;
        };

        std::vector<Phase34Test> p34Tests = {
            // 3H-Pyrrolizine success cases (candidate SMILES)
            {"C1=CN2C=CC=C2C1", "3H-pyrrolizine", false, ""},
            {"C1=CC2=CC=CN2C1", "3H-pyrrolizine", false, ""},
            // Rejection: substituted pyrrolizine
            {"CC1=CN2C=CC=C2C1", "", true, "Fused ring systems other than naphthalene are not supported in this phase."},
            // Rejection: 5-5 fused bicyclic with nitrogen NOT at bridgehead position
            {"C1=C[NH]C2=C1CCC2", "", true, "Fused ring systems other than naphthalene are not supported in this phase."}
        };

        for (const auto &t : p34Tests) {
            int m = indigoLoadMoleculeFromString(t.smiles.c_str());
            IupacResult r = IupacNamer::generateName(m);
            indigoFree(m);
            if (!t.shouldFail) {
                if (r.success && r.name == QString::fromStdString(t.expectedName)) {
                    std::cout << "[PASS] Phase 34 " << t.expectedName << " (" << t.smiles << ") -> " << r.name.toStdString() << "\n";
                    passed++;
                } else {
                    std::cout << "[FAIL] Phase 34 " << t.expectedName << " (" << t.smiles << ") -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
                    failed++;
                }
            } else {
                if (!r.success && r.error == QString::fromStdString(t.expectedError)) {
                    std::cout << "[PASS] Phase 34 rejection (" << t.smiles << ") -> " << r.error.toStdString() << "\n";
                    passed++;
                } else {
                    std::cout << "[FAIL] Phase 34 rejection (" << t.smiles << ") -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
                    failed++;
                }
            }
        }
    }

    // --- Phase 35 Tests: General Two-Heterocycle Fusion Nomenclature ---
    {
        struct Phase35Test {
            std::string smiles;
            std::string expectedName;
            bool shouldFail;
            std::string expectedError;
        };

        std::vector<Phase35Test> p35Tests = {
            // Different-ring-type success 1: furo[3,2-b]pyridine
            {"C1=CC2=C(C=CO2)N=C1", "furo[3,2-b]pyridine", false, ""},
            // Different-ring-type success 2: pyrido[2,3-d]pyrimidine
            {"C1=CC2=CN=CN=C2N=C1", "pyrido[2,3-d]pyrimidine", false, ""},
            // Same-ring-type success: thieno[3,2-b]thiophene
            {"C1=CSC2=C1SC=C2", "thieno[3,2-b]thiophene", false, ""},
            // Rejection: substituted target compound
            {"CC1=CC2=C(C=CO2)N=C1", "", true, "Fused ring systems other than naphthalene are not supported in this phase."},
            // Rejection: fusion involving excluded pyrrole ring (furo-pyrrole is now supported in Phase 38!)
            {"c1cc2occc2[nH]1", "4H-furo[3,2-b]pyrrole", false, ""}
        };

        for (const auto &t : p35Tests) {
            int m = indigoLoadMoleculeFromString(t.smiles.c_str());
            IupacResult r = IupacNamer::generateName(m);
            indigoFree(m);
            if (!t.shouldFail) {
                if (r.success && r.name == QString::fromStdString(t.expectedName)) {
                    std::cout << "[PASS] Phase 35 " << t.expectedName << " (" << t.smiles << ") -> " << r.name.toStdString() << "\n";
                    passed++;
                } else {
                    std::cout << "[FAIL] Phase 35 " << t.expectedName << " (" << t.smiles << ") -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
                    failed++;
                }
            } else {
                if (!r.success && r.error == QString::fromStdString(t.expectedError)) {
                    std::cout << "[PASS] Phase 35 rejection (" << t.smiles << ") -> " << r.error.toStdString() << "\n";
                    passed++;
                } else {
                    std::cout << "[FAIL] Phase 35 rejection (" << t.smiles << ") -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
                    failed++;
                }
            }
        }
    }

    // --- Phase 36: Standalone Peripheral Numbering Infrastructure ---
    {
        // 1. Indole direct peripheral numbering test
        int mIndole = indigoLoadMoleculeFromString("c1ccc2[nH]ccc2c1");
        auto indoleLocants = computePeripheralNumberingForMol(mIndole);
        indigoFree(mIndole);

        if (indoleLocants.size() == 9) {
            std::set<QString> expectedSet = {"1", "2", "3", "3a", "4", "5", "6", "7", "7a"};
            std::set<QString> actualSet;
            for (const auto &kv : indoleLocants) {
                actualSet.insert(kv.second);
            }

            bool has1 = false, has3a = false, has7a = false;
            for (const auto &kv : indoleLocants) {
                if (kv.second == "1") has1 = true;
                if (kv.second == "3a") has3a = true;
                if (kv.second == "7a") has7a = true;
            }

            if (actualSet == expectedSet && has1 && has3a && has7a) {
                std::cout << "[PASS] Phase 36 indole peripheral numbering correct (N1, 3a, 7a)\n";
                passed++;
            } else {
                std::cout << "[FAIL] Phase 36 indole peripheral numbering mismatch\n";
                failed++;
            }
        } else {
            std::cout << "[FAIL] Phase 36 indole peripheral numbering count mismatch (got " << indoleLocants.size() << ", expected 9)\n";
            failed++;
        }

        // 2. Quinoline direct peripheral numbering test
        int mQuin = indigoLoadMoleculeFromString("c1ccc2ncccc2c1");
        auto quinLocants = computePeripheralNumberingForMol(mQuin);
        indigoFree(mQuin);

        if (quinLocants.size() == 10) {
            std::set<QString> expectedSet = {"1", "2", "3", "4", "4a", "5", "6", "7", "8", "8a"};
            std::set<QString> actualSet;
            for (const auto &kv : quinLocants) {
                actualSet.insert(kv.second);
            }

            bool has1 = false, has4a = false, has8a = false;
            for (const auto &kv : quinLocants) {
                if (kv.second == "1") has1 = true;
                if (kv.second == "4a") has4a = true;
                if (kv.second == "8a") has8a = true;
            }

            if (actualSet == expectedSet && has1 && has4a && has8a) {
                std::cout << "[PASS] Phase 36 quinoline peripheral numbering correct (N1, 4a, 8a)\n";
                passed++;
            } else {
                std::cout << "[FAIL] Phase 36 quinoline peripheral numbering mismatch\n";
                failed++;
            }
        } else {
            std::cout << "[FAIL] Phase 36 quinoline peripheral numbering count mismatch (got " << quinLocants.size() << ", expected 10)\n";
            failed++;
        }

        // 3. Purine exception test: naive general peripheral numbering produces 'a'-suffixed locants,
        // which does NOT match purine's fixed retained numbering (N1, C2, N3, C4, C5, C6, N7, C8, N9 with plain bridgeheads C4 & C5).
        int mPurine = indigoLoadMoleculeFromString("c1ncnc2[nH]cnc12");
        auto purineLocants = computePeripheralNumberingForMol(mPurine);
        indigoFree(mPurine);

        bool hasSuffixedLocants = false;
        for (const auto &kv : purineLocants) {
            if (kv.second.endsWith("a")) {
                hasSuffixedLocants = true;
                break;
            }
        }
        if (purineLocants.size() == 9 && hasSuffixedLocants) {
            std::cout << "[PASS] Phase 36 purine exception captured: general numbering produces 'a'-suffixed locants, demonstrating mismatch with purine's fixed plain-integer scheme\n";
            passed++;
        } else {
            std::cout << "[FAIL] Phase 36 purine exception test failed\n";
            failed++;
        }
    }

    // --- Phase 37: Substituted Benzo-fused Heterobicyclic Ring Systems ---
    {
        struct Phase37Test {
            std::string smiles;
            std::string expectedName;
            bool shouldFail;
            std::string expectedError;
        };

        std::vector<Phase37Test> p37Tests = {
            // 1. Substituted Indole: 5-methylindole
            {"Cc1ccc2[nH]ccc2c1", "5-methylindole", false, ""},
            // 2. Substituted Isoindole: 5-methylisoindole & 1-methylisoindole mirror pair
            {"Cc1ccc2c[nH]cc2c1", "5-methylisoindole", false, ""},
            {"Cc1c2ccccc2c[nH]1", "1-methylisoindole", false, ""},
            {"Cc1[nH]cc2ccccc12", "1-methylisoindole", false, ""},
            // 3. Substituted Benzofuran: 5-methylbenzofuran
            {"Cc1ccc2occc2c1", "5-methylbenzofuran", false, ""},
            // 4. Substituted Isobenzofuran: 5-methylisobenzofuran & 1-methylisobenzofuran mirror pair
            {"Cc1ccc2cocc2c1", "5-methylisobenzofuran", false, ""},
            {"Cc1c2ccccc2co1", "1-methylisobenzofuran", false, ""},
            {"Cc1occ2ccccc12", "1-methylisobenzofuran", false, ""},
            // 5. Substituted Benzothiophene: 5-methylbenzothiophene
            {"Cc1ccc2sccc2c1", "5-methylbenzothiophene", false, ""},
            // 6. Substituted Isobenzothiophene: 5-methylisobenzothiophene & 1-methylisobenzothiophene mirror pair
            {"Cc1ccc2cscc2c1", "5-methylisobenzothiophene", false, ""},
            {"Cc1c2ccccc2cs1", "1-methylisobenzothiophene", false, ""},
            {"Cc1scc2ccccc12", "1-methylisobenzothiophene", false, ""},
            // 7. Substituted Benzimidazole: 5-methylbenzimidazole tautomeric pair
            {"Cc1ccc2[nH]cnc2c1", "5-methylbenzimidazole", false, ""},
            {"Cc1ccc2nc[nH]c2c1", "5-methylbenzimidazole", false, ""},
            // 8. Substituted Quinoline: 3-chloroquinoline
            {"c1ccc2ncc(Cl)cc2c1", "3-chloroquinoline", false, ""},
            // 9. Substituted Isoquinoline: 3-methylisoquinoline
            {"Cc1ncc2ccccc2c1", "3-methylisoquinoline", false, ""},
            // 10. Substituted Quinazoline: 2-methylquinazoline
            {"Cc1nc2ccccc2cn1", "2-methylquinazoline", false, ""},
            // 11. Substituted Cinnoline: 3-methylcinnoline
            {"Cc1nnc2ccccc2c1", "3-methylcinnoline", false, ""},
            // 12. Substituted Phthalazine: 1-methylphthalazine & mirror pair
            {"Cc1c2ccccc2cnn1", "1-methylphthalazine", false, ""},
            {"Cc1nncc2ccccc12", "1-methylphthalazine", false, ""},
            // 13. Substituted Quinoxaline: 2-methylquinoxaline & mirror pair
            {"Cc1nc2ccccc2nc1", "2-methylquinoxaline", false, ""},
            {"Cc1cnc2ccccc2n1", "2-methylquinoxaline", false, ""},

            // Multi-substituent test case: 3,5-dimethylindole
            {"Cc1ccc2[nH]cc(C)c2c1", "3,5-dimethylindole", false, ""},

            // Rejection test 1: Fusion-atom substitution (substituent directly on bridgehead)
            {"CC12C=CC=CC1=NC=CC2", "", true, "Fused ring systems other than naphthalene are not supported in this phase."},

            // Rejection test 2: Out-of-scope substituted Purine (Phase 32 keeps rejecting)
            {"Cc1ncnc2[nH]cnc12", "", true, "Fused ring systems other than naphthalene are not supported in this phase."},

            // Rejection test 3: Out-of-scope substituted Pentalene (Phase 33 keeps rejecting)
            {"CC1=CC=C2C=CC=C12", "", true, "Fused ring systems other than naphthalene are not supported in this phase."},

            // Rejection test 4: Out-of-scope substituted 3H-pyrrolizine (Phase 34 keeps rejecting)
            {"CC1=CN2C=CC=C2C1", "", true, "Fused ring systems other than naphthalene are not supported in this phase."},

            // Rejection test 5: Out-of-scope substituted General Fusion (Phase 35 keeps rejecting)
            {"CC1=CC2=C(C=CO2)N=C1", "", true, "Fused ring systems other than naphthalene are not supported in this phase."}
        };

        for (const auto &t : p37Tests) {
            int m = indigoLoadMoleculeFromString(t.smiles.c_str());
            IupacResult r = IupacNamer::generateName(m);
            indigoFree(m);
            if (!t.shouldFail) {
                if (r.success && r.name == QString::fromStdString(t.expectedName)) {
                    std::cout << "[PASS] Phase 37 " << t.expectedName << " (" << t.smiles << ") -> " << r.name.toStdString() << "\n";
                    passed++;
                } else {
                    std::cout << "[FAIL] Phase 37 " << t.expectedName << " (" << t.smiles << ") -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
                    failed++;
                }
            } else {
                if (!r.success && r.error == QString::fromStdString(t.expectedError)) {
                    std::cout << "[PASS] Phase 37 rejection (" << t.smiles << ") -> " << r.error.toStdString() << "\n";
                    passed++;
                } else {
                    std::cout << "[FAIL] Phase 37 rejection (" << t.smiles << ") -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
                    failed++;
                }
            }
        }
    }
    // --- Phase 38: Extended General Two-Heterocycle Fusion (Oxazole family + Indicated Hydrogen) ---
    {
        struct Phase38Test {
            std::string smiles;
            std::string expectedName;
            bool shouldFail;
            std::string expectedError;
        };

        std::vector<Phase38Test> p38Tests = {
            // 1. Oxazole family (thiazole + pyridine): thiazolo[5,4-b]pyridine
            {"C1=CC2=C(N=C1)SC=N2", "thiazolo[5,4-b]pyridine", false, ""},

            // 2. Oxazole family (oxazole + pyridine): oxazolo[5,4-b]pyridine
            {"C1=CC2=C(N=C1)OC=N2", "oxazolo[5,4-b]pyridine", false, ""},

            // 3. Pyrrole + pyridine: 1H-pyrrolo[2,3-b]pyridine (7-azaindole)
            {"C1=CC2=C(NC=C2)N=C1", "1H-pyrrolo[2,3-b]pyridine", false, ""},

            // 4. Imidazole + pyridine: 1H-imidazo[4,5-b]pyridine
            {"C1=CC2=C(N=CN2)N=C1", "1H-imidazo[4,5-b]pyridine", false, ""},

            // 5. Pyrazole + pyridine: 1H-pyrazolo[4,5-b]pyridine
            {"C1=CC2=C(N=C1)C=NN2", "1H-pyrazolo[4,5-b]pyridine", false, ""},

            // Rejection 1: Substituted general fusion compound (3-methyl-1H-pyrrolo[2,3-b]pyridine)
            {"CC1=CNC2=C1C=CC=N2", "", true, "Fused ring systems other than naphthalene are not supported in this phase."},

            // Rejection 2: Out-of-scope Bridgehead-N fusion (imidazo[1,2-a]pyridine)
            {"C1=CC2=NC=CN2C=C1", "", true, "Fused ring systems other than naphthalene are not supported in this phase."}
        };

        for (const auto &t : p38Tests) {
            int m = indigoLoadMoleculeFromString(t.smiles.c_str());
            IupacResult r = IupacNamer::generateName(m);
            indigoFree(m);
            if (!t.shouldFail) {
                if (r.success && r.name == QString::fromStdString(t.expectedName)) {
                    std::cout << "[PASS] Phase 38 " << t.expectedName << " (" << t.smiles << ") -> " << r.name.toStdString() << "\n";
                    passed++;
                } else {
                    std::cout << "[FAIL] Phase 38 " << t.expectedName << " (" << t.smiles << ") -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
                    failed++;
                }
            } else {
                if (!r.success && r.error == QString::fromStdString(t.expectedError)) {
                    std::cout << "[PASS] Phase 38 rejection (" << t.smiles << ") -> " << r.error.toStdString() << "\n";
                    passed++;
                } else {
                    std::cout << "[FAIL] Phase 38 rejection (" << t.smiles << ") -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
                    failed++;
                }
            }
        }
    }

    // --- Phase 44: 3-Ring Fusion ---
    {
        struct Phase44Test {
            std::string smiles;
            std::string expectedName;
            bool shouldFail;
            std::string expectedError;
        };

        std::vector<Phase44Test> p44Tests = {
            {"o1ccc2nc3ccsc3cc12", "furo[3,2-b]thieno[2,3-e]pyridine", false, ""},
            {"o1ccc2nc3ccsc3nc12", "furo[3,2-b]thieno[2,3-e]pyrazine", false, ""}
        };

        for (const auto &t : p44Tests) {
            int m = indigoLoadMoleculeFromString(t.smiles.c_str());
            IupacResult r = IupacNamer::generateName(m);
            indigoFree(m);
            if (!t.shouldFail) {
                if (r.success && r.name == QString::fromStdString(t.expectedName)) {
                    std::cout << "[PASS] Phase 44 " << t.expectedName << " (" << t.smiles << ") -> " << r.name.toStdString() << "\n";
                    passed++;
                } else {
                    std::cout << "[FAIL] Phase 44 " << t.expectedName << " (" << t.smiles << ") -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
                    failed++;
                }
            } else {
                if (!r.success && r.error == QString::fromStdString(t.expectedError)) {
                    std::cout << "[PASS] Phase 44 rejection (" << t.smiles << ") -> " << r.error.toStdString() << "\n";
                    passed++;
                } else {
                    std::cout << "[FAIL] Phase 44 rejection (" << t.smiles << ") -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
                    failed++;
                }
            }
        }
    }

    // --- Phase 40: FR-2.3 base-component seniority tie-breaks (e) variety, (f) alternate
    // order, (h) own-numbering heteroatom locants ---
    {
        struct Phase40Test {
            std::string smiles;
            std::string expectedName;
            bool shouldFail;
            std::string expectedError;
        };

        std::vector<Phase40Test> p40Tests = {
            // Oxazole (N,O) vs thiazole (N,S): tied through rules (a) N-rank, (c) size=5,
            // (d) heteroatom count=2, and (e) variety=2 each. Resolved by rule (f):
            // alternate seniority order F,Cl,Br,I,O,S,...,N,... ranks O above S, so oxazole
            // (whose top-ranked heteroatom is O) wins as base component over thiazole
            // (whose top-ranked heteroatom is S). Matches the official FR-2.3 rule (f)
            // worked example pattern ("S,N preferred to Se,N").
            {"O1C=NC2=C1SC=N2", "thiazolo[4,5-d]oxazole", false, ""},

            // Pyridazine (N,N @1,2) vs pyrazine (N,N @1,4): tied through rules (a) N-rank,
            // (c) size=6, (d) heteroatom count=2, (e) variety=1 (both all-N), and (f)
            // top-alt-rank (both N, no O/S present). Resolved by rule (h): each ring's own
            // independent numbering gives pyridazine the lower heteroatom locant set {1,2}
            // vs pyrazine's {1,4}, so pyridazine wins as base component. Exact match to the
            // official FR-2.3 rule (h) worked example: "pyrazino[2,3-d]pyridazine".
            {"C=12C=NN=CC1N=CC=N2", "pyrazino[2,3-d]pyridazine", false, ""}
        };

        for (const auto &t : p40Tests) {
            int m = indigoLoadMoleculeFromString(t.smiles.c_str());
            IupacResult r = IupacNamer::generateName(m);
            indigoFree(m);
            if (!t.shouldFail) {
                if (r.success && r.name == QString::fromStdString(t.expectedName)) {
                    std::cout << "[PASS] Phase 40 " << t.expectedName << " (" << t.smiles << ") -> " << r.name.toStdString() << "\n";
                    passed++;
                } else {
                    std::cout << "[FAIL] Phase 40 " << t.expectedName << " (" << t.smiles << ") -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
                    failed++;
                }
            } else {
                if (!r.success && r.error == QString::fromStdString(t.expectedError)) {
                    std::cout << "[PASS] Phase 40 rejection (" << t.smiles << ") -> " << r.error.toStdString() << "\n";
                    passed++;
                } else {
                    std::cout << "[FAIL] Phase 40 rejection (" << t.smiles << ") -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
                    failed++;
                }
            }
        }
    }

    // Phase 39 Generalized Ring-as-Substituent Tests
    {
        struct P39TestCase {
            std::string smiles;
            std::string expectedName;
            bool shouldFail;
            std::string expectedError;
        };

        std::vector<P39TestCase> p39Tests = {
            // 1. Motivating real-world complex molecule:
            {"C1CN(C)CCC1OC1C=CC(CC(NC(C2C=CC(OC)=CC=2)=O)C(OC)=O)=CC=1",
             "methyl 2-(4-methoxybenzamido)-3-{4-[(1-methylpiperidin-4-yl)oxy]phenyl}propanoate",
             false, ""},

            // 2. Three independent ring substituents on chain (e.g. phenyl, furyl, cyclohexyl):
            {"c1ccccc1C(c2ccoc2)C3CCCCC3", "cyclohexyl(furan-3-yl)phenylmethane", false, ""},

            // 3. Ring substituent bearing own substituent (e.g. 4-chlorophenyl):
            {"Clc1ccc(CC(=O)O)cc1", "2-(4-chlorophenyl)ethanoic acid", false, ""},

            // 4. Nested heteroatom ring substituent (e.g. furan ring via ether oxygen on phenyl):
            {"c1ccoc1Oc2ccc(CC(=O)O)cc2", "2-{4-[(furan-2-yl)oxy]phenyl}ethanoic acid", false, ""},

            // 5. Rejection: Fused/bridged/spiro substituent on acyclic chain:
            {"c1ccc2c(c1)CCCC2CC(=O)O", "", true, "Fused ring systems other than naphthalene are not supported in this phase."},

            // 6. Rejection: Fused/bridged/spiro parent > limit unaffected:
            {"C1C2CC3CC1CC(C2)C3", "", true, "Fused, bridged, spiro, or multiple ring systems are not supported in Phase 2."}
        };

        for (const auto &t : p39Tests) {
            int m = indigoLoadMoleculeFromString(t.smiles.c_str());
            IupacResult r = IupacNamer::generateName(m);
            indigoFree(m);
            if (!t.shouldFail) {
                if (r.success && r.name == QString::fromStdString(t.expectedName)) {
                    std::cout << "[PASS] Phase 39 " << t.expectedName << " (" << t.smiles << ") -> " << r.name.toStdString() << "\n";
                    passed++;
                } else {
                    std::cout << "[FAIL] Phase 39 " << t.expectedName << " (" << t.smiles << ") -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
                    failed++;
                }
            } else {
                if (!r.success && r.error == QString::fromStdString(t.expectedError)) {
                    std::cout << "[PASS] Phase 39 rejection (" << t.smiles << ") -> " << r.error.toStdString() << "\n";
                    passed++;
                } else {
                    std::cout << "[FAIL] Phase 39 rejection (" << t.smiles << ") -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
                    failed++;
                }
            }
        }
    }

    indigoReleaseSessionId(sid);

    std::cout << "\nSummary: " << passed << " passed, " << failed << " failed.\n";
    return (failed == 0) ? 0 : 1;
}


