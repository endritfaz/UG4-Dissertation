#include <iostream>
#include <cstdint>
#include "Game.h"
#include "othello.h"
#include <string>


Game::Game():
    black {0x1008000000},
    white {0x810000000},
    turn {0}
    {}    

void Game::makeMove(uint64_t move) {
    uint64_t* boards; 

    if (turn % 2 == 0) {
        boards = ::makeMove(black, white, move); 
        black = boards[0];
        white = boards[1];
    }

    else {
        boards = ::makeMove(white, black, move); 
        black = boards[1];
        white = boards[0]; 
    }

    turn += 1; 
}

void Game::makeMove(std::string move) {
    if (move == "pass") {
        turn += 1;
        return; 
    }

    std::string columns = "ABCDEFGH";

    char column = move.at(0); 
    char row = move.at(1); 
    
    int position = ((row - '0') - 1)*8 + columns.find(column, 0);

    uint64_t moveBoard = 1ULL << position; 

    uint64_t* boards; 

    if (turn % 2 == 0) {
        boards = ::makeMove(black, white, moveBoard); 
        black = boards[0];
        white = boards[1];
    }

    else {
        boards = ::makeMove(white, black, moveBoard); 
        black = boards[1];
        white = boards[0]; 
    }

    turn += 1; 
}

bool Game::gameOver() {
    uint64_t full = 0xFFFFFFFF;

    if ((black | white) == full) {
        return true; 
    }

    if (generateMoves(black, white) == 0 && generateMoves(white, black) == 0) {
        return true;
    }

    return false; 
}

// Assumes the game is over
std::string Game::getWinner() {
    int blackDiscs = std::popcount(black);
    int whiteDiscs = std::popcount(white);

    if (blackDiscs > whiteDiscs) {
        return "black";
    }

    else if (whiteDiscs > blackDiscs) {
        return "white";
    }

    return "draw"; 
}
