#include <iostream>
#include <string>
#include <format>
#include "Engine.h"

class EdaxClient {
    public:
        Engine engine;
        
        EdaxClient(Engine engine): 
            engine {engine}
        {}

        void parse() {}

        void start() {
            engine.startEngine(); 
        }

        void play() {
            engine.sendCommand("set verbose 0\n");
            std::string response = engine.getResponse('>');
            std::cout << "got response\n";
            
            // Need to get response after playing
            engine.sendCommand("play c4\n");
            response = engine.getResponse('>');

            engine.sendCommand("go\n");
            std::cout << "sent command\n";
            response = engine.getResponse('>');
            std::cout << "got response\n";
            std::cout << std::format("{}\n", extractMove(response)); 
        }

        // TODO: Replace with regex 
        std::string extractMove(std::string response) {
            std::string move = response.substr(13, 15);
            return move;
        }
};

int main() {
    char edax_executable[] = "./lEdax-x86-64";
    Engine engine{edax_executable};

    EdaxClient client{engine};
    std::cout << "instantialised\n";
    client.start();
    std::cout << "started\n"; 
    client.play();
}