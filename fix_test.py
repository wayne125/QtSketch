with open("tests/iupac_namer_test.cpp", "r", encoding="utf-8") as f:
    content = f.read()

content = content.replace("IupacResult r = generateName(m);", "IupacResult r = IupacNamer::generateName(m);")

with open("tests/iupac_namer_test.cpp", "w", encoding="utf-8") as f:
    f.write(content)
print("Replaced generateName with IupacNamer::generateName")
