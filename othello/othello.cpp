#include <iostream>
#include <cstdint>
#include "othello.h"
#include <vector>

int directions[8] = {1, 7, 8, 9, -1, -7, -8, -9};
uint64_t corners = 0x8100000000000081;

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
    
    uint64_t empty = ~(playerBoard | opponentBoard);
    uint64_t moves = 0; 

    for (int i = 0; i < 8; i++) {
        int direction = directions[i];
        
        uint64_t charge = shift(playerBoard, direction) & opponentBoard;
        for (int j = 0; j < 5; j++) {
            charge |= shift(charge, direction) & opponentBoard;
        }
        uint64_t directionMoves = shift(charge, direction) & empty;
        moves |= directionMoves;
    }

    return moves;
}


uint64_t* makeMove(uint64_t playerBoard, uint64_t opponentBoard, uint64_t move) {
    playerBoard |= move; 

    for (int i = 0; i < 8; i++) {
        int direction = directions[i];

        uint64_t captured = shift(move, direction) & opponentBoard;
        for (int j = 0; j < 5; j++) {
            captured |= shift(captured, direction) & opponentBoard; 
        }
        if ((shift(captured, direction) & playerBoard) != 0) {
            playerBoard |= captured;
            opponentBoard &= ~captured; 
        }
    }
    return new uint64_t[]{playerBoard, opponentBoard};
}

// Takes a move, e.g a1 and returns bitboard representation 00...01
uint64_t moveToBitboard(std::string move) {
    std::string columns = "abcdefgh";

    char column = move.at(0); 
    char row = move.at(1); 
    
    int position = ((row - '0') - 1)*8 + columns.find(column, 0);

    uint64_t moveBoard = 1ULL << position;

    return moveBoard; 
}

uint64_t leastSignificantBit(uint64_t x) {
    return x & (~x + 1);
}

std::vector<uint64_t> separateMoves(uint64_t moves) {
    std::vector<uint64_t> separatedMoves; 

    while (moves) {
        uint64_t move = leastSignificantBit(moves); 
        separatedMoves.push_back(move);
        moves = moves ^ move; 
    }
    return separatedMoves;
}

uint64_t stableDiscs(uint64_t playerBoard, uint64_t opponentBoard) {return 0;}


// Returns true if the disc specified by move is placed in a region with an odd number of empty tiles, and false otherwise 
bool oddParity(uint64_t playerBoard, uint64_t opponentBoard, uint64_t move) {
    uint64_t empty = ~(playerBoard | opponentBoard);

    uint64_t region = move; 

    while(true) {
        uint64_t oldRegion = region; 

        for (int i = 0; i < 8; i++) {
            int direction = directions[i]; 

            uint64_t shifted = shift(region, direction); 
            uint64_t shiftedEmpty = shifted & empty; 

            region |= shiftedEmpty; 
        }

        if (region == oldRegion) {
            break; 
        }
    }

    int count = __builtin_popcountll(region);

    return ((count % 2) != 0); 
}

// Returns the corners that may be captured as a result of a potential forced corner capture sequence defined by playerBoard, opponentBoard, and move. Returns 0 if there are no forced corner captures. 
uint64_t forcedCornerCaptureCorners(uint64_t playerBoard, uint64_t opponentBoard, uint64_t move) {
    uint64_t playerMovesFirst = generateMoves(playerBoard, opponentBoard);
    uint64_t playerMovesFirstCorners = playerMovesFirst & corners; 

    uint64_t* secondBoardState = makeMove(playerBoard, opponentBoard, move); 

    uint64_t secondPlayerBoard = secondBoardState[0];
    uint64_t secondOpponentBoard = secondBoardState[1]; 

    uint64_t playerMovesSecond = generateMoves(secondPlayerBoard, secondOpponentBoard); 
    uint64_t playerMovesSecondCorners = playerMovesSecond & corners; 

    // Corner moves that the active player could've played at the start, and that have persisted. Don't count these as being forced. 
    uint64_t originalCornerCaptureMoves = playerMovesFirstCorners & playerMovesSecondCorners; 

    // After playing move, check that opponent has 1 or less moves, and is therefore forced to potentially give up a corner
    uint64_t opponentMoves = generateMoves(secondOpponentBoard, secondPlayerBoard); 
    int numOpponentMoves = __builtin_popcountll(opponentMoves); 

    if (numOpponentMoves > 1) {
        return 0; 
    }

    // opponentMoves consists of either 0, or 1 move 
    uint64_t* thirdBoardState = makeMove(secondOpponentBoard, secondPlayerBoard, opponentMoves); 

    uint64_t thirdPlayerBoard = thirdBoardState[1]; 
    uint64_t thirdOpponentBoard = thirdBoardState[0]; 

    uint64_t playerMoves = generateMoves(thirdPlayerBoard, thirdOpponentBoard); 

    uint64_t playerMovesForcedCorners = playerMoves & (~originalCornerCaptureMoves); 

    return ((playerMovesForcedCorners & corners));
}

// Out of all possible moves the active player has, returns the ones which lead to a forced corner capture
uint64_t forcedCornerCaptures(uint64_t playerBoard, uint64_t opponentBoard) {
    std::vector<uint64_t> initialPlayerMoves = separateMoves(generateMoves(playerBoard, opponentBoard));

    uint64_t forcedCornerCaptureMoves = 0; 

    for (const auto move : initialPlayerMoves) {
        if (__builtin_popcountll(forcedCornerCaptureCorners(playerBoard, opponentBoard, move)) > 0) {
            forcedCornerCaptureMoves |= move; 
        }
    }
    return forcedCornerCaptureMoves; 
}

uint64_t frontierDiscs(uint64_t playerBoard, uint64_t opponentBoard) {
    uint64_t empty = ~(playerBoard | opponentBoard);

    uint64_t neighboursOfEmpty = 0; 

    for (int i = 0; i < 8; i++) {
        int direction = directions[i];
        neighboursOfEmpty |= shift(empty, direction);
    }

    return playerBoard & neighboursOfEmpty; 
}


/*
int main() {
    uint64_t black = 0x3d88000000; 
    uint64_t white = 0xff42767f1000; 

    uint64_t fcp = forcedCornerCaptures(black, white); 
    printBoard(fcp);

    return 0; 
}
*/
