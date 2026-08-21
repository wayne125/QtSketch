import re
with open("src/app/IupacNamer.cpp", "r", encoding="utf-8") as f:
    content = f.read()
content = re.sub(r'return \{true, fullName, ""\};(\s*\n\s*\}\n\s*\}\n\s*\}\n\s*// --- Phase 28:)',
                 r'fullName.replace("1-azabicyclo[2.2.2]octane", "quinuclidine");\n                                        return {true, fullName, ""};\1',
                 content, count=1)
with open("src/app/IupacNamer.cpp", "w", encoding="utf-8") as f:
    f.write(content)
