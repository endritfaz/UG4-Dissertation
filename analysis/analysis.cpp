#include "nlohmann/json.hpp"
#include "Game.h"
#include <iostream>
#include <fstream>
#include <fmt/format.h>

using json = nlohmann::json;

bool gameFull(std::vector<std::string> moves) {
    std::vector<json> positionsFeatures{}; 

    Game game{}; 

    for (auto move : moves) {
        game.makeMove(move);
    }

    return ((game.black | game.white) == 0xFFFFFFFFFFFFFFFF);
}

std::vector<json> extractPositionFeatures(std::vector<std::string> moves) {
    std::vector<json> positionsFeatures{}; 

    Game game{}; 

    for (auto move : moves) {
        json positionFeatures; 

        positionFeatures["black"] = game.black; 
        positionFeatures["white"] = game.white; 
        positionFeatures["active"] = game.turn % 2 == 0 ? "black" : "white"; 

        positionsFeatures.push_back(positionFeatures); 

        game.makeMove(move);
    }

    // Last board state 
    json positionFeatures; 
    positionFeatures["black"] = game.black; 
    positionFeatures["white"] = game.white; 
    positionFeatures["active"] = game.turn % 2 == 0 ? "black" : "white"; 

    positionsFeatures.push_back(positionFeatures); 

    return positionsFeatures;
}

std::vector<json> extractMoveFeatures(std::vector<std::string> moves) {
    std::vector<json> movesFeatures{}; 
    Game game{};
    int ply = 0; 

    for (auto move : moves) {
        json moveFeatures; 
        
        moveFeatures["pass"] = move == "pass" ? true : false; 
        
        // black_active means black played the move
        moveFeatures["black_active"] = (game.turn % 2 == 0);
        moveFeatures["parity"] = game.oddParity(move);
        
        bool fcpsPossible = game.forcedCornerCapturePossible(); 
        moveFeatures["forced_corner_capture_possible"] = fcpsPossible; 
        
        // Check if the corner capture was actually executed. That is, there was a corner capture sequence available, and the relevant corner was captured on move ply + 2 
    
        int cornerCapturePly = ply + 2;
        
        bool fcpsExecuted = false;
        if (fcpsPossible) {
            if (ply + 2 < moves.size()) {
                std::string cornerCaptureMove = moves[cornerCapturePly];
                
                // Needs to be a real move as it will be converted to bitboard
                if (cornerCaptureMove != "pass" && cornerCaptureMove != "resign") {
                    fcpsExecuted = game.forcedCornerCaptureExecuted(move, cornerCaptureMove); 
                }
            }
        }

        moveFeatures["forced_corner_capture_executed"] = fcpsExecuted;
      
        game.makeMove(move);
        ply += 1; 

        moveFeatures["black_board"] = game.black; 
        moveFeatures["white_board"] = game.white;
        moveFeatures["ply"] = ply;
        moveFeatures["num_discs"] = game.countWhitePieces() + game.countBlackPieces(); 
        moveFeatures["num_discs_black"] = game.countBlackPieces();
        moveFeatures["num_discs_white"] = game.countWhitePieces();
        moveFeatures["num_moves_black"] = game.countMoves("black"); 
        moveFeatures["num_moves_white"] = game.countMoves("white");
        moveFeatures["num_frontier_black"] = game.countFrontierDiscs("black");
        moveFeatures["num_frontier_white"] = game.countFrontierDiscs("white");
        moveFeatures["num_stable_black"] = game.countStableDiscs("black");
        moveFeatures["num_stable_white"] = game.countStableDiscs("white"); 
        
        movesFeatures.push_back(moveFeatures);
    }

    return movesFeatures;
}
