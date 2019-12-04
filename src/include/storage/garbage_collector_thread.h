#pragma once

#include <transaction/deferred_action_manager.h>

#include <chrono>  //NOLINT
#include <thread>  //NOLINT

#include "storage/garbage_collector.h"

namespace terrier::storage {

/**
 * Class for spinning off a thread that runs garbage collection at a fixed interval. This should be used in most cases
 * to enable GC in the system unless you need fine-grained control over table state or profiling.
 */
class GarbageCollectorThread {
 public:
  /**
   * @param gc pointer to the garbage collector object to be run on this thread
   * @param gc_period sleep time between GC invocations
   */
  GarbageCollectorThread(common::ManagedPointer<GarbageCollector> gc, std::chrono::milliseconds gc_period);

  ~GarbageCollectorThread() {
    run_gc_ = false;
    gc_thread_.join();
    for (uint8_t i = 0; i < transaction::MIN_GC_INVOCATIONS; i++) {
      gc_->PerformGarbageCollection();
    }
  }

  /**
   * Pause the GC from running, typically for use in tests when the state of tables need to be fixed.
   */
  void PauseGC() {
    TERRIER_ASSERT(!gc_paused_, "GC should not already be paused.");
    gc_paused_ = true;
  }

  /**
   * Resume GC after being paused.
   */
  void ResumeGC() {
    TERRIER_ASSERT(gc_paused_, "GC should already be paused.");
    gc_paused_ = false;
  }

  /**
   * @return the underlying GC object, mostly to register indexes currently.
   */
  const common::ManagedPointer<GarbageCollector> GetGarbageCollector() { return gc_; }

 private:
  const common::ManagedPointer<storage::GarbageCollector> gc_;
  volatile bool run_gc_;
  volatile bool gc_paused_;
  std::chrono::milliseconds gc_period_;
  std::thread gc_thread_;

  void GCThreadLoop() {
    while (run_gc_) {
      std::this_thread::sleep_for(gc_period_);
      if (!gc_paused_) gc_->PerformGarbageCollection();
    }
  }
};

}  // namespace terrier::storage
