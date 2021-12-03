#include <boost/asio.hpp>
#include <thread>
#include <functional>

class TaskScheduler {
 public:
  /**
  * Construct with a hint about the required level of concurrency.
  *
  * @param num_threads How many threads it should allow to run simultaneously.
  */
  TaskScheduler(const int &num_threads);
  ~TaskScheduler();

  //void onTimer(const unsigned int &interval_ms, );

  void singleShotTimer(unsigned int milliseconds, std::function<void()> callback);

  size_t terminate();
  void run(); //!< it blocks until is terminated by 'terminate()' invocation
  void asyncRun();
  bool isRunning();

 private:
  void onTimeout(const boost::system::error_code &e);

  boost::asio::io_context io_ctx_;
  std::thread thread_; //!< asyncRun() thread
  std::atomic_bool running {false};
  std::mutex mutex_;
  size_t executed_handles_nb_ {0};

  // future task
  boost::asio::deadline_timer timer_;
  std::function<void()> callback_;


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