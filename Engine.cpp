#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <fcntl.h>
#include "Engine.h"

using namespace std; 

Engine::Engine(char* executable):
    argv {executable}
{}

Engine::Engine(const std::vector<std::string>& argv):
    argv {argv}
{}

void Engine::setVersion(std::string s) {
    version = s; 
}

void Engine::setName(std::string s) {
    name = s;
}

std::string Engine::getName() {
    return name;
}

std::string Engine::getVersion() {
    return version; 
}

void Engine::setColour(std::string c) {
    colour = c; 
}

void Engine::setPipe(int p[]) {
    pipe = p;
}

std::string Engine::getColour(void) {
    return colour; 
}

char* Engine::getExecutable() {
    if (argv.empty()) {
        return nullptr;
    }
    return const_cast<char*>(argv.front().c_str());
}

int* Engine::getPipe() {
    return pipe; 
}

void Engine::sendCommand(std::string command) {
    write(pipe[1], command.c_str(), command.size());
} 

std::string Engine::getResponse(char end) {
    char buffer[512]; 
    std::string line;

    ssize_t n; 
    bool endchar = false; 

    while ((n = read(pipe[0], buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < n; i++) {
            
            if (buffer[i] == end) {
                endchar = true;
                break; 
            }
            
            line += buffer[i];
        }
        
        if (endchar) {
            break; 
        }
        
    }
    return line; 
}

void Engine::emptyResponse() {
    char buffer[8192];
    int flags = fcntl(pipe[0], F_GETFL, 0);
    fcntl(pipe[0], F_SETFL, flags | O_NONBLOCK);
    while (read(pipe[0], buffer, sizeof(buffer)) > 0);
    fcntl(pipe[0], F_SETFL, flags);
}


void Engine::launchEngine(const char* dir) {
	try {
		// TODO: Replace with absolute path, that is passed in as an argument
		if (chdir(dir) != 0) {
			perror("chdir");
			exit(EXIT_FAILURE);
		}
        if (argv.empty()) {
            std::cerr << "Engine argv is empty\n";
            exit(EXIT_FAILURE);
        }
        std::vector<char*> args;
        args.reserve(argv.size() + 1);
        for (const auto& arg : argv) {
            args.push_back(const_cast<char*>(arg.c_str()));
        }
        args.push_back(nullptr);
		execvp(args[0], args.data());
	}
	catch (exception e) {
		perror("execvp");
		exit(EXIT_FAILURE);
	}
}

void Engine::configurePipes(int server_to_engine[], int engine_to_server[]) {
    close(server_to_engine[1]);
    close(engine_to_server[0]);

    dup2(server_to_engine[0], STDIN_FILENO);
    close(server_to_engine[0]);

    dup2(engine_to_server[1], STDOUT_FILENO);
    // dup2(engine_to_server[1], STDERR_FILENO);
    close(engine_to_server[1]); 
}

// Forks the process to start the engine sets the engine pipe
void Engine::startEngine(const char* dir) {
    int server_to_engine[2];
    int engine_to_server[2];
    int* engine_pipe = new int[2];

    if (::pipe(server_to_engine) < 0 || ::pipe(engine_to_server) < 0) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    else if (pid == 0) {
        configurePipes(server_to_engine, engine_to_server);
        launchEngine(dir);
    }

    else {
        enginePID = pid; 
        // engine std_out -> engine write end -> engine read end 
        engine_pipe[0] = engine_to_server[0];

        // server write -> server read -> engine std_in
        engine_pipe[1] = server_to_engine[1];

        close(engine_to_server[1]);
        close(server_to_engine[0]);

        pipe = engine_pipe;
    }
}

pid_t Engine::getEnginePID() {
    return enginePID; 
}

std::string Engine::readMove() {
    std::string buffer;
    buffer.reserve(16384);

    const std::regex move_re(R"(=\s*(Resign|PASS|pass|[A-H][1-9]))");
    char chunk[8192];
    ssize_t n;

    while ((n = read(pipe[0], chunk, sizeof(chunk))) > 0) {
        buffer.append(chunk, static_cast<size_t>(n));
        std::smatch match;
        if (std::regex_search(buffer, match, move_re)) {
            return match[1].str();
        }
    }

    return "";
}
