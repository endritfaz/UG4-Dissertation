#ifndef ANALYSIS_H
#define ANALYSIS_H

#include <cstdint>
#include <string>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

std::vector<json> extractMoveFeatures(std::vector<std::string> moves);
std::vector<json> extractPositionFeatures(std::vector<std::string> moves);
bool gameFull(std::vector<std::string> moves); 

#endif