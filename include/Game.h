#ifndef GAME_H
#define GAME_H

#include <cstdint>
#include <string>

bool isCornerMove(std::string move);

class Game {
public:
    uint64_t black;
    uint64_t white;
    int turn;
    bool resign; 
    std::string winner; 

    Game();
    int countWhitePieces(); 
    int countBlackPieces(); 
    int countMoves(std::string colour); 
    int countFrontierDiscs(std::string colour);
    void makeMove(uint64_t move);
    void makeMove(std::string move);
    bool gameOver(); 
    std::string getWinner(); 
    bool validateMove(std::string move);
    bool oddParity(std::string move);
    bool forcedCornerCapturePossible();
    bool forcedCornerCaptureExecuted(std::string initialMove, std::string cornerCaptureMove);
    int countStableDiscs(std::string colour);
};

#endif
