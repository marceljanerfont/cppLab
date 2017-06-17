#include <iostream>
#include "gtest/gtest.h"

// For print green messages in google test output
class TestCout : public std::stringstream
{
public:
    ~TestCout()
    {
        std::cout << "\033[1;32m[          ] "<< str() << "\033[0m" << std::endl;
    }
};
#define TEST_COUT  TestCout()
//////////////////////

int sum(int a, int b) {
    return a + b;
}

TEST(MathTest, TwoPlusTwoEqualsFour) {
    TEST_COUT << "hello world!";
    EXPECT_EQ(sum(2, 2), 4);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
