#include <iostream>
#include <fstream>
#include <pqxx/pqxx> 

#include "nlohmann/json.hpp"
#include "helper.h"
#include "analysis.h"
#include "othello.h"
#include "probing_db.h"

using namespace pqxx;
using json = nlohmann::json;

std::string GAME_DIRECTORY = "game_data"; 

int main() {
    try {
        connection c("dbname = ug4 user = postgres password = postgres \
        hostaddr = 127.0.0.1 port = 5432");
        if (c.is_open()) {
            std::cout << "Opened database successfully: " << c.dbname() << std::endl;
        } 
        
        else {
            std::cout << "Can't open database" << std::endl;
            return 1;
        }
        
        // Initialise the database
        if (!init_positions_table(c)) {
            std::cout << "Positions table initialisation failed" << std::endl; 
            exit(-1); 
        }

        std::vector<std::filesystem::path> json_filepaths;
        if (std::filesystem::exists(GAME_DIRECTORY)) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(GAME_DIRECTORY)) {
                if (entry.is_regular_file() && entry.path().extension() == ".json") {
                    json_filepaths.push_back(entry.path());
                }
            }
        }

        // No games found in game directory, so exit early 
        if (json_filepaths.empty()) {
            return 0; 
        }

        // For each game, call function that returns a list of json objects, add these moves to move table
        prepare_position_insert(c); 
      
        std::cout << "JSON file count: ";
        std::cout << json_filepaths.size() << std::endl; 
        
    
        for (const auto& json_path : json_filepaths) {
            std::cout << "Current JSON file: " << json_path.string() << std::endl;
    
            std::ifstream i(json_path.string());
            json j; 
            i >> j; 

            json games = j["games"];

            work tx(c); 
            /*
            for (const auto& game : games) {
                std::vector<json> position_features = extractPositionFeatures(game["moves"]); 

                for (const auto& position_feature : position_features) {
                    uint64_t black = position_feature["black"]; 
                    uint64_t white = position_feature["white"];
                    std::string turn = position_feature["active"];

                    // tx.exec(prepped{"position_insert"}, params{black, white, turn});
                }
            }
            */

            uint64_t black = 0x1008000000; 
            uint64_t white = 0x810000000;
            std::string turn = "black";

            tx.exec(prepped{"position_insert"}, params{black, white, black, white, turn});
            
            black = 0x81808000000; 
            white = 0x10000000;
            turn = "white";

            tx.exec(prepped{"position_insert"}, params{black, white, black, white, turn});

            tx.commit();  
            return 0; 
        }
    }

    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1; 
    }
}    