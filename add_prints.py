import re
with open("patch_adamantane2.py", "r", encoding="utf-8") as f:
    content = f.read()

patch = content.replace("GroupType winningType = GroupType::NONE;", """
            std::cout << "DEBUG Phase 26: baseName=" << baseName.toStdString() << " ringCount=" << ringCount << " allSSSRNodes=" << allSSSRNodes.size() << "\\n";
            GroupType winningType = GroupType::NONE;
""")
patch = patch.replace("if (subsOk) {", """
            std::cout << "DEBUG Phase 26: subsOk=" << subsOk << " winningType=" << (int)winningType << " ringSubstituents=" << ringSubstituents.size() << "\\n";
            if (subsOk) {
""")
patch = patch.replace("if (!allMappings.empty()) {", """
                std::cout << "DEBUG Phase 26: allMappings.size()=" << allMappings.size() << "\\n";
                if (!allMappings.empty()) {
""")

with open("patch_adamantane2_debug.py", "w", encoding="utf-8") as f:
    f.write(patch)
