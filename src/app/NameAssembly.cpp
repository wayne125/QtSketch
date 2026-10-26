#include "NameAssembly.h"

// Chain length root prefixes C1..C100 (IUPAC P-21.2.1 / P-14.2.1)
QString chainRoot(int len) {
    static const QString roots[] = {
        "", "meth", "eth", "prop", "but", "pent", "hex", "hept", "oct", "non", "dec",
        "undec", "dodec", "tridec", "tetradec", "pentadec", "hexadec", "heptadec",
        "octadec", "nonadec", "icos"
    };
    if (len >= 1 && len <= 20) return roots[len];
    if (len == 21) return "henicos";
    if (len >= 22 && len <= 99) {
        static const QString units[] = {
            "", "hen", "do", "tri", "tetra", "penta", "hexa", "hepta", "octa", "nona"
        };
        static const QString tens[] = {
            "", "", "cos", "triacont", "tetracont", "pentacont", "hexacont", "heptacont", "octacont", "nonacont"
        };
        int u = len % 10;
        int t = len / 10;
        return units[u] + tens[t];
    }
    if (len == 100) return "hect";
    return "";
}

// Multiplying prefixes (2..20)
QString multiPrefix(int count) {
    static const QString prefixes[] = {
        "", "", "di", "tri", "tetra", "penta", "hexa", "hepta", "octa", "nona", "deca",
        "undeca", "dodeca", "trideca", "tetradeca", "pentadeca", "hexadeca", "heptadeca",
        "octadeca", "nonadeca", "icosa"
    };
    if (count >= 2 && count <= 20) return prefixes[count];
    return "";
}

// Helper: check if character is a vowel
bool isVowel(QChar c) {
    c = c.toLower();
    return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u');
}
