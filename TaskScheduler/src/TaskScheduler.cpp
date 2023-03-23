#include "TaskScheduler.h"

#include <iostream>
#include <cassert>
#include <limits>

#include <boost/bind.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>

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

std::string Task::id() {
  return id_;
}

Task::Summary Task::summary() {
  const std::lock_guard<std::mutex> lock(mutex_);
  return summary_;
}

Task::Summary Task::terminate() {
  {
    const std::lock_guard<std::mutex> lock(mutex_);
    terminated_ = true;
    timer_.cancel();
  }

  timer_.wait();

  const std::lock_guard<std::mutex> lock(mutex_);
  int64_t pending = pendingTasks();
  summary_.cancelled += pending;
  return summary_;
}

// return true is no aborted
bool Task::invokeCallback(const boost::system::error_code &e) {

  if (e == boost::asio::error::operation_aborted) {
    std::cout << "onTimeoutRepeat --> asio::error::operation_aborted" << std::endl;
    return false;
  }

  // Timer was not cancelled, take necessary action: invoke callback
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

  ++summary_.executed;

  return true;
}

/////////////
TimerTask::TimerTask(boost::asio::deadline_timer timer, std::function<void()> callback, const int64_t &microseconds, const int64_t &repetitions):
  Task(std::move(timer), callback),
  repetitions_(repetitions),
  interval_us_(microseconds),
  prev_interval_us_(microseconds),
  interval_start_(std::chrono::steady_clock::now()) {

  const std::lock_guard<std::mutex> lock(mutex_);

  // schedule first one
  schedule();
}

void TimerTask::schedule() {
  if (!terminated_ && pendingTasks() > 0) {
    size_t cancelled_tasks_nb = timer_.expires_from_now(boost::posix_time::microseconds(prev_interval_us_));
    if (cancelled_tasks_nb > 0) {
      std::cerr << "Cancelled 1" << cancelled_tasks_nb << " scheduled tasks" << std::endl;
    }
    timer_.async_wait(boost::bind(&TimerTask::run, this, boost::asio::placeholders::error));
  }
}

void TimerTask::run(const boost::system::error_code &e) {
  const std::lock_guard<std::mutex> lock(mutex_);

  // invoke callback
  if (!invokeCallback(e)) {
    return;
  }

  // check & handle elapsed time
  std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
  std::chrono::steady_clock::duration elapsed = now - interval_start_;
  int64_t elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
  int64_t deviation_us = prev_interval_us_ - elapsed_us;
  int64_t deviation_total_us = interval_us_ - elapsed_us;
  int64_t tolerance_us = 100'000LL; // 100 ms //interval_us_ / 10; // 10%
  if (abs(deviation_total_us) > tolerance_us) {
    std::cerr << "Callback is not called at the right periodicity. Elapsed between calls: " << elapsed_us / 1'000LL << " ms. Desired execution interval: " << interval_us_ / 1'000LL << " ms" << std::endl;
  }
  interval_start_ =  now;
  prev_interval_us_ = interval_us_ + deviation_us;

  // schedule the next one
  schedule();
}

int64_t TimerTask::pendingTasks() {
  if (repetitions_ <= 0) {
    return std::numeric_limits<int64_t>::max();
  }

  return repetitions_ - summary_.executed;
}

/////////////
CalendarTask::CalendarTask(boost::asio::deadline_timer timer, std::function<void()> callback, const std::queue<boost::posix_time::ptime> &repetitions):
  Task(std::move(timer), callback),
  repetitions_(repetitions) {

  const std::lock_guard<std::mutex> lock(mutex_);

  // schedule first one
  schedule();
}

void CalendarTask::schedule() {
  if (!terminated_ && pendingTasks() > 0) {
    boost::posix_time::ptime expiry_time = repetitions_.front();
    repetitions_.pop();
    size_t cancelled_tasks_nb = timer_.expires_at(expiry_time);
    if (cancelled_tasks_nb > 0) {
      std::cerr << "Cancelled 1" << cancelled_tasks_nb << " scheduled tasks" << std::endl;
    }
    timer_.async_wait(boost::bind(&CalendarTask::run, this, boost::asio::placeholders::error));
  }
}

void CalendarTask::run(const boost::system::error_code &e) {
  const std::lock_guard<std::mutex> lock(mutex_);

  // invoke callback
  if (!invokeCallback(e)) {
    return;
  }

  // re-schedule the next one
  schedule();
}

int64_t CalendarTask::pendingTasks() {
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

std::string TaskScheduler::createTimerTask(std::function<void()> callback, const int64_t &milliseconds, const int64_t &repetitions/* = 0*/) {
  TimerTask *task = new TimerTask(boost::asio::deadline_timer(io_ctx_), callback, milliseconds * 1'000LL, repetitions);
  tasks_.insert(std::pair(task->id(), task));
  return task->id();
}

std::string TaskScheduler::createCalendarTask(std::function<void()> callback, const std::queue<boost::posix_time::ptime> &repetitions) {
  CalendarTask *task = new CalendarTask(boost::asio::deadline_timer(io_ctx_), callback, repetitions);
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