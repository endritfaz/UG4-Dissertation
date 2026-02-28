#include <iostream>
#include <fstream>
#include <pqxx/pqxx> 

#include "nlohmann/json.hpp"
#include "helper.h"
#include "analysis.h"

using namespace pqxx;
using json = nlohmann::json;

std::string GAME_DIRECTORY = "game_data"; 

bool initialise_database(connection& c) {
    std::string remove_tables_sql = "DROP TABLE IF EXISTS games, moves"; 

    std::string create_games_table_sql = "CREATE TABLE games(" \
                                    "game_id INT GENERATED ALWAYS AS IDENTITY PRIMARY KEY," \
                                    "black VARCHAR(255) NOT NULL," \
                                    "white VARCHAR(255) NOT NULL," \
                                    "black_version VARCHAR(255) NOT NULL," \
                                    "white_version VARCHAR(255) NOT NULL," \
                                    "winner VARCHAR(255) NOT NULL," \
                                    "resign BOOLEAN NOT NULL);"; 

    std::string create_moves_table_sql = "CREATE TABLE moves(" \
                                    "game_id INT REFERENCES games(game_id)," \
                                    "ply INT NOT NULL," \
                                    "black_active BOOLEAN NOT NULL," \
                                    "num_discs INT NOT NULL," \
                                    "num_discs_black INT NOT NULL," \
                                    "num_discs_white INT NOT NULL," \
                                    "num_moves_black INT NOT NULL," \
                                    "num_moves_white INT NOT NULL," \
                                    "num_frontier_black INT NOT NULL," \
                                    "num_frontier_white INT NOT NULL," \
                                    "forced_corner_capture_possible BOOLEAN NOT NULL," \
                                    "forced_corner_capture_executed BOOLEAN NOT NULL," \
                                    "parity BOOLEAN NOT NULL," \
                                    "stable_discs_black INT," \
                                    "stable_discs_white INT);";
                                    
    try {
        work tx(c); 
        tx.exec(remove_tables_sql);
        tx.exec(create_games_table_sql);
        tx.exec(create_moves_table_sql); 
        tx.commit(); 

        return true;
    }

    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return false; 
    }
}

void prepare_move_insert(connection& c) {
    c.prepare(
        "move_insert", 
        "INSERT INTO moves (game_id, ply, black_active, num_discs, num_discs_black, num_discs_white, num_moves_black, num_moves_white, num_frontier_black, num_frontier_white, forced_corner_capture_possible, forced_corner_capture_executed, parity, stable_discs_black, stable_discs_white) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14, $15);"
    );
}

void prepare_game_insert(connection& c) {
    c.prepare(
        "game_insert", 
        "INSERT INTO games (black, white, black_version, white_version, winner, resign) VALUES ($1, $2, $3, $4, $5, $6) RETURNING game_id;"
    );
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
        
        // Initialise the database
        if (!initialise_database(c)) {
            std::cout << "Database initialisation failed" << std::endl; 
            exit(-1); 
        }

        std::cout << "Database tables initialised successfully" << std::endl; 

        // Iterate through game data, add game to game table
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
        prepare_game_insert(c); 
        prepare_move_insert(c);
        for (const auto& json_path : json_filepaths) {
            std::cout << "Current JSON file: " << json_path.string() << std::endl;
    
            std::ifstream i(json_path.string());
            json j; 
            i >> j; 

            json games = j["games"];

            work tx(c); 
            for (const auto& game : games) {
                // Extract game features from json 
                std::string black = game["black"]; 
                std::string black_version = game["black_version"]; 
                std::string white = game["white"]; 
                std::string white_version = game["white_version"]; 
                std::string winner = game["winner"]; 
                
                bool resign = game["moves"].back() == "resign"; 
                
                // Temporary fix for bash script play_games.sh previously adding comma to edax version names
                removeChar(black_version, ',');
                removeChar(white_version, ',');

                result r = tx.exec(prepped{"game_insert"}, params{black, white, black_version, white_version, winner, resign});
                
                // Get the game_id of the game just inserted, to use it as a foreign key in the moves table
                int game_id = r[0]["game_id"].as<int64_t>();
                
                std::vector<json> move_features = extractMoveFeatures(game["moves"]); 

                for (const auto& move_feature : move_features) {
                    int ply = move_feature["ply"]; 
                    bool black_active = move_feature["black_active"];
                    int num_discs = move_feature["num_discs"];
                    int num_discs_black = move_feature["num_discs_black"];
                    int num_discs_white = move_feature["num_discs_white"];
                    int num_moves_black = move_feature["num_moves_black"];
                    int num_moves_white = move_feature["num_moves_white"];
                    int num_frontier_black = move_feature["num_frontier_black"];
                    int num_frontier_white = move_feature["num_frontier_white"];
                    bool forced_corner_capture_possible = move_feature["forced_corner_capture_possible"]; 
                    bool forced_corner_capture_executed = move_feature["forced_corner_capture_executed"]; 
                    bool parity = move_feature["parity"]; 
                    
                    // Temporary invalid values for stable_discs, as stable disc detection function has not been implemented yet
                    tx.exec(prepped{"move_insert"}, params{game_id, ply, black_active, num_discs, num_discs_black, num_discs_white, num_moves_black, num_moves_white, num_frontier_black, num_frontier_white, forced_corner_capture_possible, forced_corner_capture_executed, parity, -1, -1});
                }
            }

            tx.commit(); 
        } 
    }

    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1; 
    }
}   