#include <iostream>
#include <pqxx/pqxx> 

using namespace std;
using namespace pqxx;

int main(int argc, char* argv[]) {
    try {
      connection C("dbname = ug4 user = postgres password = postgres \
      hostaddr = 127.0.0.1 port = 5432");
      if (C.is_open()) {
         cout << "Opened database successfully: " << C.dbname() << endl;

         /* Create SQL statement */
        char* sql =  "INSERT INTO games (black,black_version,white,white_version,game_sequence, winner) " \
            "VALUES ('bot1','bot1_version','bot2','bot2_version','abcd','black');"; 

        /* Create a transactional object. */
        work W(C);
        
        /* Execute SQL query */
        W.exec( sql );
        W.commit();
        cout << "Records created successfully" << endl;

      } else {
         cout << "Can't open database" << endl;
         return 1;
      }
   } catch (const std::exception &e) {
      cerr << e.what() << std::endl;
      return 1;
   }
}