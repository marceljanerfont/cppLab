
#include <thread>
#include <functional>
#include <map>
#include <queue>

#include <boost/asio.hpp>
#include <boost/utility.hpp> // boost::noncopyable

/**
* @brief Task is the assset of task scheduler (TaskScheduler). It is a base class.
* It wraps the task's callback, id, and a summary of the execution report.
*/
class Task: boost::noncopyable {
 public:
  /**
  * @brief Summary execution report of the task
  */
  struct Summary {
    int64_t executed {0}; //!< number of times of execution, succeded or failed
    int64_t succeded {0}; //!< number of times that callback has been executed successfully without exception
    int64_t failed {0}; //!< number of times that callback has been executed with exception thrown
    int64_t cancelled {0}; //!< number of scheduled tasks have been cancelled
  };

  Task(boost::asio::deadline_timer timer, std::function<void()> callback);

  std::string id();
  Summary summary();
  Summary terminate();

 protected:
  bool invokeCallback(const boost::system::error_code &e); // return true is no aborted

  virtual void run(const boost::system::error_code &e) = 0;
  virtual int64_t pendingTasks() = 0; // not thread safe
  virtual void schedule() = 0;

  boost::asio::deadline_timer timer_;
  std::function<void()> callback_;
  std::string id_;
  std::atomic_bool terminated_ {false};
  Summary summary_;
  std::mutex mutex_;
};

/**
* @brief TimerTask triggers the task every 'interval_us_' microseconds 'repetitions_' times.
* If 'repetitions_' is zero, the task will be invoked forever
*/
class TimerTask: public Task {
 public:
  TimerTask(boost::asio::deadline_timer timer, std::function<void()> callback, const int64_t &microseconds, const int64_t &repetitions);

 protected:
  int64_t pendingTasks() override;;
  void run(const boost::system::error_code &e) override;
  void schedule() override;

  int64_t repetitions_ {0}; // if (repetitions_ < 1) then repeats for ever
  int64_t interval_us_ {0};
  int64_t prev_interval_us_ {0};

  std::chrono::steady_clock::time_point interval_start_;
};

/**
* @brief CalendarTask triggers the task every date-time specified in the 'repetitions_' queue.
* date-time should be in absolute coordinate: UTC
*/
class CalendarTask: public Task {
 public:
  CalendarTask(boost::asio::deadline_timer timer, std::function<void()> callback, const std::queue<boost::posix_time::ptime> &repetitions);

 protected:
  int64_t pendingTasks() override;
  void run(const boost::system::error_code &e) override;
  void schedule() override;

  std::queue<boost::posix_time::ptime> repetitions_;
};

/**
* @brief TaskScheduler is the interface to schedule tasks: TimerTask or CalendarTask
*/
// TODO: how to purgue 'tasks_' map
// never purgue
// auto-purgue: clean completed task
// completion-handler: invoke completion handler and there you can remove the task
class TaskScheduler {
 public:
  /**
  * Construct with a hint about the required level of concurrency.
  *
  * @param num_threads How many threads it should allow to run simultaneously.
  */
  TaskScheduler(const int &num_threads);
  ~TaskScheduler();

  std::string createTimerTask(std::function<void()> callback, const int64_t &milliseconds, const int64_t &repetitions = 0);
  std::string createCalendarTask(std::function<void()> callback, const std::queue<boost::posix_time::ptime> &repetitions); //!< repetitions must be in UTC (Absolut Time)
  Task::Summary terminateTask(const std::string &task_id);

  int64_t terminate();
  void run(); //!< it blocks until is terminated by 'terminate()' invocation
  void asyncRun();
  bool isRunning();
  Task::Summary summaryTask(const std::string &task_id);

 private:
  void destroy();


  boost::asio::io_context io_ctx_;
  std::thread thread_; //!< asyncRun() thread
  std::atomic_bool running {false};
  std::mutex mutex_;
  size_t executed_handles_nb_ {0};

  std::map<std::string, Task *> tasks_;
  boost::asio::deadline_timer timer_;
};

//////////////////////////////////////////////////
//////////////////////////////////////////////////

class SingleShotTimer {
  std::thread thread_;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::atomic_bool shutdown_ {false};

 public:
  SingleShotTimer() = default;
  ~SingleShotTimer() {
    shutdown();
  }

  void shutdown() {
    {
      std::unique_lock<std::mutex> locker(mutex_);
      shutdown_ = true;
      cv_.notify_one();
    }
    thread_.join();
  }

  void scheduleTask(unsigned int milliseconds, std::function<void()> callback) {
    // we need a detached thread, IVSInterfaceImpl::streamReopenHandlerAvigilon() will delete *this
    thread_ = std::thread([&](unsigned int milliseconds, std::function<void()> callback) {
      bool timeout = false;
      {
        std::unique_lock<std::mutex> locker(mutex_);
        if (!shutdown_) {
          timeout = (cv_.wait_for(locker, std::chrono::milliseconds(milliseconds)) == std::cv_status::timeout);
        }
      }
      if (timeout && !shutdown_) {
        callback();
      }
    }, milliseconds, callback);
  }
};