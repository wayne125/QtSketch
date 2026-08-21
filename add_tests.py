import sys

def process_file(file_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        c = f.read()

    insertion = """
        // Rootless acyl carbon / multiple halogen fixes
        {"ClC(=O)Cl", "Carbonic acid halides with multiple halogens are not supported in this phase."},
        {"ClC(=O)Br", "Carbonic acid halides with multiple halogens are not supported in this phase."},
        {"NC(=O)Cl", "Carbonic/carbamic acid derivatives (rootless acyl carbons) are not supported in this phase."},
"""
    t = "    std::vector<TestCase> tests = {\n"
    if t in c:
        c = c.replace(t, t + insertion, 1)
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(c)
        print("Tests added.")
    else:
        print("Could not find tests vector.")

if __name__ == "__main__":
    process_file('tests/iupac_namer_test.cpp')
