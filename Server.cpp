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
#include <filesystem>

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

            // Wait until engines are ready before starting play 
            engine1.getResponse('\n');
            engine2.getResponse('\n');      
        }
        
        std::string play(std::string engine1colour, std::string engine2colour, json& jgame) {
            Game game{}; 
            std::vector<std::string> moves{};
            
            engine1.setColour(engine1colour); 
            engine2.setColour(engine2colour); 
            
            // Set black/white names and versions for JSON file 
            jgame[engine1.getColour()] = engine1.getName(); 
            jgame[fmt::format("{}_version", engine1.getColour())] = engine1.getVersion();

            jgame[engine2.getColour()] = engine2.getName(); 
            jgame[fmt::format("{}_version", engine2.getColour())] = engine2.getVersion(); 

            Engine active = engine1; 
            Engine inactive = engine2;

            if (engine1.getColour() == "white" && engine2.getColour() == "black") {
                active = engine2; 
                inactive = engine1; 
            } 

            std::string command; 
            std::string response; 
            
            active.sendCommand("init\n");
            response = active.getResponse('\n');
            
            inactive.sendCommand("init\n"); 
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

        void playGames(int n, float black_probability, bool save=false, int checkpoint_freq=-1, std::string output_dir="") {
            std::vector<json> games{};  
            
            std::cout << fmt::format("Playing {} game(s)\n", n);
            std::cout << "------------------------------\n";

            // Bernoulli distribution used for player colour selection in each game 
            std::random_device rd;
            std::mt19937 gen(rd());

            std::bernoulli_distribution d(black_probability);
            
            std::string engine1colour; 
            std::string engine2colour; 
        
            int engine1wins = 0; 
            int engine2wins = 0; 
            int draws = 0; 

            for (int i = 0; i < n; i++) {
                json game;
                
                // Choose engine1 playing as black with probability black_probability
                engine1colour = "black";
                engine2colour = "white";
                
                bool outcome = d(gen);
                if (!outcome) {
                    engine1colour = "white";
                    engine2colour = "black"; 
                }
                
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
                
                // Games are saved every checkpoint_freq number of games 
                if ((i + 1) % checkpoint_freq == 0 || i == n-1) {
                    if (save) {
                        json j; 
                        j["games"] = games; 
                        j["num_games"] = checkpoint_freq;

                        // Save the game to output_path (relative to program)
                        std::string output_name = fmt::format("games-{}-{}-{}.json", engine1.getName(), engine2.getName(), i+1);
                        std::string output_path = fmt::format("{}/{}", output_dir, output_name);
                        std::ofstream o(output_path);

                        o << std::setw(4) << j << std::endl; 

                        games = {};
                    }
                }
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

// e.g ./server az 1000 edax 21 2 0.5 true 10
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
    bool save = std::string(argv[7]) == "true";

    // How often played games are persisted 
    std::stringstream convert_checkpoint_freq{ argv[8] }; 

	int checkpoint_freq{};
	if (!(convert_checkpoint_freq >> checkpoint_freq)) {
		return -1; 
    }

    // Handle invalid checkpoint frequency 
    if (checkpoint_freq < 1 || checkpoint_freq > num_games) {
        checkpoint_freq = num_games; 
    }

    // Start the server and bots 
    std::string primary_engine_executable = nameToExecutable(primary_engine_name); 
    std::string secondary_engine_executable = nameToExecutable(secondary_engine_name); 

    if (primary_engine_executable == "" || secondary_engine_executable == "") {
        std::cout << "Invalid engine name"; 
        return -1;
    }

    // Name of directory to which games are persisted 
    srand(time(0));

    std::string save_dir = fmt::format("test_game_data/{}v{}-{}v{}-{}", primary_engine_name, primary_engine_version, secondary_engine_name, secondary_engine_version, rand()); 

    // Create save directories if necessary
    if (save) {
        std::filesystem::create_directories(save_dir);
    }
    
    std::vector<std::string> primary_engine_args = {primary_engine_executable, primary_engine_version}; 
    std::vector<std::string> secondary_engine_args = {secondary_engine_executable, secondary_engine_version};

    Engine engine1{primary_engine_args};
    Engine engine2{secondary_engine_args};
    Server serv{engine1, engine2};

    // Set engines' names and versions 
    serv.engine1.setName(primary_engine_name); 
    serv.engine2.setName(secondary_engine_name);

    serv.engine1.setVersion(primary_engine_version); 
    serv.engine2.setVersion(secondary_engine_version);

    serv.start();

    serv.playGames(num_games, black_probability, save, checkpoint_freq, save_dir);

    // Shut down engines
    kill(serv.engine1.getEnginePID(), SIGTERM); 
    kill(serv.engine2.getEnginePID(), SIGTERM); 
  
    return 0; 
}