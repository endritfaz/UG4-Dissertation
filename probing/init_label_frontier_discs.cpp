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
        if (!init_label_frontier_table(c)) {
            std::cout << "Frontier discs label table initialisation failed" << std::endl; 
            exit(-1); 
        }   

        work tx(c);
        result positions = tx.exec("SELECT id, black, white, turn FROM positions;");
        tx.commit();

        
        prepare_frontier_discs_insert(c);

        for (const auto& row : positions) {
            int id = row["id"].as<int>();
            std::string turn = row["turn"].as<std::string>(); 

            // For some assurance that program is not stuck
            std::cout << id << std::endl; 

            auto black_bytes = row["black"].as<pqxx::bytes>();
            auto white_bytes = row["white"].as<pqxx::bytes>();

            uint64_t black = std::stoull(std::string(reinterpret_cast<const char*>(black_bytes.data()), black_bytes.size()));
            uint64_t white = std::stoull(std::string(reinterpret_cast<const char*>(white_bytes.data()), white_bytes.size()));
            
            int black_frontier = __builtin_popcountll(frontierDiscs(black, white)); 
            int white_frontier = __builtin_popcountll(frontierDiscs(white, black)); 
            
            int active_frontier = black_frontier; 
            int inactive_frontier = white_frontier;

            if (turn == "white") {
                active_frontier = white_frontier; 
                inactive_frontier = black_frontier; 
            }

            int total_frontier = black_frontier + white_frontier; 

            tx.exec(prepped{"label_fd_insert"}, params{id, active_frontier, inactive_frontier, total_frontier, turn});
        }

        tx.commit();
    }


    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1; 
    }
} 
