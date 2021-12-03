#include "TaskScheduler.h"

#include <iostream>
#include <cassert>

#include <boost/bind.hpp>


TaskScheduler::TaskScheduler(const int &num_threads): io_ctx_(num_threads), running(false), timer_(io_ctx_) {
}

TaskScheduler::~TaskScheduler() {
  terminate();
}

size_t TaskScheduler::terminate() {
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


void TaskScheduler::onTimeout(const boost::system::error_code &e) {
  if (e == boost::asio::error::operation_aborted) {
    std::cout << "onTimeout --> asio::error::operation_aborted" << std::endl;
  } else {
    // Timer was not cancelled, take necessary action.
    callback_();
  }
}

void TaskScheduler::singleShotTimer(unsigned int milliseconds, std::function<void()> callback) {
  size_t cancelled_tasks_nb = timer_.expires_from_now(boost::posix_time::milliseconds(milliseconds));
  assert(cancelled_tasks_nb == 0); // we are not expecting to cancel any one

  callback_ = callback;
  timer_.async_wait(boost::bind(&TaskScheduler::onTimeout, this, boost::asio::placeholders::error));
  //boost::asio::post(callback);
}