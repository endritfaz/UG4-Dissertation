#include <unistd.h>
#include <iostream>
#include <cstring> 

int main() { 
    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        if (strcmp(buffer, "hello")) {
            std::cout << "It's working\n" << std::flush;
        }
    }

    return 0;
}