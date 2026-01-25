#include "nlohmann/json.hpp"
#include <iostream>
#include <fstream>

using json = nlohmann::json;

int main() {
    json j; 
    json p; 
    std::vector<std::string> games {"example_game1", "example_game_2"}; 
    games.push_back("eg3");
    j["games"] = games;
    p["total"] = j; 
    // Print JSON with indent of 4 spaces
    std::cout << p.dump(4); 

    // Write JSON to file with indent of 4 spaces 
    std::ofstream o("output.json");
    o << std::setw(4) << j << std::endl; 
}