#include <unistd.h>
#include <iostream>
#include <string>
#include <fmt/format.h> 
#include "Game.h"
#include "Engine.h"
#include <signal.h>
#include "nlohmann/json.hpp"
#include <fstream>

using namespace std; 
using json = nlohmann::json;

class Server {
   
    public:
        Engine engine1; 
        Engine engine2;

        Server(Engine engine1, Engine engine2):
            engine1 {engine1},
            engine2 {engine2}
        {}

        void start() {
            const char* dir = "."; 
            engine1.startEngine(dir);
            engine2.startEngine(dir);

            // sleep(5);
        }
        
        std::string play(std::string engine1colour, std::string engine2colour, json& jgame) {
            Game game{};

            std::vector<std::string> moves{};
            
            engine1.setColour(engine1colour); 
            engine2.setColour(engine2colour); 
            
            // Set black/white names for JSON file 
            jgame[engine1.getColour()] = engine1.getName(); 
            jgame[engine2.getColour()] = engine2.getName(); 

            Engine active = engine1; 
            Engine inactive = engine2; 

            std::string command; 
            std::string response; 
            
            engine1.sendCommand("init\n");
            response = active.getResponse('\n');
            
            engine2.sendCommand("init\n"); 
            response = inactive.getResponse('\n');
            
            while(true) {
                // Check for a winner (no player has valid moves or board full)
                if (game.gameOver()) { 
                    std::string winner = game.getWinner(); 
                    jgame["winner"] = winner;
                    jgame["moves"] = moves;
                    return winner;
                }

                command = fmt::format("genmove {}\n", active.getColour()); 
                active.sendCommand(command);
                // std::cout << std::format("{}\n", command); 
                // TODO: Check response is valid 
                response = active.getResponse('\n');
                // std::cout << std::format("{}\n", response); 
               
                // TODO: Check if the move is actually valid 
                game.makeMove(response); 
                
                // Record move for JSON file
                moves.push_back(response);

                command = fmt::format("play {} {}\n", active.getColour(), response); 
                inactive.sendCommand(command);
                // std::cout << std::format("{}\n", command); 

                // TODO: Check if inactive player board update has succeeded
                response = inactive.getResponse('\n');
                // std::cout << std::format("{}\n", response); 
                // Swap active and inactive engine for next turn
                Engine temp = active;
                active = inactive; 
                inactive = temp; 
            }
        }

        void playGames(int n, bool save=false) {
            std::vector<json> games{};  
            
            std::cout << fmt::format("Playing {} game(s)\n", n);
            std::cout << "------------------------------\n";

            std::string engine1colour = "black";
            std::string engine2colour = "white"; 
            
            int engine1wins = 0; 
            int engine2wins = 0; 
            int draws = 0; 

            for (int i = 0; i < n; i++) {
                json game;
                std::string winner = play(engine1colour, engine2colour, game); 
                
                if (engine1colour == "black" && winner == "black" || engine1colour == "white" && winner == "white") {
                    engine1wins += 1; 
                }

                else if (engine2colour == "black" && winner == "black" || engine2colour == "white" && winner == "white") {
                    engine2wins += 1; 
                }

                else {
                    draws += 1; 
                }

                games.push_back(game);
            }
            
            // Save games played to output JSON file for later analysis
            if (save) {
                json j; 
                j["games"] = games; 
                j["num_games"] = n;

                // TODO: Make the filename a combination of the two player names, and a random number
                std::ofstream o("output-az1200.json");
                o << std::setw(4) << j << std::endl; 
            }

            printSummary(engine1wins, engine2wins, draws);
        }

        void printSummary(int engine1wins, int engine2wins, int draws) {
            std::cout << fmt::format("Engine1 won {} games\n", engine1wins);
            std::cout << fmt::format("Engine2 won {} games\n", engine2wins);
            std::cout << fmt::format("There were {} draws\n", draws); 
            std::cout << "------------------------------\n";
        }
};

int main() {
    char engine1_executable[] = "./AZClient2";
    char engine2_executable[] = "./RandomClient2"; 

    Engine engine1{engine1_executable};
    Engine engine2{engine2_executable};
    Server serv{engine1, engine2};

    serv.engine1.setName("az1200"); 
    serv.engine2.setName("random");

    serv.start();

    int num_games = 10;
    bool save = true;
    serv.playGames(num_games, save);

    // Shut down engines
    kill(serv.engine1.getEnginePID(), SIGTERM); 
    kill(serv.engine2.getEnginePID(), SIGTERM); 
  
    return 0; 
}
