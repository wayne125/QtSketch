import re

with open('src/app/IupacNamer.cpp', 'r') as f:
    content = f.read()

detect_old = """                    if (isHydrazide) {
                        carbonGroup[i] = GroupType::HYDRAZIDE;
                    } else {
                        carbonGroup[i] = GroupType::AMIDE;
                    }"""

detect_new = """                    if (isHydrazide) {
                        carbonGroup[i] = GroupType::HYDRAZIDE;
                        std::cout << "Hydrazide detected on carbon " << i << "\n";
                    } else {
                        carbonGroup[i] = GroupType::AMIDE;
                        std::cout << "Amide detected on carbon " << i << "\n";
                    }"""

content = content.replace(detect_old, detect_new)

with open('src/app/IupacNamer.cpp', 'w') as f:
    f.write(content)
