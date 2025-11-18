#include <iostream>
#include <cstdint>
#include "Game.h"
#include "othello.h"
#include <string>

#include <cstdint>
#include <random>
#include <bitset>
#include <sstream>

class RandomClient {
    public:
        Game game{};

        uint64_t pickRandomSetBit(uint64_t x) {
            if (x == 0) return 0;  // nothing to pick

            // Count set bits
            int count = __builtin_popcountll(x);

            // Pick random index
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, count - 1);
            int k = dis(gen);  // which set bit to pick

            // Create a mask that selects the k-th 1
            uint64_t bit = 1;
            uint64_t ones = x;

            // Mask all bits before the k-th set bit
            for (int i = 0; i < k; ++i) {
                // clear the lowest set bit
                ones &= (ones - 1);
            }

            // The lowest set bit now is the k-th set bit
            bit = ones & -ones;
            return bit;
        }

        std::string convertToSquare(uint64_t move) {
            std::string columns = "ABCDEFGH"; 

            int shifts = 0; 
            while (move != 1ULL) {
                move = move >> 1; 
                shifts += 1; 
            }

            char row = '0' + ((shifts / 8) + 1); 
            char column = columns.at(shifts % 8); 

            return std::format("{}{}", column, row);
        }

        void parse(std::string line) {
            std::stringstream ss(line);

            std::string command_type;
            std::string colour; 
            std::string move; 

            char del = ' '; 

            std::getline(ss, command_type, del); 
            std::getline(ss, colour, del); 

            if (command_type == "play") {
                std::getline(ss, move, del);
                game.makeMove(move);
                std::cout << "success\n";
            }

            else if (command_type == "genmove") {
                uint64_t moves; 
                if (colour == "black") {
                    moves = generateMoves(game.black, game.white);
                }
                else if (colour == "white")
                {
                    moves = generateMoves(game.white, game.black);
                }
                
                uint64_t move = pickRandomSetBit(moves);
                
                if (move == 0) {
                    std::cout << "pass\n";
                }

                std::cout << std::format("{}\n", convertToSquare(move));
            }
        }

        void play() {
            while(true) {
                std::string line; 
                std::getline(std::cin, line);
                
                parse(line);
            }
        }
};

int main() {
    RandomClient client{}; 
    client.play(); 
}