#include <unistd.h>
#include <iostream>
#include <string>
#include <fmt/format.h> 
#include "Game.h"
#include "Engine.h"
#include <signal.h>
using namespace std; 

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
        
        std::string play(std::string engine1colour, std::string engine2colour) {
            Game game{};
            
            // TODO: Have this be decided separately 
            engine1.setColour(engine1colour); 
            engine2.setColour(engine2colour); 
            
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
                    return game.getWinner();
                }

                command = fmt::format("genmove {}\n", active.getColour()); 
                active.sendCommand(command);
                // std::cout << std::format("{}\n", command); 
                // TODO: Check response is valid 
                response = active.getResponse('\n');
                // std::cout << std::format("{}\n", response); 
               
                // TODO: Check if the move is actually valid 
                game.makeMove(response); 

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

        void playGames(int n) {
            std::cout << fmt::format("Playing {} game(s)\n", n);
            std::cout << "------------------------------\n";

            std::string engine1colour = "black";
            std::string engine2colour = "white"; 
            
            int engine1wins = 0; 
            int engine2wins = 0; 
            int draws = 0; 

            for (int i = 0; i < n; i++) {
                std::string winner = play(engine1colour, engine2colour); 

                if (engine1colour == "black" && winner == "black" || engine1colour == "white" && winner == "white") {
                    engine1wins += 1; 
                }

                else if (engine2colour == "black" && winner == "black" || engine2colour == "white" && winner == "white") {
                    engine2wins += 1; 
                }

                else {
                    draws += 1; 
                }
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
    char engine1_executable[] = "./EdaxClient";
    char engine2_executable[] = "./RandomClient2"; 

    Engine engine1{engine1_executable};
    Engine engine2{engine2_executable};
    Server serv{engine1, engine2};

    serv.start();
    serv.playGames(10);

    // Shut down engines
    kill(serv.engine1.getEnginePID(), SIGTERM); 
    kill(serv.engine2.getEnginePID(), SIGTERM); 
  
    return 0; 
}
