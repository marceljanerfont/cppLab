#include <iostream>
#include <iomanip>
#include <chrono>

// For print green messages in google test output
class GTestLoggerCout : public std::stringstream {
public:
    ~GTestLoggerCout() {
        static auto start = std::chrono::high_resolution_clock::now();
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "\033[1;32m[          ] " <<
        "[" << std::setfill('0') << std::setw(6) << 
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms] " <<
        str() << "\033[0m" << std::endl;
    }
};
#define LOGGER GTestLoggerCout()
//////////////////////