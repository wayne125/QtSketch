with open("src/app/IupacNamer.cpp", "r", encoding="utf-8") as f:
    content = f.read()

content = content.replace("numericalMultiplier(", "multiPrefix(")

with open("src/app/IupacNamer.cpp", "w", encoding="utf-8") as f:
    f.write(content)
print("Replaced multiplier func.")
