#pragma once

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <thread>
#include <atomic>

#include "storage/checkpoints/checkpoint.h"

namespace terrier::storage {
class CheckpointBackgroundLoop {
 public:
  explicit CheckpointBackgroundLoop(const std::string &path, catalog::db_oid_t db, const char *cur_log_file,
                                    uint32_t num_threads, common::WorkerPool *thread_pool, Checkpoint *checkpoint)
      : path_(path),
        db_(db),
        cur_log_file_(cur_log_file),
        num_threads_(num_threads),
        thread_pool_(thread_pool),
        checkpoint_(checkpoint) {}

  void BackgroundLoop(const int64_t interval, const int64_t num_checkpoints) {
    std::unique_lock<std::mutex> lck(mtx);

    for (uint64_t i = 0; i < num_checkpoints; i++) {
      if (stop) break;
      STORAGE_LOG_INFO("Taking Checkpoint: ", i);
      checkpoint_->TakeCheckpoint(path_, db_, cur_log_file_, num_threads_,
                                  thread_pool_);
      STORAGE_LOG_INFO("Finish Checkpoint: ", i);
      cv.wait_for(lck, std::chrono::seconds(interval));
    }
  }

  // Epoch is number of seconds to wait
  void StartBackgroundLoop(const int64_t interval, const int64_t num_checkpoints){
    thread_pool_->SubmitTask([this, interval, num_checkpoints]{BackgroundLoop(interval, num_checkpoints);});
  }

  void EndBackgroundLoop() {
    stop = true;
    cv.notify_all();
  }

 private:
  const std::string path_;
  const catalog::db_oid_t db_;
  const char *cur_log_file_;
  const uint32_t num_threads_;
  common::WorkerPool *thread_pool_;
  Checkpoint *checkpoint_;
  std::atomic<bool> stop = false;
  std::mutex mtx;
  std::condition_variable cv;
};

}  // namespace terrier::storage
