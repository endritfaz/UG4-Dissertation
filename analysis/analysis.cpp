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

        if (moves[i] == "pass" || moves[i] == "PASS") {
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

int main() {
    std::string feature = "discs"; 
    std::string model = "az1000";
    std::ifstream i("games-az1000-random.json");
    json j; 
    i >> j; 

    json games = j["games"];

    calcFeatureAverage(games, &calcDiscMove, feature, model);
}