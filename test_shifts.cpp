#include <iostream>
#include <cstdint>
#include "othello.h"

bool testNormalShifts() {
    uint64_t normalRightShift = 0x200000000000; 
    uint64_t normalRightShiftExpected = 0x400000000000; 

    uint64_t normalLeftShift = 0x8000000;
    uint64_t normalLeftShiftExpected = 0x4000000;

    uint64_t normalUpShift = 0x4000000000; 
    uint64_t normalUpShiftExpected = 0x400000000000; 

    uint64_t normalDownShift = 0x10000; 
    uint32_t normalDownShiftExpected = 0x100; 

    uint64_t normalRightUpShift = 0x2000000000;
    uint64_t normalRightUpShiftExpected = 0x400000000000;

    uint64_t normalRightDownShift = 0x200000; 
    uint64_t normalRightDownShiftExpected = 0x4000; 

    uint64_t normalLeftUpShift = 0x4000000; 
    uint64_t normalLeftUpShiftExpected = 0x200000000; 

    uint64_t normalLeftDownShift = 0x4000000; 
    uint64_t normalLeftDownShiftExpected = 0x20000;

    int shamts[8] = {1, -1, 8, -8, 9, -7, 7, -9};

    uint64_t tests[8][2] = {{normalRightShift, normalRightShiftExpected}, {normalLeftShift, normalLeftShiftExpected}, {normalUpShift, normalUpShiftExpected}, {normalDownShift, normalDownShiftExpected}, {normalRightUpShift, normalRightUpShiftExpected}, {normalRightDownShift, normalRightDownShiftExpected}, {normalLeftUpShift, normalLeftUpShiftExpected}, {normalLeftDownShift, normalLeftDownShiftExpected}};

    int testsLen = sizeof(tests) / sizeof(tests[0]);

    bool res = true;
    for (int i = 0; i < testsLen; i++) {
        res &= (shift(tests[i][0], shamts[i]) == tests[i][1]);
    }
    return res; 
}

bool testWrapShifts() {
    uint64_t wrapRightShift = 0x8000000000;
    uint64_t wrapRightShiftExpected = 0x0;

    uint64_t wrapRightUpShift = 0x8000000000;
    uint64_t wrapRightUpShiftExpected = 0x0;

    uint64_t wrapRightDownShift = 0x8000000000;
    uint32_t wrapRightDownShiftExpected = 0x0;

    uint64_t wrapLeftShift = 0x100000000; 
    uint32_t wrapLeftShiftExpected = 0x0; 

    uint64_t wrapLeftUpShift = 0x100000000; 
    uint32_t wrapLeftUpShiftExpected = 0x0; 

    uint64_t wrapLeftDownShift = 0x100000000; 
    uint64_t wrapLeftDownShiftExpected = 0x0;

    int shamts[6] = {1, 9, -7, -1, 7, -9};

    uint64_t tests[6][2] = {{wrapRightShift, wrapRightShiftExpected}, {wrapRightUpShift, wrapRightUpShiftExpected}, {wrapRightDownShift, wrapRightDownShiftExpected}, {wrapLeftShift, wrapLeftShiftExpected}, {wrapLeftUpShift, wrapLeftUpShiftExpected}, {wrapLeftDownShift, wrapLeftDownShiftExpected}};

    int testsLen = sizeof(tests) / sizeof(tests[0]);
    
    bool res = true;
    for (int i = 0; i < testsLen; i++) {
        res &= (shift(tests[i][0], shamts[i]) == tests[i][1]);
    }
    return res;  
}

int main() {
    std::cout << testWrapShifts(); 
    return -1;
}