#include <iostream>
#include <climits>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <array>
#include <vector>
#include "gtest/gtest.h"

#include "pplx/pplxtasks.h"

// For print green messages in google test output
class TestCout : public std::stringstream {
public:
    ~TestCout() {
        std::cout << "\033[1;32m[          ] "<< str() << "\033[0m" << std::endl;
    }
};
#define TEST_COUT  TestCout()
//////////////////////


TEST(pplx_task, string) {
    auto str = std::make_shared<std::string>("before all, ");

    //pplx::task<std::string> my_task = pplx::create_task([str])
    pplx::task<std::string> my_task = pplx::create_task([str] {
        *str += "inside task, ";
        //std::this_thread::sleep_for(std::chrono::seconds(2));
    }).then([str] {
        *str += "inside then1, ";
        //std::this_thread::sleep_for(std::chrono::seconds(1));
        // if there is a then more-> do not return value
    }).then([str] {
        *str += "inside then2.";
        //std::this_thread::sleep_for(std::chrono::seconds(1));
        return *str;
    });

    TEST_COUT << my_task.get();
}

pplx::task<int> init_task(int i) {
    return pplx::create_task([i]() -> int {
        return i;
    });
}

TEST(pplx_task, concatenate_increment_tasks) {
    auto my_task = pplx::create_task([]() -> int {
        return 0;
    });

    // lambda that increments input value
    auto increment = [](int i) {
        TEST_COUT << "i: " << i;
        return i + 1;
    };

    EXPECT_EQ(3, my_task.then(increment).then(increment).then(increment).get());
    EXPECT_EQ(2, init_task(1).then(increment).get());
}

pplx::task<void> create_my_task(int i){
    return pplx::create_task([i] {
        TEST_COUT << "Hello from task" << i;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
}

TEST(pplx_task, store_and_launch_several) {
    // Start multiple tasks.
    int NUM_TASKS = 2;
    TEST_COUT << "begin task array asynchronously";
    std::vector<pplx::task<void>> tasks;
    for(int i= 0; i < NUM_TASKS; ++i) {
        tasks.push_back(create_my_task(i));
    }
    auto joinTask = pplx::when_all(begin(tasks), end(tasks));
    // Print a message from the joining thread.
    TEST_COUT << "Hello from the joining thread.";
    // Wait for the tasks to finish.
    joinTask.wait();
    tasks.clear();
}

std::mutex mutex;
// it does not work by function argument
std::condition_variable cv;
pplx::task<int> create_long_task(int i, const pplx::cancellation_token_source &cts){
    return pplx::create_task([i, &cts]() -> int {
        int j = 0;
        while (++j < 20) {
            TEST_COUT << " - " << i;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            // cheack if it is canceled
            if (cts.get_token().is_canceled()) {
                //cv.notify_one();
                pplx::cancel_current_task();
            }
        }
        TEST_COUT << "***** task ends";
        cv.notify_one();
        return j;
    });
    // .then([] (int j) {
    //     TEST_COUT << "notifying";
    //     cv.notify_one();
    //     return j;
    // });
}

// TEST(pplx_task, cancel_one_long_task) {
//     std::condition_variable cv;
//     std::mutex mutex;
//     pplx::cancellation_token_source cts;
//     auto task = create_long_task(1, cts, cv);
//     //std::this_thread::sleep_for(std::chrono::milliseconds(3000));
//     // cts.cancel();
//     std::unique_lock<std::mutex> lk(mutex);
//     TEST_COUT << "waiting task ends";
//     cv.wait_for(lk, std::chrono::milliseconds(2000), [&task]() {
//         TEST_COUT << "is done? " << task.is_done();
//         return task.is_done();
//     });
//     TEST_COUT << "waiting end";
//     cts.cancel();
//     if (task.wait() == pplx::task_status::canceled) {
//         TEST_COUT << "task was cencelled";
//     } else {
//         TEST_COUT << "task result: " << task.get();
//     }
// }
////////////////////////
// THIS IS WHAT I NEED
////////////////////////
TEST(pplx_task, cancel_many_long_task) {
    // it doesn't exits !!!!
    //pplx::task_group task_group;
    pplx::cancellation_token_source cts;

    int NUM_TASKS = 10;
    TEST_COUT << "begin task array asynchronously";
    std::vector<pplx::task<int>> tasks;
    for(int i= 0; i < NUM_TASKS; ++i) {
        tasks.push_back(create_long_task(i, cts));
    }
    auto joinTask = pplx::when_all(begin(tasks), end(tasks));
    // Print a message from the joining thread.
    TEST_COUT << "Hello from the joining thread.";
    // Wait for the tasks to finish.
    TEST_COUT << "waiting tasks ends";
    std::unique_lock<std::mutex> lk(mutex);
    cv.wait_for(lk, std::chrono::milliseconds(20000), [&joinTask]() {
        TEST_COUT << "is done? " << joinTask.is_done();
        return joinTask.is_done();
    });
    TEST_COUT << "waiting end";
    cts.cancel();
    if (joinTask.wait() == pplx::task_status::canceled) {
        TEST_COUT << "tasks were cencelled";
    } else {
        TEST_COUT << "tasks finished ";
    }
    tasks.clear();
}


/////////////////////////////////////////////


int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
