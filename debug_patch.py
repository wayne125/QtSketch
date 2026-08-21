import re
with open("src/app/IupacNamer.cpp", "r", encoding="utf-8") as f:
    content = f.read()

content = re.sub(r'int ringCount = indigoCountSSSR\(mol\);',
                 'int ringCount = indigoCountSSSR(mol);\n    // DEBUG\n    std::cout << "MOL " << indigoCanonicalSmiles(mol) << " ringCount=" << ringCount << "\\n";',
                 content, count=1)
content = re.sub(r'// PHASE 59: Guard for ringCount == 2',
                 'std::cout << "allSSSRNodes size=" << allSSSRNodes.size() << "\\n";\n    // PHASE 59: Guard for ringCount == 2',
                 content, count=1)

with open("src/app/IupacNamer.cpp", "w", encoding="utf-8") as f:
    f.write(content)
