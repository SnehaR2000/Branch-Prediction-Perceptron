
#include <iostream>
#include <cstdint>

// Include the header file with the functions you want to test
#include "predictor.h"

// Define a function to perform a test case for localPrediction
void testLocalPrediction(uint16_t pcIndex, bool expected) {
    bool result = localPrediction(pcIndex);
    if (result == expected) {
        std::cout << "LocalPrediction test passed: pcIndex = " << pcIndex << ", expected = " << expected << std::endl;
    }
    else {
        std::cout << "LocalPrediction test failed: pcIndex = " << pcIndex << ", expected = " << expected << ", actual = " << result << std::endl;
    }
}

// Define a function to perform a test case for globalPrediction
void testGlobalPrediction(uint32_t pIndex, bool expected) {
    bool result = globalPrediction();
    if (result == expected) {
        std::cout << "GlobalPrediction test passed: pIndex = " << pIndex << ", expected = " << expected << std::endl;
    }
    else {
        std::cout << "GlobalPrediction test failed: pIndex = " << pIndex << ", expected = " << expected << ", actual = " << result << std::endl;
    }
}

int main() {
    // Perform test cases for localPrediction
    testLocalPrediction(123, true);
    testLocalPrediction(456, false);

    // Perform test cases for globalPrediction
    testGlobalPrediction(789, true);
    testGlobalPrediction(1011, false);

    return 0;
}
