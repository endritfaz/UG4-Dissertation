#include <iostream>
#include <string>
#include <fmt/format.h>
#include "Engine.h"
#include <sstream>
#include <vector>
#include <helper.h>

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

                if (move == "pass") {
                    move = "PASS";
                }

                std::string command = fmt::format("play {} {}\n", colour, move);
                engine.sendCommand(command);
                engine.getResponse('\n');
                engine.emptyResponse();
                std::cout << "success\n";
            }

            else if (command_type == "genmove") {
                std::string command = fmt::format("genmove {}\n", colour);

                engine.sendCommand(command);
                std::string move = engine.readMove();
                toLower(move);

                engine.emptyResponse();
                std::cout << move << "\n";
            }
        }

        void start() {
            const char* path = "/home/endret/minizero";
            engine.startEngine(path);
        }

        // TODO: Hacky, fix this
        void ready() {
            engine.readUntilReady();
            sleep(5);
            engine.emptyResponse();
            std::cout << "Ready\n";
        }

        void play() {
            while(true) {
                std::string line;
                std::getline(std::cin, line);

                parse(line);
            }
        }
};

int main(int argc, char* argv[]) {
    // TODO: Security risk, check this is a number
    std::string iteration = argv[1];

    std::vector<std::string> az_argv = {
        "./tools/quick-run.sh",
        "console",
        "othello",
        fmt::format("weights/weight_iter_{}.pt", iteration),
        "othello_8x8_az_play.cfg",
        "-conf_str",
        "actor_resign_threshold=-1.0"
    };

    Engine engine{az_argv};
    AZClient client{engine};

    client.start();
    client.ready();
    client.play();
}
