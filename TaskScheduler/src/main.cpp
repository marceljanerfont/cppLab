#include <iostream>
#include <climits>
#include <cstdlib>
#include <chrono>
#include <thread>

#include "gtest/gtest.h"

#include "TaskScheduler.h"
#include "../include/utils.h"

void printMsg(const std::string &msg) {
  std::cout << "msg: " << msg << std::endl;
}

struct MyClass {
  std::string id_;

  MyClass(const std::string &id) : id_(id) {}
  void printMsg(const std::string &msg) {
    std::cout << id_ << " - msg: " << msg << std::endl;
  }
};



//TEST(TaskScheduler, run_and_stop_twice) {
//  TaskScheduler scheduler(1);
//  EXPECT_EQ(scheduler.isRunning(), false);
//
//  // first run
//  scheduler.asyncRun();
//  std::this_thread::sleep_for(std::chrono::seconds(1));
//  EXPECT_EQ(scheduler.isRunning(), true);
//  scheduler.terminate();
//  EXPECT_EQ(scheduler.isRunning(), false);
//
//  // second run
//  scheduler.asyncRun();
//  std::this_thread::sleep_for(std::chrono::seconds(1));
//  EXPECT_EQ(scheduler.isRunning(), true);
//  scheduler.terminate();
//  EXPECT_EQ(scheduler.isRunning(), false);
//}

TEST(TaskScheduler, single_shot) {
  TaskScheduler scheduler(1);
  scheduler.asyncRun();

  MyClass theClass("testing class");

  scheduler.singleShotTimer(500, std::bind(&MyClass::printMsg, &theClass, "hello!"));
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  size_t executed_nb = scheduler.terminate();
  EXPECT_EQ(scheduler.isRunning(), false);
  EXPECT_EQ(executed_nb, 1);
}

TEST(TaskScheduler, single_shot_notime) {
  TaskScheduler scheduler(1);
  scheduler.asyncRun();

  MyClass theClass("testing class");

  scheduler.singleShotTimer(5000, std::bind(&MyClass::printMsg, &theClass, "hello!"));
  std::this_thread::sleep_for(std::chrono::milliseconds(3));
  size_t executed_nb = scheduler.terminate();
  EXPECT_EQ(scheduler.isRunning(), false);
  EXPECT_EQ(executed_nb, 0);
}

//TEST(SingleShotTimer, single_shot_class) {
//  SingleShotTimer singleshot;
//
//  MyClass theClass("testing class");
//
//  singleshot.scheduleTask(500, std::bind(&MyClass::printMsg, &theClass, "hello!"));
//  std::this_thread::sleep_for(std::chrono::milliseconds(600));
//
//}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
