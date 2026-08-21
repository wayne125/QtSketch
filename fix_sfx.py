import re

with open('src/app/IupacNamer.cpp', 'r') as f:
    content = f.read()

sfx_old = """else if (winningType == GroupType::AMIDE) sfx = (pCount == 1) ? "carboxamide" : "dicarboxamide";\n                else if (winningType == GroupType::NITRILE)"""
sfx_new = """else if (winningType == GroupType::AMIDE) sfx = (pCount == 1) ? "carboxamide" : "dicarboxamide";\n                else if (winningType == GroupType::HYDRAZIDE) sfx = (pCount == 1) ? "carbohydrazide" : "dicarbohydrazide";\n                else if (winningType == GroupType::NITRILE)"""
content = content.replace(sfx_old, sfx_new)

sfx2_old = """else if (winningType == GroupType::AMIDE) sfx = (pCount == 1) ? "carboxamide" : "dicarboxamide";\n            else if (winningType == GroupType::NITRILE)"""
sfx2_new = """else if (winningType == GroupType::AMIDE) sfx = (pCount == 1) ? "carboxamide" : "dicarboxamide";\n            else if (winningType == GroupType::HYDRAZIDE) sfx = (pCount == 1) ? "carbohydrazide" : "dicarbohydrazide";\n            else if (winningType == GroupType::NITRILE)"""
content = content.replace(sfx2_old, sfx2_new)

with open('src/app/IupacNamer.cpp', 'w') as f:
    f.write(content)
