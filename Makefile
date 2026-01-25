CXXFLAGS = -Iinclude

analysis: analysis/analysis.cpp 
	g++ $(CXXFLAGS) -o anal analysis/analysis.cpp othello/Game.cpp othello/othello.cpp