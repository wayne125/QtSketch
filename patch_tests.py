import sys

def apply_patch(file_path):
    with open(file_path, "r", encoding="utf-8") as f:
        content = f.read()

    old_tests = """        {"CS(=O)(=O)Cl", "methanesulfonyl chloride"},
        {"c1ccccc1S(=O)(=O)Cl", "benzenesulfonyl chloride"},
        {"CCS(=O)(=O)Br", "ethanesulfonyl bromide"},"""
    new_tests = """        {"CS(=O)(=O)Cl", "methanesulfonyl chloride"},
        {"c1ccccc1S(=O)(=O)Cl", "benzenesulfonyl chloride"},
        {"CCS(=O)(=O)Br", "ethanesulfonyl bromide"},
        {"CS(=O)Cl", "methanesulfinyl chloride"},
        {"c1ccccc1S(=O)Cl", "benzenesulfinyl chloride"},
        {"CCS(=O)Br", "ethanesulfinyl bromide"},"""

    content = content.replace(old_tests, new_tests)

    with open("tests/iupac_namer_test.cpp", "w", encoding="utf-8") as f:
        f.write(content)

if __name__ == "__main__":
    apply_patch("tests/iupac_namer_test.cpp")
