#include <iostream>
#include <string>
#include <fmt/format.h>
#include "Engine.h"
#include <sstream>

class EdaxClient {
    public:
        Engine engine;
        
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
                engine.sendCommand("init\n");
                std::string response = engine.getResponse('>');
                std::cout << "success\n";
                return; 

            }
            
            std::getline(ss, colour, del); 

            if (command_type == "play") {
                std::getline(ss, move, del);
                move = reflectRow(move);
                // std::cerr << std::format("play {}\n", move);
                
                std::string command = fmt::format("play {}\n", move);
                engine.sendCommand(command);
                std::string response = engine.getResponse('>');

                std::cout << "success\n";
            }

            else if (command_type == "genmove") {
                std::string command = "go\n"; 

                engine.sendCommand(command); 
                std::string response = engine.getResponse('>');
                
                std::string move = extractMove(response);
                toLower(&move); 

                std::cout << fmt::format("{}\n", move);
            }
        }

        void start() {
            engine.startEngine("edax-4.6-linux-x86"); 
        }

        void ready() {
            // Capture intial input (to throwaway)
            std::string response = engine.getResponse('>');
      
            engine.sendCommand("set verbose 0\n");

            // Capture newline created by setting verbose (to throwaway)
            response = engine.getResponse('>');
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

            if (move.at(0) == 'p' && move.at(1) == 'a') {
                return "pass"; 
            }

            return reflectRow(move);
        }

        std::string reflectRow(std::string move) {
            move.replace(1, 1, 1, '9' - move.at(1) + '0');
            return move; 
        }
};

int main() {
    char edax_executable[] = "./lEdax-x86-64";
    Engine engine{edax_executable};
    EdaxClient client{engine};
    
    client.start();
    client.ready(); 
    client.play();
}