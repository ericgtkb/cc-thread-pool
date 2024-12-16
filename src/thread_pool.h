#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <future>
#include <exception>


template<typename ReturnType>
class ThreadPool {
public:
  explicit ThreadPool(int num_threads) {
    for (int i = 0; i < num_threads; ++i) {
      // Start all the threads with the worker_loop
      workers_.emplace_back([this] {this->worker_loop();});
    }
  }

  void shutdown() {
    {
      std::scoped_lock lock(mutex_);
      shutting_down_ = true;
    }
    cond_var_.notify_all();
  }

  void wait() {
    // Prevents new jobs being submitted
    shutdown();
    for (auto& worker : workers_) {
      worker.join();
    }
  }

  template<typename FuncType, typename ...ArgTypes>
  std::future<ReturnType> submit(FuncType&& func, ArgTypes&& ...args) {
    {
      // Cannot submit if we are shutting down
      std::scoped_lock lock(mutex_);
      if (shutting_down_) {
        throw std::runtime_error("Cannot submit jobs after shutdown.");
      }
    }

    // Forward the job into a packaged task
    auto task = std::packaged_task<ReturnType()>(
      [
        func = std::forward<FuncType>(func),
        args = std::make_tuple(std::forward<ArgTypes>(args)...)
      ]() {
        return std::apply(func, std::move(args));
      }
    );

    auto future = task.get_future();

    {
      std::scoped_lock lock(mutex_);
      jobs_.emplace(std::move(task));
    }

    cond_var_.notify_one();
    return future;
  }

private:
  std::vector<std::thread> workers_;
  std::queue<std::packaged_task<ReturnType()>> jobs_;
  std::mutex mutex_;
  std::condition_variable cond_var_;
  bool shutting_down_ = false;

  // The loop job for each thread to run util the new job is invalid
  void worker_loop() {
    while (true) {
      // Get a job from the queue
      std::unique_lock lock(mutex_);
      cond_var_.wait(lock, [this] {return !jobs_.empty() || shutting_down_;});

      if (shutting_down_ && jobs_.empty()) {
        // lock will be released as it goes out of scope after break.
        break;
      }

      auto task {std::move(jobs_.front())};
      jobs_.pop();
      lock.unlock();

      // Actual job execution
      task();
    }
  }
};
