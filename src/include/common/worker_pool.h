#pragma once
#include <atomic>
#include <condition_variable>  // NOLINT
#include <functional>
#include <iostream>
#include <mutex>  // NOLINT
#include <queue>
#include <string>
#include <thread>  // NOLINT
#include <utility>
#include <vector>
#include "common/macros.h"

namespace terrier::common {

using TaskQueue = std::queue<std::function<void()>>;

/**
 * A worker pool that maintains a group of worker threads and a task queue.
 *
 * As soon as there is a task in the task queue, a worker thread will be
 * assigned to run that task. A task must be a function that takes no argument.
 * After a worker finishes a task, it will eagerly try to get a new task.
 *
 * This pool is restartable, meaning it can be started again after it has been
 * shutdown. Calls to Startup() and Shutdown() are thread-safe.
 */
class WorkerPool {
 public:
  /**
   * Initialize the worker pool. Once the number of worker is set by the constructor
   * it cannot be changed.
   *
   * After initialization, the worker pool automatically starts up.
   *
   * @param num_workers the number of workers in this pool
   * @param task_queue a queue of tasks
   */
  // NOLINTNEXTLINE  lint thinks it has only one arguement
  WorkerPool(uint32_t num_workers, TaskQueue task_queue)
      : num_workers_(num_workers), is_running_(false), task_queue_(std::move(task_queue)), busy_workers_{0} {
    Startup();
  }

  /**
   * Destructor. Wake up all workers and let them finish before it's destroyed.
   */
  ~WorkerPool() {
    std::unique_lock<std::mutex> lock(task_lock_);  // grab the lock
    is_running_ = false;                            // signal all the threads to shutdown
    task_cv_.notify_all();                          // wake up all the threads
    lock.unlock();                                  // free the lock
    for (auto &thread : workers_) thread.join();
  }

  /**
   * Start the worker pool. If the task queue is empty or we run out of tasks,
   * workers will be put into sleep.
   */
  void Startup() {
    {
      std::lock_guard lock(task_lock_);
      is_running_ = true;
    }

    // Create enough workers if necessary
    while (workers_.size() < num_workers_) {
      AddThread();
    }
    // Cut down the number of workers if we have too many workers
    if (workers_.size() > num_workers_) workers_.resize(num_workers_);
  }

  /**
   * It tells all worker threads to finish their current task and stop working.
   * No more tasks will be consumed. It waits until all worker threads stop working.
   */
  void Shutdown() {
    {
      std::lock_guard<std::mutex> lock(task_lock_);
      is_running_ = false;
    }
    // tell everyone to stop working
    task_cv_.notify_all();
    for (auto &worker : workers_) {
      worker.join();
    }
    workers_.clear();
  }

  /**
   * Add a task to the task queue and inform worker threads.
   * You can only submit tasks after the thread pool has started up.
   *
   * @param func the new task
   */
  template <typename F>
  void SubmitTask(const F &func) {
    TERRIER_ASSERT(is_running_, "Only allow to submit task after the thread pool has been started up");
    {
      std::lock_guard<std::mutex> lock(task_lock_);
      task_queue_.emplace(std::move(func));
    }
    task_cv_.notify_one();
  }

  /**
   * Block until all the tasks in the task queue has been completed
   */
  void WaitUntilAllFinished() {
    std::unique_lock<std::mutex> lock(task_lock_);
    // wait for all the tasks to complete

    // Note: Notifications can happen before wait starts.
    // If finished_cv_ missed all notifications, then when it grabs the lock, the condition must be true so it won't
    // deadlock.
    // As long as finished_cv_ received at least one notification, it must receive that last worker's notification so
    // the predicate will evaluate to true
    finished_cv_.wait(lock, [&] { return busy_workers_ == 0 && task_queue_.empty(); });
  }

  /**
   * Get the number of worker threads in this pool
   *
   * @return The number of worker threads
   */
  uint32_t NumWorkers() const { return num_workers_; }

  /**
   * Change the number of worker threads. It can only be done when the thread pool
   * if not running.
   *
   * @param num the number of worker threads.
   */
  void SetNumWorkers(uint32_t num) {
    TERRIER_ASSERT(!is_running_, "Only allow to set num of workers when the thread pool is not running");
    num_workers_ = num;
  }

 private:
  // The worker threads
  std::vector<std::thread> workers_;
  // The number of worker threads
  uint32_t num_workers_;
  // Flag indicating whether the pool is running
  bool is_running_;
  // The queue where workers pick up tasks
  TaskQueue task_queue_;

  uint32_t busy_workers_;

  std::mutex task_lock_;

  std::condition_variable task_cv_;

  std::condition_variable finished_cv_;

  void AddThread() {
    workers_.emplace_back([this] {
      std::function<void(void)> task;

      // keep the thread alive
      while (true) {
        {
          // grab the lock
          std::unique_lock<std::mutex> lock(task_lock_);
          // task_cv_ is notified by new tasks and shutdown command
          // Notifications can get lost
          //  1) missed shutdown messages: the thread will wake up after it grabs the lock because it's not running
          //  2) missed new task: the thread will wake up after it grabs the lock because the queue is not empty
          // need to consider the case the notification sent before waits.
          task_cv_.wait(lock, [&] { return !is_running_ || !task_queue_.empty(); });
          if (!is_running_) {
            // we are shutting down.
            return;
          } else {
            // has a new task
            task = std::move(task_queue_.front());
            task_queue_.pop();
            ++busy_workers_;
          }
        }
        // We don't hold locks at this point
        task();
        {
          // hold the lock for updating the counter
          std::lock_guard<std::mutex> lock(task_lock_);
          --busy_workers_;
        }
        finished_cv_.notify_one();
      }
    });
  }
};
}  // namespace terrier::common
