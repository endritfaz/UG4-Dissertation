#include <unistd.h>
#include <iostream>
#include <string>
#include <fmt/format.h> 
#include "Game.h"
#include "Engine.h"
#include <signal.h>
#include "nlohmann/json.hpp"
#include <fstream>
#include "helper.h"
#include <vector>

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
                // Check for a winner (no player has valid moves, board full, or resignation)
                if (game.gameOver()) { 
                    std::string winner = game.getWinner(); 
                    jgame["winner"] = winner;
                    jgame["moves"] = moves;
                    
                    #ifdef DEBUG
                        std::cout << "*";
                    #endif

                    return winner;
                }

                command = fmt::format("genmove {}\n", active.getColour()); 
                active.sendCommand(command);

                #ifdef DEBUG
                    std::cout << fmt::format("{}\n", command); 
                #endif

                // TODO: Check response is valid 
                response = active.getResponse('\n');
                
                #ifdef DEBUG
                    std::cout << fmt::format("{}\n", response); 
                #endif

                // Check if the move is actually valid and make it  
                if (!game.validateMove(response)) {
                    std::cout << "INVALID MOVE"; 
                    exit(0);
                } 

                game.makeMove(response); 
                
                // Record move for JSON file
                moves.push_back(response);

                command = fmt::format("play {} {}\n", active.getColour(), response); 
                inactive.sendCommand(command);
                
                #ifdef DEBUG
                    std::cout << fmt::format("{}\n", command); 
                #endif

                // TODO: Check if inactive player board update has succeeded
                response = inactive.getResponse('\n');
                
                #ifdef DEBUG
                    std::cout << fmt::format("{}\n", response); 
                #endif

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
                std::string output_name = fmt::format("games-{}-{}.json", engine1.getName(), engine2.getName());
                std::ofstream o(output_name);
                o << std::setw(4) << j << std::endl; 
            }

            printSummary(engine1wins, engine2wins, draws);
        }

        void printSummary(int engine1wins, int engine2wins, int draws) {
            std::cout << fmt::format("{} won {} games\n", engine1.getName(), engine1wins);
            std::cout << fmt::format("{} won {} games\n", engine2.getName(), engine2wins);
            std::cout << fmt::format("There were {} draws\n", draws); 
            std::cout << "------------------------------\n";
        }
};

std::string nameToExecutable(std::string name) {
    toLower(name); 

    if (name == "edax") {
        return "./edaxclient"; 
    }

    if (name == "az") {
        return "./azclient"; 
    }

    if (name == "random") {
        return "./randomclient";
    }

    return "";
}

//  e.g ./server az 1000 edax 21 2 0.5 true 10
int main(int argc, char* argv[]) {
    if (argc < 9) {
        std::cout << "Missing parameters\n"; 
        return -1; 
    }

    // Bot names and versions 
    std::string primary_engine_name = argv[1]; 
    std::string primary_engine_version = argv[2]; 

    std::string secondary_engine_name = argv[3];
    std::string secondary_engine_version = argv[4]; 

    std::stringstream convert_num_games{ argv[5] }; 

    // Number of games to play 
	int num_games{};
	if (!(convert_num_games >> num_games)) {
		return -1; 
    }

    // Probability of primary bot being selected as black in any game 
    std::stringstream convert_black_probability{ argv[6] }; 

	float black_probability{};
	if (!(convert_black_probability >> black_probability)) {
		return -1; 
    }

    // Whether to save games in JSON files or not 
    bool save = argv[7] == "true" ? true : false;
    std::cout << save; 
    // How often played games are persisted 
    std::stringstream convert_checkpoint_freq{ argv[8] }; 

	int checkpoint_freq{};
	if (!(convert_checkpoint_freq >> checkpoint_freq)) {
		return -1; 
    }

    // Name of directory to which games are persisted 
    std::string save_dir = fmt::format("{}v{}-{}v{}-{}", primary_engine_name, primary_engine_version, secondary_engine_name, secondary_engine_version, rand()); 

    // Start the server and bots 
    std::string primary_engine_executable = nameToExecutable(primary_engine_name); 
    std::string secondary_engine_executable = nameToExecutable(secondary_engine_name); 

    if (primary_engine_executable == "" || secondary_engine_executable == "") {
        std::cout << "Invalid engine name"; 
        return -1;
    }

    std::vector<std::string> primary_engine_args = {primary_engine_executable}; 
    std::vector<std::string> secondary_engine_args = {secondary_engine_executable};

    Engine engine1{primary_engine_args};
    Engine engine2{secondary_engine_args};
    Server serv{engine1, engine2};

    serv.engine1.setName(primary_engine_name); 
    serv.engine2.setName(secondary_engine_name);

    serv.start();

    serv.playGames(num_games, save);

    // Shut down engines
    kill(serv.engine1.getEnginePID(), SIGTERM); 
    kill(serv.engine2.getEnginePID(), SIGTERM); 
  
    return 0; 
}

