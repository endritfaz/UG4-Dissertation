CXXFLAGS = -Iinclude -std=c++20

analysis: analysis/analysis.cpp 
	g++ $(CXXFLAGS) -o anal analysis/analysis.cpp othello/Game.cpp othello/othello.cpp -lfmt

server: Server.cpp 
	g++ $(CXXFLAGS) Server.cpp othello/othello.cpp Engine.cpp othello/Game.cpp -o server \
    -I/opt/homebrew/include \
    -L/opt/homebrew/lib \
    -lfmt

azclient: clients/AZClient.cpp
	g++ $(CXXFLAGS) -o azclient clients/AZClient.cpp othello/Game.cpp othello/othello.cpp Engine.cpp -lfmt

randomclient: clients/RandomClient.cpp 
	g++ $(CXXFLAGS) -o randomclient clients/RandomClient.cpp othello/Game.cpp othello/othello.cpp -lfmt
