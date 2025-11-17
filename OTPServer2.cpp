#include <unistd.h>
#include <iostream>
#include <string>
using namespace std; 

class OTPServer {
    private: 
        char* engine1; 
        char* engine2; 

        int engine1_pipe[2];
        int engine2_pipe[2];

    public:
        OTPServer(char* engine1, char* engine2):
            engine1 {engine1},
            engine2 {engine2}
        {}


        void start() {
            connectEngine(engine1, engine1_pipe); 
            connectEngine(engine2, engine2_pipe);
        }
        
        void connectEngine(char* engine, int engine_pipe[]) {
            int server_to_engine[2];
            int engine_to_server[2];

            if (pipe(server_to_engine) < 0 || pipe(engine_to_server) < 0) {
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
                launchEngine(engine);
            }

            else {
                // engine std_out -> engine write end -> engine read end 
                engine_pipe[0] = engine_to_server[0];

                // server write -> server read -> engine std_in
                engine_pipe[1] = server_to_engine[1];

                close(engine_to_server[1]);
                close(server_to_engine[0]);
            }
        }
        
        void configurePipes(int server_to_engine[], int engine_to_server[]) {
            close(server_to_engine[1]);
            close(engine_to_server[0]);

            dup2(server_to_engine[0], STDIN_FILENO);
            close(server_to_engine[0]);

            dup2(engine_to_server[1], STDOUT_FILENO);
            close(engine_to_server[1]); 
        }

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
};

int main() {
    char engine1[] = "./client";
    char engine2[] = "./client2"; 

    OTPServer serv{engine1, engine2};
    serv.start(); 
    return 0; 
}