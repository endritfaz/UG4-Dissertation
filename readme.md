
# Key project files

## The Othello Implementation

othello/othello.cpp includes the Othello move generator, and detection functions for stability, frontier, forced corner capture, and parity. 

othello/Game.cpp is used to play and keep track of an Othello game, and is used by the Othello arena. 

## The Othello Arena

Server.cpp is the entry point of the Othello arena. Engine.cpp is used to set up the interprocess communication to/from the bot programs. clients/AZClient.cpp and clients/EdaxClient.cpp are the programs launched by the server to play games between AlphaZero and Edax. 

Game data between AlphaZero and Edax was collected with scripts/play_games.sh and stored in the directories game_data_forced_opening and game_data_random_input. 

## Data analysis 

The analysis directory contains all the code responsible for data processing/analysis. analysis/db.cpp moves the saved games to a PostgresSQL database, and the analysis/plots_*.cpp read from the PostgresSQL database to plot the relevant information. 

## Probing

The probing directory contains all of the files that set up the PostgresSQL database used in the probing process, including the activations, label, and results tables. probing/probing.py is where the the models are trained. 