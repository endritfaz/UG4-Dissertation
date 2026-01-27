#ifndef GAME_H
#define GAME_H

#include <cstdint>
#include <string>

class Game {
public:
    uint64_t black;
    uint64_t white;
    int turn;

    Game();
    int countWhitePieces(); 
    int countBlackPieces(); 
    int countMoves(std::string colour); 
    int countFrontierDiscs(std::string colour);
    void makeMove(uint64_t move);
    void makeMove(std::string move);
    bool gameOver(); 
    std::string getWinner(); 

};

#endif
