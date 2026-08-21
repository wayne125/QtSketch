with open("tests/iupac_namer_test.cpp", "r", encoding="utf-8") as f:
    c = f.read()
c = c.replace('{"C12C3C4C1C5C2C3C45", "cubane", false, ""},',
              '{"C12C3C4C1C5C2C3C45", "cubane", false, ""},\n            {"C1CN2CCC1CC2", "1-azabicyclo[2.2.2]octane", false, ""},')
with open("tests/iupac_namer_test.cpp", "w", encoding="utf-8") as f:
    f.write(c)
