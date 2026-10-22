#include "NameAssembly.h"

// Chain length root prefixes C1..C20
QString chainRoot(int len) {
    static const QString roots[] = {
        "", "meth", "eth", "prop", "but", "pent", "hex", "hept", "oct", "non", "dec",
        "undec", "dodec", "tridec", "tetradec", "pentadec", "hexadec", "heptadec",
        "octadec", "nonadec", "icos"
    };
    if (len >= 1 && len <= 20) return roots[len];
    return "";
}

// Multiplying prefixes
QString multiPrefix(int count) {
    static const QString prefixes[] = {
        "", "", "di", "tri", "tetra", "penta", "hexa", "hepta", "octa", "nona", "deca"
    };
    if (count >= 2 && count <= 10) return prefixes[count];
    return "";
}

// Helper: check if character is a vowel
bool isVowel(QChar c) {
    c = c.toLower();
    return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u');
}
