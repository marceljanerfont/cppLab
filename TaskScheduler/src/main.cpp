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
    boost::posix_time::ptime t = boost::posix_time::microsec_clock::local_time();
    std::cout <<  boost::posix_time::to_iso_extended_string(t) << "Z - " << id_ << " - msg: " << msg << std::endl;
  }
};


TEST(SingleShotTimer, single_shot_class) {
  SingleShotTimer singleshot;

  MyClass theClass("testing class");

  singleshot.scheduleTask(500, std::bind(&MyClass::printMsg, &theClass, "hello!"));
  std::this_thread::sleep_for(std::chrono::milliseconds(600));

}

TEST(TaskScheduler, run_and_stop_twice) {
  TaskScheduler scheduler(1);
  EXPECT_EQ(scheduler.isRunning(), false);

  // first run
  scheduler.asyncRun();
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_EQ(scheduler.isRunning(), true);
  scheduler.terminate();
  EXPECT_EQ(scheduler.isRunning(), false);

  // second run
  scheduler.asyncRun();
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_EQ(scheduler.isRunning(), true);
  scheduler.terminate();
  EXPECT_EQ(scheduler.isRunning(), false);
}

TEST(TaskScheduler, single_shot) {
  TaskScheduler scheduler(1);
  scheduler.asyncRun();

  MyClass theClass("testing class");

  scheduler.createTimerTask(std::bind(&MyClass::printMsg, &theClass, "hello!"), 500);
  std::this_thread::sleep_for(std::chrono::milliseconds(1'000));
  size_t executed_nb = scheduler.terminate();
  EXPECT_EQ(scheduler.isRunning(), false);
  EXPECT_EQ(executed_nb, 1);
}

TEST(TaskScheduler, single_shot_notime) {
  TaskScheduler scheduler(1);
  scheduler.asyncRun();

  MyClass theClass("testing class");

  scheduler.createTimerTask(std::bind(&MyClass::printMsg, &theClass, "hello!"), 5'000);
  std::this_thread::sleep_for(std::chrono::milliseconds(3));
  size_t executed_nb = scheduler.terminate();
  EXPECT_EQ(scheduler.isRunning(), false);
  EXPECT_EQ(executed_nb, 0);
}


TEST(TaskScheduler, timer_task_limited) {
  TaskScheduler scheduler(1);
  scheduler.asyncRun();

  MyClass theClass("1");

  const int64_t PERIOD_MS = 500;
  const int64_t ITERATIONS_NB = 10;

  // with lambda
  std::string task_id = scheduler.createTimerTask([&theClass]() {
    theClass.printMsg("hello!");
  }, PERIOD_MS, ITERATIONS_NB);

  std::this_thread::sleep_for(std::chrono::milliseconds(ITERATIONS_NB * PERIOD_MS + PERIOD_MS / 2));
  Task::Summary summary = scheduler.summaryTask(task_id);
  int64_t executed_nb = scheduler.terminate();
  EXPECT_EQ(summary.executed, executed_nb);

  EXPECT_EQ(scheduler.isRunning(), false);

  const int64_t tolerance = 0;
  std::cout << "Executed " << summary.executed << " of " << ITERATIONS_NB << ", tolerance: " << tolerance << std::endl;

  EXPECT_LE(summary.executed, ITERATIONS_NB + tolerance);
  EXPECT_GE(summary.executed, ITERATIONS_NB - tolerance);
}

TEST(TaskScheduler, timer_task_infinite) {
  TaskScheduler scheduler(1);
  scheduler.asyncRun();

  MyClass theClass("1");

  const int64_t PERIOD_MS = 500;
  const int64_t ITERATIONS_NB = 10;

  // with lambda
  std::string task_id = scheduler.createTimerTask([&theClass]() {
    theClass.printMsg("hello!");
  }, PERIOD_MS);

  std::this_thread::sleep_for(std::chrono::milliseconds(ITERATIONS_NB * PERIOD_MS + PERIOD_MS / 2));
  Task::Summary summary = scheduler.summaryTask(task_id);
  int64_t executed_nb = scheduler.terminate();
  EXPECT_EQ(summary.executed, executed_nb);

  EXPECT_EQ(scheduler.isRunning(), false);

  const int64_t tolerance =  std::max(1LL, ITERATIONS_NB / 10LL); // 10% of deviation
  std::cout << "Executed " << summary.executed << " of " << ITERATIONS_NB << ", tolerance: " << tolerance << std::endl;

  EXPECT_LE(summary.executed, ITERATIONS_NB + tolerance);
  EXPECT_GE(summary.executed, ITERATIONS_NB - tolerance);
}

TEST(TaskScheduler, two_timer_task_infinite) {
  TaskScheduler scheduler(1);
  scheduler.asyncRun();

  MyClass theClass("1");
  MyClass theClass2("2");

  const int64_t PERIOD_MS = 100;
  const int64_t ITERATIONS_NB = 10;
  const int64_t tolerance =  std::max(1LL, ITERATIONS_NB / 10LL); // 10% of deviation

  // with lambda
  std::string task_id = scheduler.createTimerTask([&theClass]() {
    theClass.printMsg("hello!");
  }, PERIOD_MS);

  // with lambda
  std::string task_id2 = scheduler.createTimerTask([&theClass2]() {
    theClass2.printMsg("hello 2!");
  }, PERIOD_MS);

  std::this_thread::sleep_for(std::chrono::milliseconds(ITERATIONS_NB * PERIOD_MS + PERIOD_MS / 2));
  int64_t executed_nb = scheduler.terminate();

  Task::Summary summary = scheduler.summaryTask(task_id);
  Task::Summary summary2 = scheduler.summaryTask(task_id);

  std::cout << "Executed: " << summary.executed << " of " << ITERATIONS_NB << ", cancelled: " << summary.cancelled << ", tolerance: " << tolerance << std::endl;
  std::cout << "Executed2: " << summary2.executed << " of " << ITERATIONS_NB << ", cancelled2: " << summary2.cancelled << ", tolerance: " << tolerance << std::endl;

  EXPECT_EQ(scheduler.isRunning(), false);
  EXPECT_EQ(summary.executed + summary2.executed, executed_nb);
}

TEST(TaskScheduler, cancel_timer_task_limited) {
  TaskScheduler scheduler(1);
  scheduler.asyncRun();

  MyClass theClass("1");

  const int64_t PERIOD_MS = 500;
  const int64_t ITERATIONS_NB = 10;
  const int64_t ITERATIONS_DONE_NB = ITERATIONS_NB / 2;

  // with lambda
  std::string task_id = scheduler.createTimerTask([&theClass]() {
    theClass.printMsg("hello!");
  }, PERIOD_MS, ITERATIONS_NB);

  std::this_thread::sleep_for(std::chrono::milliseconds(ITERATIONS_DONE_NB * PERIOD_MS + 10));
  Task::Summary summary = scheduler.terminateTask(task_id);
  //std::cout << "Executed_pre: " << summary.executed << " of " << ITERATIONS_NB << ", cancelled_pre: " << summary.cancelled << std::endl;

  std::this_thread::sleep_for(std::chrono::milliseconds(PERIOD_MS / 2));
  int64_t executed_nb = scheduler.terminate();
  Task::Summary summary_post = scheduler.summaryTask(task_id);

  std::cout << "Executed_post: " << summary_post.executed << " of " << ITERATIONS_NB << ", cancelled_post: " << summary_post.cancelled << std::endl;

  EXPECT_EQ(summary.executed, summary_post.executed);
  EXPECT_EQ(summary.cancelled, summary_post.cancelled);

  const int64_t tolerance =  std::max(1LL, ITERATIONS_NB / 10LL); // 10% of deviation

  EXPECT_LE(summary_post.executed, ITERATIONS_DONE_NB + tolerance);
  EXPECT_GE(summary_post.executed, ITERATIONS_DONE_NB - tolerance);
}

// calendar tasks
TEST(TaskScheduler, many_calendar_tasks) {
  TaskScheduler scheduler(1);
  scheduler.asyncRun();

  const int64_t PERIOD_MS = 500;
  const int64_t ITERATIONS_NB = 10;

  MyClass theClass("calendar");

  //boost::posix_time::ptime datetime = boost::posix_time::microsec_clock::local_time() + boost::posix_time::seconds(1);
  //std::cout << "scheduling at: " << boost::posix_time::to_iso_string(datetime) << std::endl;
  std::queue<boost::posix_time::ptime> repetitions_;
  for (int i = 0; i < ITERATIONS_NB; ++i) {
    repetitions_.push(boost::posix_time::microsec_clock::universal_time() + boost::posix_time::seconds(1) + boost::posix_time::milliseconds(i * PERIOD_MS));
  }

  std::string task_id = scheduler.createCalendarTask([&theClass]() {
    theClass.printMsg("hello!");
  }, repetitions_);

  std::this_thread::sleep_for(std::chrono::milliseconds(ITERATIONS_NB * PERIOD_MS + 3'000));
  Task::Summary summary = scheduler.summaryTask(task_id);
  std::cout << "Executed: " << summary.executed << " of " << ITERATIONS_NB << ", cancelled: " << summary.cancelled << std::endl;
  size_t executed_nb = scheduler.terminate();
  EXPECT_EQ(scheduler.isRunning(), false);
  EXPECT_EQ(executed_nb, ITERATIONS_NB);
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  //::testing::GTEST_FLAG(filter) = "TaskScheduler.many_calendar_tasks";
  return RUN_ALL_TESTS();
}
