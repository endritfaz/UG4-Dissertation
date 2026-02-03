#include <iostream>
#include <string>
#include <fmt/format.h>
#include "Engine.h"
#include <sstream>
#include "helper.h"
#include "Game.h"

class EdaxClient {
    public:
        Engine engine;
        Game game; 
        bool pass = false; 

        EdaxClient(Engine engine): 
            engine {engine}
        {}

        void parse(std::string line) {
            std::stringstream ss(line);

            std::string command_type;
            std::string colour; 
            std::string move; 

            char del = ' '; 

            std::getline(ss, command_type, del);

            if (command_type == "init") {
                game = Game(); 
                pass = false; 
                engine.sendCommand("init\n");
                std::string response = engine.getResponse('>');
                std::cout << "success\n";
                return; 
            }
            
            std::getline(ss, colour, del); 

            if (command_type == "play") {
                std::getline(ss, move, del);
                game.makeMove(move);
                
                if (game.validateMove("pass")) {
                    game.makeMove("pass"); 
                    pass = true;  
                }

                move = reflectRow(move);
                // std::cerr << std::format("play {}\n", move);
                
                std::string command = fmt::format("play {}\n", move);
                engine.sendCommand(command);
                std::string response = engine.getResponse('>');

                std::cout << "success\n";
            }

            else if (command_type == "genmove") {
                if (pass) {
                    pass = false; 
                    std::cout << "pass\n"; 
                    return;
                }

                std::string command = "go\n"; 

                engine.sendCommand(command); 
                std::string response = engine.getResponse('>');
                
                std::string move = extractMove(response);
                toLower(move);  
                
                game.makeMove(move); 
                std::cout << fmt::format("{}\n", move);
            }
        }

        void start() {
            engine.startEngine("edax-4.6-linux-x86"); 
        }

        void ready(std::string level) {
            // Capture intial input (to throwaway)
            std::string response = engine.getResponse('>');
      
            engine.sendCommand("set verbose 0\n");

            // Capture newline created by setting verbose (to throwaway)
            response = engine.getResponse('>');

            engine.sendCommand(fmt::format("level {}\n", level));

            // Capture newline created by setting level
            response = engine.getResponse('>');

            std::cout << "Ready\n";
        }

        void play() {
            while(true) {
                std::string line; 
                std::getline(std::cin, line);
                
                parse(line);
            }
        }

        // TODO: Replace with regex 
        std::string extractMove(std::string response) {
            std::string move = response.substr(13, 15);

            if (move.at(0) == 'P' && move.at(1) == 'A') {
                return "pass"; 
            }

            return reflectRow(move);
        }

        std::string reflectRow(std::string move) {
            move.replace(1, 1, 1, '9' - move.at(1) + '0');
            return move; 
        }
};

int main(int argc, char* argv[]) {
    char edax_executable[] = "./lEdax-x86-64";

    std::string level = argv[1]; 

    Engine engine{edax_executable};
    EdaxClient client{engine};
    
    client.start();
    client.ready(level); 
    client.play();
}
