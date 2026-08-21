import re
with open("src/app/IupacNamer.cpp", "r", encoding="utf-8") as f:
    content = f.read()

index = content.find("    // --- Phase 28:")
if index != -1:
    before = content[:index]
    after = content[index:]
    # Find the last return {true, fullName, ""}; in 'before'
    last_return = before.rfind('return {true, fullName, ""};')
    if last_return != -1:
        new_before = before[:last_return] + 'fullName.replace("1-azabicyclo[2.2.2]octane", "quinuclidine");\n                                        return {true, fullName, ""};' + before[last_return + len('return {true, fullName, ""};'):]
        with open("src/app/IupacNamer.cpp", "w", encoding="utf-8") as f:
            f.write(new_before + after)
        print("Patched!")
