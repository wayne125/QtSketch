import re

with open('src/app/IupacNamer.cpp', 'r') as f:
    content = f.read()

# Replace the specific block of lines in acyclic/ring/naphthalene logic:
# if (g.nodes[nei].neighbors.size() == 1) {
#     isHydrazide = true;
#     break;
# }
content = re.sub(r'if \(g\.nodes\[nei\]\.neighbors\.size\(\) == 1\) \{\s*isHydrazide = true;\s*break;\s*\}', 'isHydrazide = true; break;', content)

# And in cage logic:
# if (g.nodes[n2].neighbors.size() == 1) {
#     isHydrazide = true;
#     break;
# }
content = re.sub(r'if \(g\.nodes\[n2\]\.neighbors\.size\(\) == 1\) \{\s*isHydrazide = true;\s*break;\s*\}', 'isHydrazide = true; break;', content)

with open('src/app/IupacNamer.cpp', 'w') as f:
    f.write(content)
