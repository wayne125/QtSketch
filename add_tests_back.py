import os

with open("tests/iupac_namer_test.cpp", "r", encoding="utf-8") as f:
    content = f.read()

tests_block = """
    struct Phase26Test { std::string smiles, expected; bool shouldFail; std::string err; };
    std::vector<Phase26Test> phase26Tests = {
        {"C1C2CC3CC1CC(C2)C3", "adamantane", false, ""},
        {"C12C3C4C1C5C2C3C45", "cubane", false, ""},
        {"OC12CC3CC(CC(C1)C3)C2", "adamantan-1-ol", false, ""},
        {"ClC12CC3CC(CC(C1)C3)C2", "1-chloroadamantane", false, ""},
        {"O=C1C2CC3CC(C2)CC1C3", "adamantan-2-one", false, ""},
        {"ClC12C3C4C1C5C2C3C45", "1-chlorocubane", false, ""},
        {"C1CCC2CCCCC2C1", "bicyclo[4.4.0]decane", false, ""},
        {"C1CN2CCC1CC2", "1-azabicyclo[2.2.2]octane", false, ""}
    };
    for (const auto& t : phase26Tests) {
        int m = indigoLoadMoleculeFromString(t.smiles.c_str());
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!t.shouldFail) {
            if (r.success && r.name == QString::fromStdString(t.expected)) {
                std::cout << "[PASS] Phase 26 " << t.expected << " (" << t.smiles << ") -> " << r.name.toStdString() << "\\n";
                passed++;
            } else {
                std::cout << "[FAIL] Phase 26 " << t.expected << " (" << t.smiles << ") -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\\n";
                failed++;
            }
        } else {
            if (!r.success && (t.err.empty() || r.error.toStdString().find(t.err) != std::string::npos)) {
                std::cout << "[PASS] Phase 26 rejection (" << t.smiles << ") -> " << r.error.toStdString() << "\\n";
                passed++;
            } else {
                std::cout << "[FAIL] Phase 26 rejection (" << t.smiles << ") -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\\n";
                failed++;
            }
        }
    }
"""

# Insert before Phase 39
content = content.replace("    // --- Phase 39 Generalized Ring-as-Substituent Tests ---",
                          tests_block + "\n    // --- Phase 39 Generalized Ring-as-Substituent Tests ---")

# Fix Phase 39 adamantane rejection -> success
content = content.replace('{"C1C2CC3CC1CC(C2)C3", "", true, "Fused, bridged, spiro, or multiple ring systems are not supported in Phase 2."}',
                          '{"C1C2CC3CC1CC(C2)C3", "adamantane", false, ""}')


with open("tests/iupac_namer_test.cpp", "w", encoding="utf-8") as f:
    f.write(content)

os.system("mingw32-make.exe -j4 sketch iupac_namer_test -C build")
os.system(".\\build\\iupac_namer_test.exe > final.txt")
