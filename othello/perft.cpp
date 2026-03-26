#include <iostream>
#include <vector>
#include <fmt/format.h> 
#include "othello.h"

void perft(int depth, int current_depth, uint64_t active, uint64_t inactive, int* moves) {
    
    if (current_depth > depth) {
        return; 
    }

    uint64_t depth_moves_bitboard = generateMoves(active, inactive);

    if (depth_moves_bitboard == 0) {
        moves[current_depth] += 1; 
        perft(depth, current_depth+1, inactive, active, moves);
        return; 
    }

    std::vector<uint64_t> depth_moves = separateMoves(depth_moves_bitboard); 

    moves[current_depth] += depth_moves.size(); 

    for (const auto move : depth_moves) {
        uint64_t* new_boards = makeMove(active, inactive, move);

        uint64_t new_active = new_boards[1];
        uint64_t new_inactive= new_boards[0];

        delete[] new_boards;

        perft(depth, current_depth+1, new_active, new_inactive, moves);
    }
}

void perft_caller(int depth) {
    if (depth > 256) {
        return;
    }

    int moves[depth+1] = {0}; 

    uint64_t start_black = 0x1008000000; 
    uint64_t start_white = 0x810000000; 
    perft(depth, 1, start_black, start_white, moves);

    for (int i = 1; i <= depth; i++) {
        std::cout << fmt::format("1 -> {}", moves[i]) << std::endl; 
    }
}

int main() {
    perft_caller(11);
    return 0; 
}