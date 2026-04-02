#ifndef ENGINE_H
#define ENGINE_H

#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>
class Engine {
private: 
    std::vector<std::string> argv; 
    int* pipe;
    std::string colour;
    pid_t enginePID; 
    std::string name; 
    std::string version; 

public:
    Engine(char* executable);
    Engine(const std::vector<std::string>& argv);

    void setVersion(std::string s);
    std::string getVersion(); 
    void setName(std::string s);
    std::string getName();
    void setColour(std::string c); 
    void setPipe(int p[]);
    std::string getColour(void);
    char* getExecutable(); 
    int* getPipe(); 
    void sendCommand(std::string command);
    std::string getResponse(char end); 
    void emptyResponse();
    void launchEngine(const char* dir);
    void configurePipes(int server_to_engine[], int engine_to_server[]);
    void startEngine(const char* dir);
    pid_t getEnginePID(void);
    std::string readMove();
    std::string readUntilReady();
};

#endif
