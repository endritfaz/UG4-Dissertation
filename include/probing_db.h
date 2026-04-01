#include <iostream>
#include <fstream>
#include <pqxx/pqxx> 

#include "nlohmann/json.hpp"
#include "helper.h"
#include "analysis.h"
#include "othello.h"

using namespace pqxx;

bool init_positions_table(connection& c);
bool init_activations_table(connection& c);
bool init_label_discs_table(connection& c);
bool init_label_stable_table(connection& c);
bool init_results_table(connection& c); 
bool init_label_frontier_table(connection& c);
bool init_label_fcc_table(connection& c); 

void prepare_position_insert(connection& c);
void prepare_label_discs_total_insert(connection& c);
void prepare_stable_discs_insert(connection& c);
void prepare_frontier_discs_insert(connection& c);
void prepare_fcc_insert(connection& c);