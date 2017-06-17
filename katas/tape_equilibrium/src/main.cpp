#include <iostream>
#include <climits>
#include <cstdlib>
#include <chrono>
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


// 1st approach O(n) x O(n) = O(n^2)
int difference(int A[], int N, int P) {
    int sum_left = 0;
    for (int i = 0; i < P; i++) {
        sum_left += A[i];
    }
    int sum_right= 0;
    for (int i = P; i < N; i++) {
        sum_right += A[i];
    }
    return abs(sum_left - sum_right);
}

int solution_approach1(int A[], int N) {
    int minimum = INT_MAX;
    for (int P = 1; P < N; P++) {
        minimum = std::min(minimum, difference(A, N, P));
    }
    return minimum;
}

TEST(SolutionAppreach1, Case1) {
    int N = 5;
    int A[5] = {3, 1, 2, 4, 3};
    EXPECT_EQ(solution_approach1(A, N), 1);
}

/////////////////////////////////////////////

// 1st approach O(n) + O(n) = O(n)
int solution_approach2(int A[], int N) {
    long long minimum = LLONG_MAX;
    long long sum_left = A[0];
    long long sum_right = A[1];
    // first pass
    for (int i = 2; i < N; i++) {
        sum_right += A[i];
    }
    minimum = std::min(minimum, llabs(sum_left - sum_right));
    // second pass
    for (int i = 1; i < N; i++) {
        sum_left += A[i];
        sum_right -= A[i];
        minimum = std::min(minimum, llabs(sum_left - sum_right));
    }
    return minimum;
}

TEST(SolutionAppreach2, Case1) {
    int N = 5;
    int A[5] = {3, 1, 2, 4, 3};
    EXPECT_EQ(solution_approach2(A, N), 1);
}
/////////////////////////////////////////////


TEST(ComprareApproachs, CaseHuge) {
    int rand_min  = 0;
    int rand_max = 1000;
    int N = 100000;
    int A[100000];
    for (int i = 0; i < N; i++) {
        A[i] = rand_min + (rand() % static_cast<int>(rand_max - rand_min + 1));
    }
    auto start = std::chrono::steady_clock::now();
    int sol1 = solution_approach1(A, N);
    auto end = std::chrono::steady_clock::now();
    int sol1_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    TEST_COUT << "Approach 1 elapsed time is :  " << sol1_elapsed << " ms ";

    start = std::chrono::steady_clock::now();
    int sol2 = solution_approach2(A, N);
    end = std::chrono::steady_clock::now();
    int sol2_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    TEST_COUT << "Approach 2 elapsed time is :  " << sol2_elapsed << " ms ";

    EXPECT_LT(sol2_elapsed, sol1_elapsed) << "Approach 2 should be musch faster";
    EXPECT_EQ(sol1, sol2);
}


int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
