CXXFLAGS = -Iinclude -std=c++20

perft: othello/perft.cpp 
	g++ $(CXXFLAGS) -o perft othello/perft.cpp othello/othello.cpp -lfmt

analysis: analysis/analysis.cpp 
	g++ $(CXXFLAGS) -o anal analysis/analysis.cpp othello/Game.cpp othello/othello.cpp -lfmt

server: Server.cpp 
	g++ $(CXXFLAGS) Server.cpp othello/othello.cpp Engine.cpp othello/Game.cpp helper.cpp -o server \
    -I/opt/homebrew/include \
    -L/opt/homebrew/lib \
    -lfmt

server_debug: Server.cpp 
	g++ -DDEBUG $(CXXFLAGS) Server.cpp othello/othello.cpp Engine.cpp othello/Game.cpp helper.cpp -o server \
    -I/opt/homebrew/include \
    -L/opt/homebrew/lib \
    -lfmt

azclient: clients/AZClient.cpp
	g++ $(CXXFLAGS) -o azclient clients/AZClient.cpp othello/Game.cpp othello/othello.cpp Engine.cpp helper.cpp -lfmt

randomclient: clients/RandomClient.cpp 
	g++ $(CXXFLAGS) -o randomclient clients/RandomClient.cpp othello/Game.cpp othello/othello.cpp -lfmt

edaxclient: clients/EdaxClient.cpp
	g++ $(CXXFLAGS) -o edaxclient clients/EdaxClient.cpp othello/Game.cpp othello/othello.cpp Engine.cpp helper.cpp -lfmt

db: db.cpp 
	g++ $(CXXFLAGS) -std=c++17 db.cpp othello/Game.cpp othello/othello.cpp analysis/analysis.cpp helper.cpp -I/usr/local/include -L/usr/local/lib -lpqxx -lpq -lfmt -o db

positions: probing/init_positions.cpp
	g++ $(CXXFLAGS) -std=c++17 probing/init_positions.cpp probing/db.cpp othello/Game.cpp othello/othello.cpp analysis/analysis.cpp helper.cpp -I/usr/local/include -L/usr/local/lib -lpqxx -lpq -lfmt -o positions

stability: probing/stability_setup.cpp
	g++ $(CXXFLAGS) -std=c++17 probing/stability_setup.cpp probing/db.cpp probing/shuffler.cpp othello/Game.cpp othello/othello.cpp analysis/analysis.cpp helper.cpp -I/usr/local/include -L/usr/local/lib -lpqxx -lpq -lfmt -o stability

label_sd: probing/init_label_stable_discs.cpp
	g++ $(CXXFLAGS) -std=c++17 probing/init_label_stable_discs.cpp probing/db.cpp othello/Game.cpp othello/othello.cpp analysis/analysis.cpp helper.cpp -I/usr/local/include -L/usr/local/lib -lpqxx -lpq -lfmt -o label_sd

label_fd: probing/init_label_frontier_discs.cpp
	g++ $(CXXFLAGS) -std=c++17 probing/init_label_frontier_discs.cpp probing/db.cpp othello/Game.cpp othello/othello.cpp analysis/analysis.cpp helper.cpp -I/usr/local/include -L/usr/local/lib -lpqxx -lpq -lfmt -o label_fd

label_fcc: probing/init_label_fcc.cpp
	g++ $(CXXFLAGS) -std=c++17 probing/init_label_fcc.cpp probing/db.cpp othello/Game.cpp othello/othello.cpp analysis/analysis.cpp helper.cpp -I/usr/local/include -L/usr/local/lib -lpqxx -lpq -lfmt -o label_fcc