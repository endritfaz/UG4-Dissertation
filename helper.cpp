#include <string>
#include <algorithm>
#include <bits/stdc++.h>

void toLower(std::string& s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower); 
}

void removeChar(std::string& s, char c) {
    s.erase(std::remove(s.begin(), s.end(), c), s.end());
}

// GENERATED  BY AI 
std::string join(const std::vector<std::string>& vec, const std::string& delimiter) {
    std::string result;

    for (size_t i = 0; i < vec.size(); ++i) {
        result += vec[i];
        if (i != vec.size() - 1) {
            result += delimiter;
        }
    }

    return result;
}