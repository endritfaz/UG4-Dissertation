#include <unistd.h>
#include <iostream>
#include <string>
#include <format> 
#include "Game.h"
#include "Engine.h"
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
            engine1.startEngine();
            engine2.startEngine();
        }
        
        void play() {
            Game game{};
            
            // TODO: Have this be decided separately 
            engine1.setColour("black"); 
            engine2.setColour("white"); 

            Engine active = engine1; 
            Engine inactive = engine2; 

            std::string command; 
            std::string response; 

            while(true) {
                // sleep(1);
                // Check for a winner (no player has valid moves or board full)
                if (game.gameOver()) { 
                    std::cout << game.getWinner();
                    break; 
                }
                command = std::format("genmove {}\n", active.getColour()); 
                active.sendCommand(command);
                std::cout << std::format("{}\n", command); 
                // TODO: Check response is valid 
                response = active.getResponse();
                std::cout << std::format("{}\n", response); 
               
                // TODO: Check if the move is actually valid 
                game.makeMove(response); 

                command = std::format("play {} {}\n", active.getColour(), response); 
                inactive.sendCommand(command);
                std::cout << std::format("{}\n", command); 

                // TODO: Check if inactive player board update has succeeded
                response = inactive.getResponse();
                std::cout << std::format("{}\n", response); 
                // Swap active and inactive engine for next turn
                Engine temp = active;
                active = inactive; 
                inactive = temp; 
            }
        }
};

int main() {
    char engine1_executable[] = "./client1";
    char engine2_executable[] = "./client2"; 

    Engine engine1{engine1_executable};
    Engine engine2{engine2_executable};
    Server serv{engine1, engine2};

    serv.start();
    serv.play();
    return 0; 
}