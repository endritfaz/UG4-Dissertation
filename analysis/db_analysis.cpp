#include <filesystem>
#include <iostream>
#include <pqxx/pqxx> 
#include <vector>
#include "nlohmann/json.hpp"
#include <fstream>
#include "helper.h"
#include <fmt/format.h> 

using namespace std;
using namespace pqxx;
using json = nlohmann::json;

int main(int argc, char* argv[]) {
   std::string GAME_DIRECTORY="test_game_data"; 
   
   try {
      connection C("dbname = ug4 user = postgres password = postgres \
      hostaddr = 127.0.0.1 port = 5432");
        if (C.is_open()) {
            cout << "Opened database successfully: " << C.dbname() << endl;
            
            std::string sql;

            std::vector<float> edax6_winrate = {}; 
            std::vector<float> edax15_winrate = {}; 
            std::vector<float> edax21_winrate = {}; 

            // For each AZ training iteration in intervals of 10000
            for (int i = 0; i <= 28000; i+=1000) {
                
                std::string sql = fmt::format("SELECT * FROM games WHERE black='az' AND black_version='{}' AND white='edax' AND white_version='6';", i);

                nontransaction N(C); 
                result R(N.exec(sql)); 

                std::cout << R.size();
            }
        }

        else {
            cout << "Can't open database" << endl;
            return 1;
        }
    } 
    catch (const std::exception &e) {
      cerr << e.what() << std::endl;
      return 1;
    }
}