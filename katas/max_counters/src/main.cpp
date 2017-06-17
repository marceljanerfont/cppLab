#include <iostream>
#include <vector>
#include <climits>
#include <cstdlib>
#include <chrono>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

// For print green messages in google test output
class TestCout : public std::stringstream {
public:
    ~TestCout() {
        std::cout << "\033[1;32m[          ] "<< str() << "\033[0m" << std::endl;
    }
};
#define TEST_COUT  TestCout()
/////////////////////////////////////////////


// 1st approach 
std::vector<int> solution_approach1(int N, std::vector<int> &A) {
    std::vector<int> counters(N, 0);

    return counters;
}

TEST(SolutionAppreach, Case1) {
    int N = 5;
    std::vector<int> A {3, 4, 4, 6, 1, 4, 4};
    std::vector<int> counters = solution_approach1(N, A);
    std::vector<int> counters_expected {3, 2, 2, 4, 2};

    EXPECT_THAT(counters, ::testing::ContainerEq(counters_expected));
}
/////////////////////////////////////////////

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
