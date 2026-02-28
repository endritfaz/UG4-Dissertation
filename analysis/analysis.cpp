#include "nlohmann/json.hpp"
#include "Game.h"
#include <iostream>
#include <fstream>
#include <fmt/format.h>

using json = nlohmann::json;

struct discsPerMoveInfo {
    std::vector<int> piecesPerMoveBlack = std::vector<int>(60); 
    std::vector<int> piecesPerMoveWhite = std::vector<int>(60); 
    int placed = 0; 
};

discsPerMoveInfo calcDiscMove(json jgame) {
    discsPerMoveInfo discInfo; 

    Game game{};
    std::vector<std::string> moves = jgame["moves"];

    int placed = 0; 
    for (int i = 0; i < moves.size(); i++) {
        game.makeMove(moves[i]); 

        if (moves[i] == "pass" || moves[i] == "PASS") {
            continue;
        }

        placed += 1; 
        // placed - 1 shouldn't be negative because first move can't pass
        discInfo.piecesPerMoveBlack[placed - 1] = game.countBlackPieces(); 
        discInfo.piecesPerMoveWhite[placed - 1] = game.countWhitePieces(); 
    }
    discInfo.placed = placed; 

    return discInfo; 
}

discsPerMoveInfo calcAvailableMoves(json jgame) {
    discsPerMoveInfo discInfo; 

    Game game{};
    std::vector<std::string> moves = jgame["moves"];

    int placed = 0; 
    for (int i = 0; i < moves.size(); i++) {
        game.makeMove(moves[i]); 

        if (moves[i] == "pass" || moves[i] == "PASS") {
            continue;
        }

        placed += 1; 
        // placed - 1 shouldn't be negative because first move can't pass
        discInfo.piecesPerMoveBlack[placed - 1] = game.countMoves("black");
        discInfo.piecesPerMoveWhite[placed - 1] = game.countMoves("white");
    }
    discInfo.placed = placed; 

    return discInfo; 
}

discsPerMoveInfo calcFrontierMove(json jgame) {
    discsPerMoveInfo discInfo; 

    Game game{};
    std::vector<std::string> moves = jgame["moves"];

    int placed = 0; 
    for (int i = 0; i < moves.size(); i++) {
        game.makeMove(moves[i]); 

        if (moves[i] == "pass" || moves[i] == "PASS" || moves[i] == "Resign") {
            continue;
        }

        placed += 1; 
        // placed - 1 shouldn't be negative because first move can't pass
        discInfo.piecesPerMoveBlack[placed - 1] = game.countFrontierDiscs("black");
        discInfo.piecesPerMoveWhite[placed - 1] = game.countFrontierDiscs("white");
    }
    discInfo.placed = placed; 

    return discInfo; 
}

void calcFeatureAverage(json games, std::function<discsPerMoveInfo(json)> func, std::string feature, std::string model) {
    std::vector<int> totalPiecesPerMoveBlack(60);
    std::vector<int> totalPiecesPerMoveWhite(60);

    std::vector<float> avgPiecesPerMoveBlack(60);
    std::vector<float> avgPiecesPerMoveWhite(60);

    std::vector<int> samplesPerMove(60);
    
    for (int i = 0; i < games.size(); i++) {
        discsPerMoveInfo discInfo = func(games[i]);

        for (int i = 0; i < totalPiecesPerMoveBlack.size(); i++) {
            if (i <= discInfo.placed - 1) {
                samplesPerMove[i] += 1;
            }
            totalPiecesPerMoveBlack[i] += discInfo.piecesPerMoveBlack[i];
            totalPiecesPerMoveWhite[i] += discInfo.piecesPerMoveWhite[i];
        }
    }

    for (int i = 0; i < totalPiecesPerMoveBlack.size(); i++) {
        if (samplesPerMove[i] == 0) {
            break; 
        }

        avgPiecesPerMoveBlack[i] = (float) totalPiecesPerMoveBlack[i] / samplesPerMove[i];
        avgPiecesPerMoveWhite[i] = (float) totalPiecesPerMoveWhite[i] / samplesPerMove[i];
    }

    // Account for starting position
    avgPiecesPerMoveBlack.insert(avgPiecesPerMoveBlack.begin(), 2.0);
    avgPiecesPerMoveWhite.insert(avgPiecesPerMoveWhite.begin(), 2.0); 

    json j; 
    
    j[fmt::format("avg_{}_per_move_black", feature)] = avgPiecesPerMoveBlack; 
    j[fmt::format("avg_{}_per_move_white", feature)] = avgPiecesPerMoveWhite; 

    // Write JSON to file with indent of 4 spaces 
    std::ofstream o(fmt::format("avg-{}-{}.json", feature, model));
    o << std::setw(4) << j << std::endl; 
}

bool forcedCornerCapture(std::string move, json prevMoveFeature) {
    if (!isCornerMove(move)) {
        return false;
    }

    return (prevMoveFeature["black_active"] && (prevMoveFeature["num_moves_black"] == 1 || prevMoveFeature["num_moves_black"] == 0) || !prevMoveFeature["black_active"] && (prevMoveFeature["num_moves_white"] == 1 || prevMoveFeature["num_moves_white"] == 0));
}

std::vector<json> extractMoveFeatures(std::vector<std::string> moves) {
    std::vector<json> movesFeatures{}; 
    Game game{};
    int ply = 0; 

    for (auto move : moves) {
        json moveFeatures; 

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

        moveFeatures["ply"] = ply;
        moveFeatures["num_discs"] = game.countWhitePieces() + game.countBlackPieces(); 
        moveFeatures["num_discs_black"] = game.countBlackPieces();
        moveFeatures["num_discs_white"] = game.countWhitePieces();
        moveFeatures["num_moves_black"] = game.countMoves("black"); 
        moveFeatures["num_moves_white"] = game.countMoves("white");
        moveFeatures["num_frontier_black"] = game.countFrontierDiscs("black");
        moveFeatures["num_frontier_white"] = game.countFrontierDiscs("white");

        movesFeatures.push_back(moveFeatures);
    }

    return movesFeatures;
}
