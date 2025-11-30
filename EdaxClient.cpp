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
            engine.sendCommand("go\n");
            std::cout << "sent command\n";
            std::string response = engine.getResponse();
            std::cout << "got response\n";
            std::cout << std::format("{}\n", response); 
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