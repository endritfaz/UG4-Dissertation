#include <iostream>
#include <fstream>
#include <pqxx/pqxx> 

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
        
        // Initialise the database
        if (!init_results_table(c)) {
            std::cout << "Results table initialisation failed" << std::endl; 
            exit(-1); 
        }

        
    }

    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1; 
    }
} 