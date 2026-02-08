#include <iostream>
#include <cstdint>
#include "othello.h"

bool testParityEven() {
    uint64_t playerBoard = 0xf9fdff0000ff6737;
    uint64_t opponentBoard = 0; 
    uint64_t move = 0x100000000; 

    return (!oddParity(playerBoard, opponentBoard, move)); 
}

bool testParityOdd() {
    uint64_t playerBoard = 0xf9fdff0000ff6737;
    uint64_t opponentBoard = 0x8381000000; 
    uint64_t move = 0x8000000; 

    return (oddParity(playerBoard, opponentBoard, move)); 
}

bool testParityEvenDiag() {
    uint64_t playerBoard = 0xdfe7fb0000ff6737;
    uint64_t opponentBoard = 0; 
    uint64_t move = 0x10000000; 

    return (!oddParity(playerBoard, opponentBoard, move)); 
}

bool testParityOddDiag() {
    uint64_t playerBoard = 0xffe7fb0000f76737;
    uint64_t opponentBoard = 0; 
    uint64_t move = 0x10000000; 

    return (oddParity(playerBoard, opponentBoard, move)); 
}

int main() {
    std::cout << testParityEven(); 
    std::cout << "\n"; 

    std::cout << testParityOdd(); 
    std::cout << "\n"; 

    std::cout << testParityEvenDiag(); 
    std::cout << "\n"; 

    std::cout << testParityOddDiag(); 
    std::cout << "\n"; 
}