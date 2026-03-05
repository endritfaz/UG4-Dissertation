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
    if (move == "pass") {
        turn += 1;
        return; 
    }

    if (move == "resign") {
        resign = true;

        if (turn % 2 == 0) {
            winner = "white"; 
        }

        else {
            winner = "black";
        }
        return; 
    }

    uint64_t moveBoard = moveToBitboard(move);
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

    if (resign) {
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

bool Game::validateMove(std::string move) {
    if (move == "resign") {
        return true; 
    }
    
    uint64_t moveBoard = moveToBitboard(move); 

    uint64_t activePlayer; 
    uint64_t inactivePlayer; 

    if (turn % 2 == 0) {
        activePlayer = black; 
        inactivePlayer = white; 
    }

    else {
        activePlayer = white; 
        inactivePlayer = black; 
    }

    if (move == "pass") {
        return generateMoves(activePlayer, inactivePlayer) == 0; 
    }
    
    return (generateMoves(activePlayer, inactivePlayer) & moveBoard) != 0;
}

bool Game::oddParity(std::string move) {
    uint64_t moveBoard = moveToBitboard(move);

    uint64_t activePlayer; 
    uint64_t inactivePlayer; 

    if (turn % 2 == 0) {
        activePlayer = black; 
        inactivePlayer = white; 
    }

    else {
        activePlayer = white; 
        inactivePlayer = black; 
    }

    return (::oddParity(activePlayer, inactivePlayer, moveBoard));
}

bool Game::forcedCornerCapturePossible() {
    uint64_t activePlayer; 
    uint64_t inactivePlayer; 

    if (turn % 2 == 0) {
        activePlayer = black; 
        inactivePlayer = white; 
    }

    else {
        activePlayer = white; 
        inactivePlayer = black; 
    }

    return (forcedCornerCaptures(activePlayer, inactivePlayer) > 0); 
}

bool Game::forcedCornerCaptureExecuted(std::string initialMove, std::string cornerCaptureMove) {
    uint64_t activePlayer; 
    uint64_t inactivePlayer; 

    if (turn % 2 == 0) {
        activePlayer = black; 
        inactivePlayer = white; 
    }

    else {
        activePlayer = white; 
        inactivePlayer = black; 
    }

    uint64_t initialMoveBitboard = moveToBitboard(initialMove); 
    uint64_t cornerCaptureMoveBitboard = moveToBitboard(cornerCaptureMove); 

    return ((forcedCornerCaptureCorners(activePlayer, inactivePlayer, initialMoveBitboard) & cornerCaptureMoveBitboard) > 0); 
}

int Game::countStableDiscs(std::string colour) {
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

    return __builtin_popcountll(stableDiscs(targetBoard, otherBoard)); 
}

bool isCornerMove(std::string move) {
    return (move == "a1" || move == "a8" || move == "h1" || move == "h8");
}