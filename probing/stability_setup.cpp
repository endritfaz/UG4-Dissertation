#include <iostream>
#include <fstream>
#include <pqxx/pqxx> 
#include <cstring>
#include "othello.h"
#include "shuffler.h"
#include "probing_db.h"

using namespace pqxx;

std::string GAME_DIRECTORY = "game_data"; 

bool load_positions(connection& c) {
    prepare_position_insert(c);
    prepare_stable_discs_insert(c);
    buildStableEdgeTable();

    // Ensure a fairly uniform distribution of stable disc positions is chosen 
    int STABLE_LIMIT = 1000; 
    int stable_positions[65] = {0}; 

    // Gather filepaths of game files 
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
        return false; 
    }

    // Shuffler to generate probing controls 
    shuffle::shuffler shuffler{}; 

    for (const auto& json_path : json_filepaths) {
        std::cout << "Current JSON file: " << json_path.string() << std::endl;

        std::ifstream i(json_path.string());
        json j; 
        i >> j; 

        json games = j["games"];

        work tx(c); 
        
        for (const auto& game : games) {
            std::vector<json> position_features = extractPositionFeatures(game["moves"]); 

            for (const auto& position_feature : position_features) {
                uint64_t black = position_feature["black"]; 
                uint64_t white = position_feature["white"];
                
                std::array<uint64_t, 2> shuffled_boards = shuffler.shuffle_bitboards(black, white); 

                uint64_t black_shuffled = shuffled_boards[0]; 
                uint64_t white_shuffled = shuffled_boards[1];  

                std::string turn = position_feature["active"];
                
                int black_stable = __builtin_popcountll(stableDiscs(black, white)); 
                int white_stable = __builtin_popcountll(stableDiscs(white, black)); 
                int total_stable = black_stable + white_stable; 
                
                // TODO: Add early stopping if entire array is "full", but this shouldn't happen with my currently sparse database
                if (stable_positions[total_stable] >= STABLE_LIMIT) {
                    continue;
                }
                
                result r = tx.exec(prepped{"position_insert"}, params{black, white, black_shuffled, white_shuffled, turn});
                
                // SQL return NULL because of duplicate position
                if (r.empty()) { 
                    continue;
                }

                // Position ID of position just inserted into positions table
                int position_id = r[0]["id"].as<int64_t>();
                
                tx.exec(prepped{"label_sd_insert"}, params{position_id, black_stable, white_stable, total_stable, turn});

                stable_positions[total_stable] += 1; 
            }
        }

        tx.commit();  
    }
    return true; 
}

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
        
        // Initialise positions, activations, label, and results tables
        bool pos_init = init_positions_table(c);
        bool act_init = init_activations_table(c);
        bool label_init = init_label_stable_table(c);
        bool res_init = init_results_table(c);

        if (!(pos_init && act_init && label_init && res_init)) {
            std::cout << "Table initialisation failed" << std::endl; 
            exit(-1); 
        } 
        
        // Load the positions and label tables 
        load_positions(c); 
    }

    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1; 
    }
} 
