import os

with open("patch_adamantane3.py", "r", encoding="utf-8") as f:
    content = f.read()

# Fix the sfx concatenation
old_sfx_logic = """                        if (bestPrincipalLocants.size() == 1) {
                            fullName = QString("%1-%2-%3").arg(fullName).arg(bestPrincipalLocants[0]).arg(sfx);
                        } else {
                            QStringList lStrs;
                            for (int l : bestPrincipalLocants) lStrs.append(QString::number(l));
                            fullName = QString("%1-%2-%3").arg(fullName, lStrs.join(","), sfx);
                        }"""

new_sfx_logic = """                        fullName += sfx;"""

content = content.replace(old_sfx_logic, new_sfx_logic)

# Fix the prefix concatenation
old_prefix_logic = """                        fullName = prefix + "-" + fullName;"""
new_prefix_logic = """                        fullName = prefix + fullName;"""

content = content.replace(old_prefix_logic, new_prefix_logic)

with open("patch_adamantane4.py", "w", encoding="utf-8") as f:
    f.write(content)

os.system("git checkout src/app/IupacNamer.cpp")
os.system("python patch_adamantane4.py")
