#include <iostream>
#include <fstream>
#include <pqxx/pqxx> 

#include "nlohmann/json.hpp"
#include "helper.h"
#include "analysis.h"
#include "othello.h"

using namespace pqxx;
using json = nlohmann::json;

bool init_positions_table(connection& c) {
    std::string remove_positions_table_sql = "DROP TABLE IF EXISTS positions CASCADE";

    std::string create_positions_table_sql = "CREATE TABLE positions(" \
                                    "id INT GENERATED ALWAYS AS IDENTITY PRIMARY KEY," \
                                    "black BYTEA NOT NULL," \
                                    "white BYTEA NOT NULL," \
                                    "turn VARCHAR(255) NOT NULL," \
                                    "UNIQUE (black, white, turn));"; 

    
    try {
        work tx(c); 
        tx.exec(remove_positions_table_sql);
        tx.exec(create_positions_table_sql);
        tx.commit(); 

        return true;
    }

    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return false; 
    }
}

bool init_results_table(connection& c) {
    std::string remove_results_table_sql = "DROP TABLE IF EXISTS activations";

    std::string create_results_table_sql = "CREATE TABLE results(" \
                                    "iteration VARCHAR(255) NOT NULL," \
                                    "block_1_coeff BYTEA NOT NULL," \
                                    "block_2_coeff BYTEA NOT NULL," \
                                    "block_3_coeff BYTEA NOT NULL," \
                                    "concept VARCHAR(255) NOT NULL);";

    try {
        work tx(c); 
        tx.exec(remove_results_table_sql);
        tx.exec(create_results_table_sql);
        tx.commit(); 

        return true;
    }

    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return false; 
    }
}

bool init_activations_table(connection& c) {
    std::string remove_activations_table_sql = "DROP TABLE IF EXISTS activations";

    std::string create_positions_table_sql = "CREATE TABLE activations(" \
                                    "position_id INT REFERENCES positions(id)," \
                                    "block_1 BYTEA NOT NULL," \
                                    "block_2 BYTEA NOT NULL," \
                                    "block_3 BYTEA NOT NULL," \
                                    "iteration VARCHAR(255) NOT NULL);";

    try {
        work tx(c); 
        tx.exec(remove_activations_table_sql);
        tx.exec(create_positions_table_sql);
        tx.commit(); 

        return true;
    }

    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return false; 
    }
}

bool init_label_discs_table(connection& c) {
    std::string remove_total_discs_table_sql = "DROP TABLE IF EXISTS label_discs_total";

    std::string create_total_discs_table_sql = "CREATE TABLE label_discs_total(" \
                                    "position_id INT PRIMARY KEY REFERENCES positions(id)," \
                                    "discs_total INT NOT NULL);";

    try {
        work tx(c); 
        tx.exec(remove_total_discs_table_sql);
        tx.exec(create_total_discs_table_sql);
        tx.commit(); 

        return true;
    }

    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return false; 
    }
}

bool init_label_stable_table(connection& c) {
    std::string remove_stable_discs_table_sql = "DROP TABLE IF EXISTS label_stable_discs";

    std::string create_stable_discs_table_sql = "CREATE TABLE label_stable_discs(" \
                                    "position_id INT PRIMARY KEY REFERENCES positions(id)," \
                                    "black_stable INT NOT NULL," \
                                    "white_stable INT NOT NULL," \
                                    "total_stable INT NOT NULL ," \
                                    "turn VARCHAR(255) NOT NULL);";

    try {
        work tx(c); 
        tx.exec(remove_stable_discs_table_sql);
        tx.exec(create_stable_discs_table_sql);
        tx.commit(); 

        return true;
    }

    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return false; 
    }
}

void prepare_position_insert(connection& c) {
    c.prepare(
        "position_insert", 
        "INSERT INTO positions (black, white, turn) VALUES ($1, $2, $3) ON CONFLICT (black, white, turn) DO NOTHING;"
    );
}

void prepare_label_discs_total_insert(connection& c) {
    c.prepare(
        "label_td_insert", 
        "INSERT INTO label_discs_total (position_id, discs_total) VALUES ($1, $2);"
    );
}

void prepare_stable_discs_insert(connection& c) {
    c.prepare(
        "label_sd_insert", 
        "INSERT INTO label_stable_discs (position_id, black_stable, white_stable, total_stable, turn) VALUES ($1, $2, $3, $4, $5);"
    );
}
