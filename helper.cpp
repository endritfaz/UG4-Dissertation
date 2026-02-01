#include <string>
#include <algorithm>
#include <bits/stdc++.h>

void toLower(std::string& s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower); 
}