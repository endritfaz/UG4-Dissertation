#include <iostream>
#include <cstdint>
#include "othello.h"

// Shifts by shamt, postive shamt means moving right on board
uint64_t shift(uint64_t board, int shamt) {
    uint64_t westMask = 0xFEFEFEFEFEFEFEFE;
    uint64_t eastMask = 0x7F7F7F7F7F7F7F7F;
    
    if (shamt < 0) {
        shamt *= -1; 
        // Don't allow bits to roll over if moving west
        if (shamt == 1 || shamt == 9) {
            board &= westMask; 
        }
        else if (shamt == 7) {
            board &= eastMask; 
        }
        board = board >> shamt; 
    }

    else {
        // Don't allow bits to roll over if moving east or north
        if (shamt == 1 || shamt == 9) {
            board &= eastMask;
        }
        else if (shamt == 7) {
            board &= westMask; 
        }
        board = board << shamt; 
    }
    return board; 
}

void printBoard(uint64_t board) {
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << (rank + 1) << " ";
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            if (board & (1ULL << square)) {
                std::cout << "1 ";
            } else {
                std::cout << ". ";
            }
        }
        std::cout << "\n";
    }
    std::cout << "  A B C D E F G H\n";
}

uint64_t generateMoves(uint64_t playerBoard, uint64_t opponentBoard) {
    int directions[8] = {1, 7, 8, 9, -1, -7, -8, -9};
    
    uint64_t empty = ~(playerBoard | opponentBoard);
    uint64_t moves = 0; 

    for (int i = 0; i < 8; i++) {
        int direction = directions[i];
        
        uint64_t charge = shift(playerBoard, direction) & opponentBoard;
        for (int j = 0; j < 4; j++) {
            charge |= shift(charge, direction) & opponentBoard;
        }
        uint64_t directionMoves = shift(charge, direction) & empty;
        moves |= directionMoves;
    }

    return moves;
}

int main() {
    uint64_t playerBoard = 0x40000000000; 
    uint64_t opponentBoard = 0x180000000000; 
    
    uint64_t moves = generateMoves(playerBoard, opponentBoard);

    printBoard(playerBoard);
    printBoard(opponentBoard);
    printBoard(moves);

    return 0; 
}
