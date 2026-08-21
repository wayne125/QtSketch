import sys

def process_file(file_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        c = f.read()

    orig = """        // Rootless acyl carbon / multiple halogen fixes
        {"ClC(=O)Cl", "Carbonic acid halides with multiple halogens are not supported in this phase."},
        {"ClC(=O)Br", "Carbonic acid halides with multiple halogens are not supported in this phase."},
        {"NC(=O)Cl", "Carbonic/carbamic acid derivatives (rootless acyl carbons) are not supported in this phase."},"""

    replacement = """        // Rootless acyl carbon / multiple halogen fixes
        {"ClC(=O)Cl", "", true, "Carbonic acid halides with multiple halogens are not supported in this phase."},
        {"ClC(=O)Br", "", true, "Carbonic acid halides with multiple halogens are not supported in this phase."},
        {"NC(=O)Cl", "", true, "Carbonic/carbamic acid derivatives (rootless acyl carbons) are not supported in this phase."},"""

    c = c.replace(orig, replacement)
    with open(file_path, 'w', encoding='utf-8') as f:
        f.write(c)
    print("Fixed tests.")

if __name__ == "__main__":
    process_file('tests/iupac_namer_test.cpp')
