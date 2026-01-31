#include <iostream>
#include <cstdint>
#include "Game.h"
#include "othello.h"
#include <string>

Game::Game():
    black {0x1008000000},
    white {0x810000000},
    turn {0}, 
    resign {false},
    winner {""}
    {}    

int Game::countWhitePieces() {
    return __builtin_popcountll(white);
}

int Game::countBlackPieces() {
    return __builtin_popcountll(black);
}

int Game::countMoves(std::string colour) {
    uint64_t targetBoard; 
    uint64_t otherBoard;

    if (colour == "black") {
        targetBoard = black; 
        otherBoard = white; 
    }

    else if (colour == "white") {
        targetBoard = white; 
        otherBoard = black; 
    }

    else {
        exit(EXIT_FAILURE); 
    }

    int targetMoveCount = __builtin_popcountll(generateMoves(targetBoard, otherBoard));

    return targetMoveCount; 
}

int Game::countFrontierDiscs(std::string colour) {
    uint64_t targetBoard; 
    uint64_t otherBoard;

    if (colour == "black") {
        targetBoard = black; 
        otherBoard = white; 
    }

    else if (colour == "white") {
        targetBoard = white; 
        otherBoard = black; 
    }

    else {
        exit(EXIT_FAILURE); 
    }

    int targetMoveCount = __builtin_popcountll(frontierDiscs(targetBoard, otherBoard));

    return targetMoveCount; 
}

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
    if (move == "pass" || move == "PASS") {
        turn += 1;
        return; 
    }

    else if (move == "Resign") {
        resign = true;

        if (turn % 2 == 0) {
            winner = "white"; 
        }

        else {
            winner = "black";
        }

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

    if (resign) {
        return true; 
    }

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
    int blackDiscs = __builtin_popcountll(black);
    int whiteDiscs = __builtin_popcountll(white);

    if (resign = true) {
        return winner; 
    }
    
    if (blackDiscs > whiteDiscs) {
        return "black";
    }

    if (whiteDiscs > blackDiscs) {
        return "white";
    }

    return "draw"; 
}
