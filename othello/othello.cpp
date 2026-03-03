#include <iostream>
#include <cstdint>
#include "othello.h"
#include <vector>

int directions[8] = {1, 7, 8, 9, -1, -7, -8, -9};
int lineDirections[2] = {1, -1}; 

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

static inline uint8_t shiftLine(uint8_t line, int shamt) {
    if (shamt > 0) {
        return line << shamt; 
    }
    return line >> (-shamt);   
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

void printLine(uint8_t line) {
    uint8_t mask = 0x01; 
    for (int i = 0; i < 8; i++) {
        uint8_t p = mask & line; 

        if (p == 1) {
            std::cout << "0";
        }
        else {
            std::cout << ".";
        }
        line = line >> 1; 
    }
    std::cout << "\n";
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

uint8_t* makeLineMove(uint8_t playerLine, uint8_t opponentLine, uint8_t move) {
    playerLine |= move; 

    for (int i = 0; i < 2; i++) {
        int direction = lineDirections[i];

        uint64_t captured = shiftLine(move, direction) & opponentLine;
        for (int j = 0; j < 5; j++) {
            captured |= shiftLine(captured, direction) & opponentLine; 
        }
        if ((shiftLine(captured, direction) & playerLine) != 0) {
            playerLine |= captured;
            opponentLine &= ~captured; 
        }
    }
    return new uint8_t[]{playerLine, opponentLine};
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

std::vector<uint8_t> separateLineMoves(uint8_t moves) {
    std::vector<uint8_t> separatedMoves; 

    while (moves) {
        uint8_t move = moves & (~moves + 1); 
        separatedMoves.push_back(move);
        moves = moves ^ move; 
    }
    return separatedMoves;
}
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

uint8_t extractLeftEdge(uint64_t board) {
    uint64_t leftEdgeMask = 0x101010101010101; 

    board &= leftEdgeMask; 

    uint8_t line = 0; 

    for (int i = 0; i < 8; i++) {
        line |= ((board >> (i*8)) & 1ULL) << i; 
    }

    return line; 
}

uint64_t insertLeftEdge(uint8_t line) {
    uint64_t board = 0; 

    for (int i = 0; i < 8; i++) {
        board |= ((line >> i) & 1ULL) << i*8; 
    }

    return board; 
}

uint8_t extractRightEdge(uint64_t board) {
    uint64_t rightEdgeMask = 0x8080808080808080; 

    board &= rightEdgeMask; 

    uint8_t line = 0; 

    for (int i = 0; i < 8; i++) {
        line |= ((board >> ((i*8) + 7)) & 1ULL) << i; 
    }

    return line; 
}

uint64_t insertRightEdge(uint8_t line) {
    uint64_t board = 0; 

    for (int i = 0; i < 8; i++) {
        board |= ((line >> i) & 1ULL) << (i*8 + 7); 
    }

    return board; 
}

uint8_t stableEdge(uint8_t playerEdge, uint8_t opponentEdge) {
    uint8_t empty = ~(playerEdge | opponentEdge); 
    std::vector<uint8_t> moves = separateLineMoves(empty);

    uint8_t res = playerEdge | opponentEdge; 

    if (res == 0) {
        return 0; 
    }

    for (const auto move : moves) {
        uint8_t* newEdge = makeLineMove(playerEdge, opponentEdge, move); 
        // Remove opponent edges that got flipped
        res &= playerEdge | newEdge[1]; 

        if (res == 0) return 0; 
        res &= stableEdge(newEdge[0], newEdge[1]);

        newEdge = makeLineMove(opponentEdge, playerEdge, move); 
        res &= opponentEdge | newEdge[1]; 

        if (res == 0) return 0; 
        res &= stableEdge(newEdge[1], newEdge[0]);
    }
    return res; 
}

uint64_t stableEdges(uint64_t playerBoard, uint64_t opponentBoard) {
    uint64_t stableEdges = 0; 

    uint64_t topMask = 0xff00000000000000; 
    uint64_t bottomMask = 0xff; 

    uint8_t topPlayer = (playerBoard & topMask) >> 56; 
    uint8_t topOpponent = (opponentBoard & topMask) >> 56;

    uint8_t bottomPlayer = (playerBoard & bottomMask); 
    uint8_t bottomOpponent = (opponentBoard & bottomMask); 

    uint64_t leftPlayer = extractLeftEdge(playerBoard); 
    uint64_t leftOpponent = extractLeftEdge(opponentBoard); 

    uint64_t rightPlayer = extractRightEdge(playerBoard);
    uint64_t rightOpponent = extractRightEdge(opponentBoard);

    uint8_t stableTop = stableEdge(topPlayer, topOpponent);
    uint8_t stableBottom = stableEdge(bottomPlayer, bottomOpponent); 
    uint8_t stableLeft = stableEdge(leftPlayer, leftOpponent);
    uint8_t stableRight = stableEdge(rightPlayer, rightOpponent); 

    stableEdges |= (uint64_t) stableTop << 56;
    stableEdges |= (uint64_t) stableBottom; 
    stableEdges |= insertLeftEdge(stableLeft); 
    stableEdges |= insertRightEdge(stableRight); 

    return stableEdges; 
}

uint64_t fullVertical(uint64_t board) {
    board &= shift(board, -8) | shift(board, 56);
    board &= shift(board, -16) | shift(board, 48);
    board &= shift(board, -32) | shift(board, 32);
    return board;
}

uint64_t fullHorizontal(uint64_t board) {
    board &= shift(board, -1);
    board &= shift(board, -2);
    board &= shift(board, -4); 

    return (board & 0x0101010101010101) * 0xFF;
}

// Full diagonal functions only work for internal squares 
uint64_t fullDiagonal7(uint64_t board) {
    constexpr uint64_t edge = 0xFF818181818181FF;
    uint64_t l7, r7;

    l7 = r7 = board;

    l7 &= edge | shift(l7, -7);        
    r7 &= edge | shift(r7, 7);

    l7 &= 0xFFFF030303030303ULL | shift(l7, -14);    
    r7 &= 0xC0C0C0C0C0C0FFFFULL | shift(r7, 14);

    l7 &= 0xFFFFFFFF0F0F0F0FULL | shift(l7, -28);    
    r7 &= 0xF0F0F0F0FFFFFFFFULL | shift(r7, 28);

    return l7 & r7;
}

uint64_t fullDiagonal9(uint64_t board) {
    constexpr uint64_t edge = 0xFF818181818181FF;
    uint64_t l9, r9;

    l9 = r9 = board;

    l9 &= edge | shift(l9, -9);        
    r9 &= edge | shift(r9, 9);

    l9 &= 0xFFFFC0C0C0C0C0C0ULL | shift(l9, -18);    
    r9 &= 0x030303030303FFFFULL | shift(r9, 18);

    return l9 & r9 & (0x0F0F0F0FF0F0F0F0ULL | shift(l9, -36) | shift(r9, 36));
}

/*
1) Find stable discs on edges 

2) Find discs that are stable because the lines running through the disc in all four directions are full 

3) Expand interior stable discs are stable in each of the four directions. A disc is stable in a direction if it is adjacent to a stable disc in that direction, or it is full in that direction. 

Suppose it wasn't stable in that direction. Then the stable disc it is adjacent to would have to be flanked, and captured, contradicting the assumption that it was stable. But that disk could only have been marked stable because it a neighbour of another stable disc, or it was marked stable by (2) or (3), and these discs are definitely stable. 

Hence, the algorithm does not produce false positives 

Idea from https://github.com/Nyanyan/Egaroucid/blob/main/src/engine/stability.hpp 
*/
uint64_t stableDiscs(uint64_t playerBoard, uint64_t opponentBoard) {
    const uint64_t playerInternal = playerBoard & 0x007E7E7E7E7E7E00ULL;

    uint64_t stableE = stableEdges(playerBoard, opponentBoard); 

    uint64_t fullV = fullVertical(playerBoard | opponentBoard); 
    uint64_t fullH = fullHorizontal(playerBoard | opponentBoard); 
    uint64_t fullD7 = fullDiagonal7(playerBoard | opponentBoard); 
    uint64_t fullD9 = fullDiagonal9(playerBoard | opponentBoard); 

    // Edge stable discs 
    uint64_t stableDiscs = stableE; 

    // Internal stable discs because all the lines through them are full 
    stableDiscs |= (fullV & fullH & fullD7 & fullD9); 

    stableDiscs &= playerBoard; 

    uint64_t finalStableDiscs = 0; 
    uint64_t verticalStable, horizontalStable, diagonal7Stable, diagonal9Stable; 

    // stableDiscs is updated in the loop. Check if any new stable discs have been discovered
    while (stableDiscs & ~finalStableDiscs) {
        printBoard(stableDiscs); 
        finalStableDiscs |= stableDiscs; 

        verticalStable = shift(finalStableDiscs, 8) | shift(finalStableDiscs, -8) | fullV; 
        horizontalStable = shift(finalStableDiscs, 1) | shift(finalStableDiscs, -1) | fullH; 
        diagonal7Stable = shift(finalStableDiscs, 7) | shift(finalStableDiscs, -7) | fullD7; 
        diagonal9Stable = shift(finalStableDiscs, 9) | shift(finalStableDiscs, -9) | fullD9; 
        
        stableDiscs = verticalStable & horizontalStable & diagonal7Stable & diagonal9Stable & playerInternal; 


    }
    return finalStableDiscs;
}

/*
int main() {
    uint64_t testb = 0x9f1f131150141090; 
    uint64_t testw = 0; 
    printBoard(stableDiscs(testb, testw));
}
*/