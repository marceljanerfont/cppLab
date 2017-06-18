#include <iostream>
#include <climits>
#include <cstdlib>
#include <chrono>
#include <thread>

#include "gtest/gtest.h"

#include "../include/TaskGroup.h"
#include "../include/utils.h"


pplx::task<void> my_task(int millis, const pplx::cancellation_token_source& cts) {
    return pplx::create_task([millis, cts]() {
        int j = 0;
        int sleep_slice = 500;
        int iters = millis/sleep_slice;
        while (++j < iters) {
            LOGGER << " * ";
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_slice));
            // cheack if it is canceled
            if (cts.get_token().is_canceled()) {
                LOGGER << "- task cancelled -";
                pplx::cancel_current_task();
            }
        }
        LOGGER << "- task end -";
    });
}

TEST(TaskGroup, empty_case) {
    TaskGroup group;
    auto start = std::chrono::steady_clock::now();
    group.wait(2000);
    auto end = std::chrono::steady_clock::now();
    int elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    EXPECT_LT(elapsed, 100) << "should be musch faster";

    int suceeded, total;
    group.getStatus(suceeded, total);
    EXPECT_EQ(suceeded, 0);
    EXPECT_EQ(total, 0);
}

TEST(TaskGroup, single_task_with_time) {
    TaskGroup group;
    group.pushTask(my_task(1000, group.getCts()));
    group.wait(2000);
    int suceeded, total;
    group.getStatus(suceeded, total);
    EXPECT_EQ(suceeded, 1);
    EXPECT_EQ(total, 1);
}

TEST(TaskGroup, single_task_without_time) {
    TaskGroup group;
    group.pushTask(my_task(2000, group.getCts()));
    auto start = std::chrono::steady_clock::now();
    group.wait(100);
    auto end = std::chrono::steady_clock::now();
    int elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    EXPECT_LT(elapsed, 200) << "should be faster";
    int suceeded, total;
    group.getStatus(suceeded, total);
    EXPECT_EQ(suceeded, 0);
    EXPECT_EQ(total, 1);
}

TEST(TaskGroup, many_tasks_with_time) {
    TaskGroup group;
    int NUM_TASKS = 5;
    for (int i = 0; i < NUM_TASKS; ++i) {
        group.pushTask(my_task(500, group.getCts()));
    }
    group.wait(1000);
    int suceeded, total;
    group.getStatus(suceeded, total);
    EXPECT_EQ(suceeded, NUM_TASKS);
    EXPECT_EQ(total, NUM_TASKS);
}


TEST(TaskGroup, many_tasks_without_time) {
    TaskGroup group;
    int NUM_TASKS = 5;
    for (int i = 0; i < NUM_TASKS; ++i) {
        group.pushTask(my_task(5000, group.getCts()));
    }
    group.wait(1000);
    int suceeded, total;
    group.getStatus(suceeded, total);
    EXPECT_EQ(suceeded, 0);
    EXPECT_EQ(total, NUM_TASKS);
}


TEST(TaskGroup, many_tasks_some_with_others_without_time) {
    TaskGroup group;
    int NUM_TASKS_OK = 3;
    int NUM_TASKS_KO = 4;
    for (int i = 0; i < NUM_TASKS_OK; ++i) {
        group.pushTask(my_task(500, group.getCts()));
    }
    for (int i = 0; i < NUM_TASKS_KO; ++i) {
        group.pushTask(my_task(5000, group.getCts()));
    }
    group.wait(1000);
    int suceeded, total;
    group.getStatus(suceeded, total);
    EXPECT_EQ(suceeded, NUM_TASKS_OK);
    EXPECT_EQ(total, NUM_TASKS_OK + NUM_TASKS_KO);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
