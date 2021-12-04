
#include <thread>
#include <functional>
#include <map>

#include <boost/asio.hpp>

#include <boost/utility.hpp> // boost::noncopyable


class Task: boost::noncopyable {
  struct Summary {
    int64_t executed {0};
    int64_t succeded {0};
    int64_t failed {0};
    int64_t cancelled {0};
  };

 public:
  Task(boost::asio::deadline_timer timer, std::function<void()> callback);
  std::string id() {
    return id_;
  }
  Summary report() {
    return summary;
  }

 protected:
  virtual void schedule(const boost::system::error_code &e) = 0;

  boost::asio::deadline_timer timer_;
  std::function<void()> callback_;
  std::string id_;
  int64_t executed_times_{0};
  Summary summary;
};

class TimerTask: public Task {
 public:
  TimerTask(boost::asio::deadline_timer timer, std::function<void()> callback, const int64_t &microseconds, const int64_t &repetitions);

 private:
  void schedule(const boost::system::error_code &e) override;

  int64_t repetitions_ {0}; // if (repetitions_ < 1) then repeats for ever
  int64_t interval_us_ {0};
  int64_t prev_interval_us_ {0};

  std::chrono::steady_clock::time_point interval_start_;
};

class CalendarTask: public Task {
 public:
  CalendarTask(boost::asio::deadline_timer timer, std::function<void()> callback, const int64_t &microseconds, const std::vector<boost::posix_time::ptime> &repetitions);

 private:
  void schedule(const boost::system::error_code &e) override;

  std::vector<boost::posix_time::ptime> repetitions_;
};

class TaskScheduler {
 public:
  /**
  * Construct with a hint about the required level of concurrency.
  *
  * @param num_threads How many threads it should allow to run simultaneously.
  */
  TaskScheduler(const int &num_threads);
  ~TaskScheduler();


  std::string createTimerTask(const int64_t &milliseconds, std::function<void()> callback, const int64_t &repetitions = 0);

  size_t terminate();
  void run(); //!< it blocks until is terminated by 'terminate()' invocation
  void asyncRun();
  bool isRunning();


 private:
  boost::asio::io_context io_ctx_;
  std::thread thread_; //!< asyncRun() thread
  std::atomic_bool running {false};
  std::mutex mutex_;
  size_t executed_handles_nb_ {0};

  std::map<std::string, Task *> tasks_;


  // future task
  boost::asio::deadline_timer timer_;
  std::function<void()> callback_;
  int64_t interval_us_;
  int64_t prev_interval_us_;
  std::chrono::steady_clock::time_point interval_start_;
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