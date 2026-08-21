#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

std::string multiPrefix(int n) {
    if (n == 2) return "di";
    if (n == 3) return "tri";
    if (n == 4) return "tetra";
    if (n == 5) return "penta";
    if (n == 6) return "hexa";
    if (n == 7) return "hepta";
    if (n == 8) return "octa";
    return "";
}

int main() {
    int ringSize = 7;
    int actualDB = 2; // e.g. 4,5-dihydro-3H-azepine
    std::vector<int> satLocants = {3, 4, 5}; 
    std::sort(satLocants.begin(), satLocants.end());
    
    int indicatedH = -1;
    std::vector<int> hydroLocants;
    
    if (ringSize % 2 != 0) {
        indicatedH = satLocants[0];
        for (size_t i = 1; i < satLocants.size(); ++i) hydroLocants.push_back(satLocants[i]);
    } else {
        hydroLocants = satLocants;
    }
    
    std::string prefix = "";
    if (!hydroLocants.empty()) {
        for (size_t i = 0; i < hydroLocants.size(); ++i) {
            prefix += std::to_string(hydroLocants[i]);
            if (i + 1 < hydroLocants.size()) prefix += ",";
        }
        prefix += "-" + multiPrefix(hydroLocants.size()) + "hydro-";
    }
    
    if (indicatedH != -1) {
        prefix = std::to_string(indicatedH) + "H-" + prefix;
    } else if (prefix.back() == '-') {
        // wait, we shouldn't prepend it if indicatedH == -1, just use prefix.
    }
    
    std::cout << prefix << ""azine"" << std::endl;
}
