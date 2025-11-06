#include <unistd.h>
#include <iostream>
#include <string>
using namespace std; 

class OTPServer {
    private: 
        char* engine1; 
        char* engine2; 

    public:
        OTPServer(char* engine1, char* engine2):
            engine1 {engine1},
            engine2 {engine2}
        {}

        void print() {
            cout << engine1; 
            cout << engine2; 
        }

        void start(); 

        void launchEngine(char* engine) {
            char *args[] = {engine, NULL};
            try {
                execvp(args[0], args); 
            }
            catch (exception e) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        }

        void configurePipes(int server_to_engine_p[], int engine_to_server_p[], int server_to_engine_s[], int engine_to_server_s[]) {
            close(server_to_engine_s[0]);
            close(engine_to_server_s[0]);
            close(server_to_engine_s[1]);
            close(engine_to_server_s[1]);
            
            close(server_to_engine_p[1]);
            close(engine_to_server_p[0]);

            dup2(server_to_engine_p[0], STDIN_FILENO);
            close(server_to_engine_p[0]);

            dup2(engine_to_server_p[1], STDOUT_FILENO);
            close(engine_to_server_p[1]);  
        }
};

void OTPServer::start() {
    int server_to_engine1[2]; 
    int engine1_to_server[2];

    int server_to_engine2[2];
    int engine2_to_server[2];

    if (pipe(server_to_engine1) < 0 || pipe(engine1_to_server) < 0 || pipe(server_to_engine2) < 0 || pipe(engine2_to_server) < 0) {
        perror("pipe");
        exit(EXIT_FAILURE); 
    }

    pid_t pid1 = fork();
    // Stops first child process from forking 
    pid_t pid2 = -1; 
    if (pid1 != 0) {
        pid2 = fork();
        if (pid2 == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    if (pid1 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } 

    else if (pid1 == 0) {
        configurePipes(server_to_engine1, engine1_to_server, server_to_engine2, engine2_to_server);
        launchEngine(engine1);     
    }
    
    else if (pid2 == 0) {
        configurePipes(server_to_engine2, engine2_to_server, server_to_engine1, engine1_to_server);
        launchEngine(engine2); 
    }
    
    else {
        // Close engine 1 unnecessary pipes 
        close(engine1_to_server[1]);
        close(server_to_engine1[0]);

        // Close engine 2 unnecessary pipes
        close(engine2_to_server[1]);
        close(server_to_engine2[0]);

        char buffer[256];
        ssize_t n = read(engine1_to_server[0], buffer, sizeof(buffer) - 1);
        
        if (n > 0) {
            buffer[n] = '\0';
            std::cout << "Parent received: " << buffer;
        }

        char buffer2[256];
        ssize_t n2 = read(engine2_to_server[0], buffer2, sizeof(buffer2) - 1);

        if (n2 > 0) {
            buffer2[n2] = '\0';
            std::cout << "Parent received: " << buffer2;
        }
        
        // Close remaining pipes 
        close(server_to_engine1[1]);
        close(engine1_to_server[0]);

        close(server_to_engine2[1]);
        close(engine2_to_server[0]);
    }
}

int main() {
    char engine1[] = "./client";
    char engine2[] = "./client2"; 

    OTPServer serv{engine1, engine2};
    serv.start(); 
    return 0; 
}
