$content = Get-Content tests/iupac_namer_test.cpp -Raw

$tests = @'
    {
        // Phase XY: Acenaphthylene (ortho- and peri-fused, P-25.3.1.1.2)
        int m = indigoLoadMoleculeFromString("C1=CC2=C3C1=CC=CC3=CC=C2");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!r.success && r.error.contains("ortho- and peri-fused ring systems (P-25.3.1.1.2)")) {
            std::cout << "[PASS] Acenaphthylene rejected as P-25.3.1.1.2\n";
            passed++;
        } else {
            std::cout << "[FAIL] Acenaphthylene should reject as P-25.3.1.1.2, got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }
    {
        // Phase XY: Pyrene (three-component ortho- and peri-fused, P-25.5)
        int m = indigoLoadMoleculeFromString("C1=CC2=C3C1=CC=C4C3=C(C=C2)C=C4");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!r.success && r.error.contains("three-component ortho- and peri-fused systems (P-25.5)")) {
            std::cout << "[PASS] Pyrene rejected as P-25.5\n";
            passed++;
        } else {
            std::cout << "[FAIL] Pyrene should reject as P-25.5, got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }
    {
        // Phase XY: 1,4-methanonaphthalene (bridged fused, P-25.4)
        int m = indigoLoadMoleculeFromString("C1C2C=CC1C3=CC=CC=C23");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        if (!r.success && r.error.contains("bridged fused ring systems (P-25.4)")) {
            std::cout << "[PASS] 1,4-methanonaphthalene rejected as P-25.4\n";
            passed++;
        } else {
            std::cout << "[FAIL] 1,4-methanonaphthalene should reject as P-25.4, got success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
            failed++;
        }
    }
'@

$target = "    std::cout << "\nSummary: " << passed << " passed, " << failed << " failed.\n";"
$content = $content.Replace($target, "$tests
$target")
Set-Content tests/iupac_namer_test.cpp $content
