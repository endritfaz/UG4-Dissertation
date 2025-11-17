#include <unistd.h>
#include <iostream>
#include <string>
using namespace std; 

class Engine {
    private:
        char* executable; 
        int* pipe;

    public:
        Engine(char* executable):
            executable {executable}
        {}

        void setPipe(int p[]) {
            pipe = p;
        }

        char* getExecutable() {
            return executable;
        }

        int* getPipe() {
            return pipe; 
        }

        void sendCommand(char* command) {

        } 

        std::string getResponse() {
            char buffer[256]; 
            std::string line;

            ssize_t n; 
            bool newline = false; 

            while ((n = read(pipe[0], buffer, sizeof(buffer))) > 0) {
                for (ssize_t i = 0; i < n; i++) {
                    if (buffer[i] == '\n') {
                        newline = true;
                        break; 
                    }
                    line += buffer[i];
                }

                if (newline) {
                    break; 
                }
            }
            return line; 
        }
};


class OTPServer {
   
    public:
        Engine engine1; 
        Engine engine2;

        OTPServer(Engine engine1, Engine engine2):
            engine1 {engine1},
            engine2 {engine2}
        {}


        void start() {
            engine1.setPipe(startEngine(engine1.getExecutable()));
            engine2.setPipe(startEngine(engine2.getExecutable()));
        }
        
        // Forks the process to start the engine and returns a pipe to the engine
        int* startEngine(char* engine) {
            int server_to_engine[2];
            int engine_to_server[2];
            int* engine_pipe = new int[2];

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

                return engine_pipe;
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
    char engine1_executable[] = "./client";
    char engine2_executable[] = "./client2"; 

    Engine engine1{engine1_executable};
    Engine engine2{engine2_executable};
    OTPServer serv{engine1, engine2};

    serv.start();
    
    std::cout << serv.engine2.getResponse();
    return 0; 
}