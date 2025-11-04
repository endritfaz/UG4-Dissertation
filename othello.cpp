#include <iostream>

int main() {
    std::cout << "Hello World!";
}

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
}

void printBoard(uint64_t board) {

}

uint64_t generateMoves(uint64_t playerBoard, uint64_t opponentBoard) {
    int DIRECTIONS[8]
    return -1; 
}

