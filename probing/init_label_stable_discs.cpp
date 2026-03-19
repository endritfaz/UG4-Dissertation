#include <iostream>
#include <fstream>
#include <pqxx/pqxx> 
#include <cstring>
#include "othello.h"

#include "probing_db.h"

using namespace pqxx;

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
        
        // Initialise the total discs label
        if (!init_label_stable_table(c)) {
            std::cout << "Total discs label table initialisation failed" << std::endl; 
            exit(-1); 
        }   

        work tx(c);
        result positions = tx.exec("SELECT id, black, white, turn FROM positions;");
        tx.commit();

        
        prepare_stable_discs_insert(c);

        // Required for stable disc detection
        buildStableEdgeTable(); 

        for (const auto& row : positions) {
            int id = row["id"].as<int>();
            std::string turn = row["turn"].as<std::string>(); 

            // For some assurance that program is not stuck
            std::cout << id << std::endl; 

            auto black_bytes = row["black"].as<pqxx::bytes>();
            auto white_bytes = row["white"].as<pqxx::bytes>();

            uint64_t black = std::stoull(std::string(reinterpret_cast<const char*>(black_bytes.data()), black_bytes.size()));
            uint64_t white = std::stoull(std::string(reinterpret_cast<const char*>(white_bytes.data()), white_bytes.size()));
            
            int black_stable = __builtin_popcountll(stableDiscs(black, white)); 
            int white_stable = __builtin_popcountll(stableDiscs(white, black)); 
            int total_stable = black_stable + white_stable; 

            tx.exec(prepped{"label_sd_insert"}, params{id, black_stable, white_stable, total_stable, turn});
        }

        tx.commit();
    }


    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1; 
    }
} 
