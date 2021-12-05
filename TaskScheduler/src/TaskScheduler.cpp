#include "TaskScheduler.h"

#include <iostream>
#include <cassert>
#include <limits>

#include <boost/bind.hpp>

// uuid
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include <boost/lexical_cast.hpp>


Task::Task(boost::asio::deadline_timer timer, std::function<void()> callback):
  timer_(std::move(timer)),
  callback_(callback),
  terminated_(false) {
  // create task uuid
  static boost::uuids::random_generator uuid_generator; // reuse generator ffor all tasks
  boost::uuids::uuid uuid = uuid_generator();
  id_ = boost::lexical_cast<std::string>(uuid);
}

TimerTask::TimerTask(boost::asio::deadline_timer timer, std::function<void()> callback, const int64_t &microseconds, const int64_t &repetitions):
  Task(std::move(timer), callback),
  repetitions_(repetitions),
  interval_us_(microseconds),
  prev_interval_us_(microseconds),
  interval_start_(std::chrono::steady_clock::now()) {

  size_t cancelled_tasks_nb = timer_.expires_from_now(boost::posix_time::microseconds(prev_interval_us_));
  if (cancelled_tasks_nb > 0) {
    std::cerr << "Cancelled " << cancelled_tasks_nb << " scheduled tasks" << std::endl;
  }
  timer_.async_wait(boost::bind(&TimerTask::schedule, this, boost::asio::placeholders::error));
}

void TimerTask::schedule(const boost::system::error_code &e) {
  if (e == boost::asio::error::operation_aborted) {
    std::cout << "onTimeoutRepeat --> asio::error::operation_aborted" << std::endl;
  } else {
    // Timer was not cancelled, take necessary action.
    int64_t elapsed_us = 0;
    try {
      callback_();
      ++summary_.succeded;
    } catch (const std::exception &e) {
      ++summary_.failed;
      std::cerr << "onTimeoutRepeat exception: " << e.what() << std::endl;
    } catch (...) {
      ++summary_.failed;
      std::cerr << "onTimeoutRepeat unknown exception" << std::endl;
    }

    const std::lock_guard<std::mutex> lock(mutex_);

    ++summary_.executed;
    // check elapsed time
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

    std::chrono::steady_clock::duration elapsed = now - interval_start_;
    elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    int64_t deviation_us = prev_interval_us_ - elapsed_us;
    int64_t deviation_total_us = interval_us_ - elapsed_us;
    int64_t tolerance_us = 100'000LL; // 100 ms //interval_us_ / 10; // 10%
    if (abs(deviation_total_us) > tolerance_us) {
      std::cerr << "Callback is not called at the right periodicity. Elapsed between calls: " << elapsed_us / 1'000LL << " ms. Desired execution interval: " << interval_us_ / 1'000LL << " ms" << std::endl;
    }

    if (!terminated_ &&
        (repetitions_ <= 0 || summary_.executed < repetitions_)) {
      // schedule
      interval_start_ =  now;
      prev_interval_us_ = interval_us_ + deviation_us;
      size_t cancelled_tasks_nb = timer_.expires_from_now(boost::posix_time::microseconds(prev_interval_us_));
      summary_.cancelled += cancelled_tasks_nb;
      if (cancelled_tasks_nb > 0) {
        std::cerr << "Cancelled " << cancelled_tasks_nb << " scheduled tasks" << std::endl;
      }
      timer_.async_wait(boost::bind(&TimerTask::schedule, this, boost::asio::placeholders::error));
    }
  }
}

int64_t TimerTask::pendingTasks() {
  const std::lock_guard<std::mutex> lock(mutex_);

  if (repetitions_ <= 0) {
    return std::numeric_limits<int64_t>::max();
  }

  return repetitions_ - summary_.executed;
}

/////////////
CalendarTask::CalendarTask(boost::asio::deadline_timer timer, std::function<void()> callback, const int64_t &microseconds, const std::vector<boost::posix_time::ptime> &repetitions):
  Task(std::move(timer), callback),
  repetitions_(repetitions) {
}

void CalendarTask::schedule(const boost::system::error_code &e) {
  //TODO:
}

int64_t CalendarTask::pendingTasks() {
  const std::lock_guard<std::mutex> lock(mutex_);

  return repetitions_.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

TaskScheduler::TaskScheduler(const int &num_threads): io_ctx_(num_threads), running(false), timer_(io_ctx_) {
}

TaskScheduler::~TaskScheduler() {
  terminate();
  destroy();
}

void TaskScheduler::destroy() {
  for (auto &it : tasks_) {
    delete it.second;
  }
  tasks_.clear();
}

int64_t TaskScheduler::terminate() {
  // TODO: stop al tasks
  io_ctx_.stop();
  if (thread_.joinable()) {
    thread_.join();
  }
  assert(!running);

  return executed_handles_nb_;
}

bool TaskScheduler::isRunning() {
  return running;
}

void TaskScheduler::run() {
  running = true;
  executed_handles_nb_ = 0;
  io_ctx_.restart(); // must called before run() sets stopped flag to false
  boost::asio::io_service::work work(io_ctx_); // forces io_ctx to keep running until explicitly stopped
  executed_handles_nb_ =  io_ctx_.run();
  running = false;
}

void TaskScheduler::asyncRun() {
  assert(!thread_.joinable() && !isRunning());
  thread_ = std::thread(&TaskScheduler::run, this);
}

std::string TaskScheduler::createTimerTask(const int64_t &milliseconds, std::function<void()> callback, const int64_t &repetitions/* = 0*/) {
  TimerTask *task = new TimerTask(boost::asio::deadline_timer(io_ctx_), callback, milliseconds * 1'000LL, repetitions);
  tasks_.insert(std::pair(task->id(), task));
  return task->id();
}

Task::Summary TaskScheduler::summaryTask(const std::string &task_id) {
  auto it = tasks_.find(task_id);
  if (it == tasks_.end()) {
    //not found
    throw std::runtime_error("Task not found");
  }

  return it->second->summary();
}

Task::Summary TaskScheduler::terminateTask(const std::string &task_id) {
  auto it = tasks_.find(task_id);
  if (it == tasks_.end()) {
    //not found
    throw std::runtime_error("Task not found");
  }

  return it->second->terminate();
}