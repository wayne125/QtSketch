with open("tests/iupac_namer_test.cpp", "r", encoding="utf-8") as f:
    content = f.read()

anchor = "// Phase 39 Generalized Ring-as-Substituent Tests"

patch = """    // Phase 26 Adamantane and Cubane Tests
    {
        struct Phase26Test {
            std::string smiles;
            std::string expectedName;
            bool shouldFail;
            std::string expectedError;
        };
        std::vector<Phase26Test> tests = {
            // Unsubstituted
            {"C1C2CC3CC1CC(C2)C3", "adamantane", false, ""},
            {"C12C3C4C1C5C2C3C45", "cubane", false, ""},
            
            // Substituted Adamantane
            // 1-adamantanol (bridgehead)
            {"OC12CC3CC(CC(C1)C3)C2", "adamantan-1-ol", false, ""},
            // 1-chloroadamantane (bridgehead)
            {"ClC12CC3CC(CC(C1)C3)C2", "1-chloroadamantane", false, ""},
            // 2-adamantanone (bridge)
            {"O=C1C2CC3CC(C2)CC1C3", "adamantan-2-one", false, ""},
            
            // Substituted Cubane
            {"ClC12C3C4C1C5C2C3C45", "1-chlorocubane", false, ""},
            
            // Negative test: a different C10H16 isomer that is not adamantane
            // e.g., decalin
            {"C1CCC2CCCCC2C1", "decahydronaphthalene", false, ""},
        };

        for (const auto &t : tests) {
            int m = indigoLoadMoleculeFromString(t.smiles.c_str());
            IupacResult r = generateName(m);
            indigoFree(m);
            if (!t.shouldFail) {
                if (r.success && r.name == QString::fromStdString(t.expectedName)) {
                    std::cout << "[PASS] Phase 26 " << t.expectedName << " (" << t.smiles << ") -> " << r.name.toStdString() << "\\n";
                    passed++;
                } else {
                    std::cout << "[FAIL] Phase 26 " << t.expectedName << " (" << t.smiles << ") -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\\n";
                    failed++;
                }
            } else {
                if (!r.success && r.error == QString::fromStdString(t.expectedError)) {
                    std::cout << "[PASS] Phase 26 rejection (" << t.smiles << ") -> " << r.error.toStdString() << "\\n";
                    passed++;
                } else {
                    std::cout << "[FAIL] Phase 26 rejection (" << t.smiles << ") -> got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\\n";
                    failed++;
                }
            }
        }
    }

"""

if anchor in content:
    new_content = content.replace(anchor, patch + anchor)
    with open("tests/iupac_namer_test.cpp", "w", encoding="utf-8") as f:
        f.write(new_content)
    print("Patched tests successfully")
else:
    print("Anchor not found in tests!")
