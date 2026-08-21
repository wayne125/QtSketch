import os

with open("tests/iupac_namer_test.cpp", "r", encoding="utf-8") as f:
    content = f.read()

patch = """
    // Bonus Quinuclidine test
    {
        int m = indigoLoadMoleculeFromString("C1CN2CCC1CC2");
        IupacResult r = IupacNamer::generateName(m);
        std::cout << "Quinuclidine test: " << r.name.toStdString() << " success=" << r.success << "\\n";
    }
"""

content = content.replace("Summary: ", patch + "Summary: ")

with open("tests/iupac_namer_test.cpp", "w", encoding="utf-8") as f:
    f.write(content)

os.system("mingw32-make.exe -j4 sketch iupac_namer_test -C build")
os.system(".\\build\\iupac_namer_test.exe > quinuclidine_out.txt")
