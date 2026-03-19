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
        if (!init_label_discs_table(c)) {
            std::cout << "Total discs label table initialisation failed" << std::endl; 
            exit(-1); 
        }   

        work tx(c);
        result positions = tx.exec("SELECT id, black, white, turn FROM positions;");
        tx.commit();

        
        prepare_label_discs_total_insert(c);

        for (const auto& row : positions) {
            int id = row["id"].as<int>();

            // For some assurance that program is not stuck
            std::cout << id << std::endl; 

            auto black_bytes = row["black"].as<pqxx::bytes>();
            auto white_bytes = row["white"].as<pqxx::bytes>();

            uint64_t black = std::stoull(std::string(reinterpret_cast<const char*>(black_bytes.data()), black_bytes.size()));
            uint64_t white = std::stoull(std::string(reinterpret_cast<const char*>(white_bytes.data()), white_bytes.size()));
            
            int total_pieces = countTotalPieces(black, white); 

            tx.exec(prepped{"label_td_insert"}, params{id, total_pieces});
        }
        tx.commit();
    }


    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1; 
    }
} 
