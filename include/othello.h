#ifndef OTHELLO_H
#define OTHELLO_H

#include <cstdint>

uint64_t shift(uint64_t board, int shamt);
void printBoard(uint64_t board);
uint64_t* makeMove(uint64_t playerBoard, uint64_t opponentBoard, uint64_t move);
uint64_t generateMoves(uint64_t playerBoard, uint64_t opponentBoard); 


#endif