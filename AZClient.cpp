#include <iostream>
#include <string>
#include <fmt/format.h>
#include "Engine.h"
#include <sstream>
#include <vector>

class AZClient {
    public:
        Engine engine;
        
        AZClient(Engine engine): 
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
                engine.sendCommand("clear_board\n");
                std::string response = engine.getResponse('\n');
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
                std::string command = fmt::format("genmove {}\n", colour); 

                engine.sendCommand(command); 
                std::string response = engine.getResponse('=');
                response = engine.getResponse('\n'); 
             
                std::cout << fmt::format("{}\n", extractMove(response));
            }
        }

        void start() {
            const char* path = "/home/endret/minizero";
            engine.startEngine(path); 
        }

        void ready() {
            sleep(2);
            // Capture intial input (to throwaway)
            std::string response = engine.getResponse('&');
            response = engine.getResponse('&');
            response = engine.getResponse('&');
            response = engine.getResponse('&');
            std::cout << response; 
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
    std::vector<std::string> az_argv = {
        "./tools/quick-run.sh",
        "console",
        "othello",
        "othello_az_n200.pt",
        "othello_8x8_az.cfg",
    };

    Engine engine{az_argv};
    AZClient client{engine};
    
    client.start();
    client.ready(); 
    client.play();
}
