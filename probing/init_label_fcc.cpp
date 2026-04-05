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
        if (!init_label_fcc_table(c)) {
            std::cout << "FCC label table initialisation failed" << std::endl; 
            exit(-1); 
        }   

        work tx(c);
        result positions = tx.exec("SELECT id, black, white, turn FROM positions;");
        tx.commit();

        
        prepare_fcc_insert(c); 

        int samples = 5000; 
        int pos = 0; 
        int neg = 0; 

        for (const auto& row : positions) {
            if (neg > samples && pos > samples) 
                break; 
            
            int id = row["id"].as<int>();
            std::string turn = row["turn"].as<std::string>(); 

            // For some assurance that program is not stuck
            std::cout << id << std::endl; 

            auto black_bytes = row["black"].as<pqxx::bytes>();
            auto white_bytes = row["white"].as<pqxx::bytes>();

            uint64_t black = std::stoull(std::string(reinterpret_cast<const char*>(black_bytes.data()), black_bytes.size()));
            uint64_t white = std::stoull(std::string(reinterpret_cast<const char*>(white_bytes.data()), white_bytes.size()));
            
            uint64_t active = black; 
            uint64_t inactive = white;

            if (turn == "white") {
                active = white; 
                inactive = black; 
            }
            
            uint64_t fcc = forcedCornerCaptures(active, inactive);
            int num_active_fcc_possible = __builtin_popcountll(fcc);
            bool active_fcc_possible = num_active_fcc_possible > 0; 
            
            bool exec = false;

            if (active_fcc_possible && pos < samples) {
                exec = true; 
                pos += 1;
            }

            else if (!active_fcc_possible && neg < samples) {
                exec = true; 
                neg += 1;
            }
            
            if (exec)
                tx.exec(prepped{"label_fcc_insert"}, params{id, active_fcc_possible, num_active_fcc_possible});
            
        }

        tx.commit();
    }


    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1; 
    }
} 
