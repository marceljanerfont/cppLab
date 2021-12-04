#include <iostream>
#include <climits>
#include <cstdlib>
#include <chrono>
#include <thread>

#include <boost/date_time/posix_time/posix_time.hpp>

#include "gtest/gtest.h"

#include "TaskScheduler.h"
#include "../include/utils.h"

void printMsg(const std::string &msg) {
  std::cout << "msg: " << msg << std::endl;
}

template <class Precision>
std::string getISOCurrentTimestamp() {
  auto now = std::chrono::system_clock::now();
  return std::date::format("%FT%TZ", std::date::floor<Precision>(now));
}

struct MyClass {
  std::string id_;

  MyClass(const std::string &id) : id_(id) {}
  void printMsg(const std::string &msg) {
    boost::posix_time::ptime t = boost::posix_time::microsec_clock::universal_time();
    std::cout <<  boost::posix_time::to_iso_extended_string(t) << "Z - " << id_ << " - msg: " << msg << std::endl;
  }
};


//TEST(SingleShotTimer, single_shot_class) {
//  SingleShotTimer singleshot;
//
//  MyClass theClass("testing class");
//
//  singleshot.scheduleTask(500, std::bind(&MyClass::printMsg, &theClass, "hello!"));
//  std::this_thread::sleep_for(std::chrono::milliseconds(600));
//
//}

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

//TEST(TaskScheduler, single_shot) {
//  TaskScheduler scheduler(1);
//  scheduler.asyncRun();
//
//  MyClass theClass("testing class");
//
//  scheduler.singleShotTimer(500, std::bind(&MyClass::printMsg, &theClass, "hello!"));
//  std::this_thread::sleep_for(std::chrono::milliseconds(1'000));
//  size_t executed_nb = scheduler.terminate();
//  EXPECT_EQ(scheduler.isRunning(), false);
//  EXPECT_EQ(executed_nb, 1);
//}
//
//TEST(TaskScheduler, single_shot_notime) {
//  TaskScheduler scheduler(1);
//  scheduler.asyncRun();
//
//  MyClass theClass("testing class");
//
//  scheduler.singleShotTimer(5000, std::bind(&MyClass::printMsg, &theClass, "hello!"));
//  std::this_thread::sleep_for(std::chrono::milliseconds(3));
//  size_t executed_nb = scheduler.terminate();
//  EXPECT_EQ(scheduler.isRunning(), false);
//  EXPECT_EQ(executed_nb, 0);
//}

TEST(TaskScheduler, periodic_shot) {
  TaskScheduler scheduler(1);
  scheduler.asyncRun();

  MyClass theClass("1");

  const int64_t PERIOD_MS = 1'000;
  const int64_t ITERATIONS_NB = 10;

  // binding a function
  //scheduler.periodicShotTimer(PERIOD_MS, std::bind(&MyClass::printMsg, &theClass, "hello!"));

  // with lambda
  std::string task_id = scheduler.createTimerTask(PERIOD_MS, [&theClass]() {
    theClass.printMsg("hello!");
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(ITERATIONS_NB * PERIOD_MS));
  size_t executed_nb = scheduler.terminate();
  EXPECT_EQ(scheduler.isRunning(), false);

  const int64_t tolerance =  std::max(1LL, ITERATIONS_NB / 10LL); // 10% of deviation
  std::cout << "Executed " << executed_nb << " of " << ITERATIONS_NB << ", tolerance: " << tolerance << std::endl;

  EXPECT_LE(executed_nb, ITERATIONS_NB + tolerance);
  EXPECT_GE(executed_nb, ITERATIONS_NB - tolerance);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
