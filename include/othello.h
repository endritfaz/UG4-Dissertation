#ifndef OTHELLO_H
#define OTHELLO_H

#include <cstdint>
#include <string>

uint64_t shift(uint64_t board, int shamt);
void printBoard(uint64_t board);
uint64_t* makeMove(uint64_t playerBoard, uint64_t opponentBoard, uint64_t move);
uint64_t generateMoves(uint64_t playerBoard, uint64_t opponentBoard); 
uint64_t stableDiscs(uint64_t whiteBoard, uint64_t blackBoard, std::string colour);


#endif
