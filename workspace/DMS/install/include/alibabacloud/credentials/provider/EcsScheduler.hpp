#ifndef ALIBABACLOUD_CREDENTIALS_ECSSCHEDULER_HPP_
#define ALIBABACLOUD_CREDENTIALS_ECSSCHEDULER_HPP_

#include <atomic>
#include <condition_variable>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <thread>

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_PUSH

namespace AlibabaCloud {
namespace Credentials {

/**
 * @brief Global shared scheduler for ECS credential refresh
 *
 * All EcsRamRoleProvider instances share this single background thread,
 * reducing thread count from N to 1.
 *
 * Thread-safe singleton pattern with proper cleanup.
 */
class EcsScheduler {
public:
  using RefreshCallback = std::function<void()>;
  using EntryId = size_t;

  /**
   * @brief Get singleton instance
   */
  static EcsScheduler& instance() {
    static EcsScheduler scheduler;
    return scheduler;
  }

  /**
   * @brief Register a refresh callback
   *
   * @param callback Function to call periodically
   * @return Entry ID for later unregistration
   */
  EntryId registerCallback(RefreshCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto id = nextId_++;
    entries_.push_back({id, std::move(callback)});

    // Reset stop flag when new entry is added
    threadStop_ = false;

    // Start thread if not running
    if (!threadRunning_.load()) {
      // Wait for previous thread to finish if any
      if (thread_.joinable()) {
        thread_.join();
      }
      threadRunning_.store(true);
      thread_ = std::thread(&EcsScheduler::run, this);
    }

    return id;
  }

  /**
   * @brief Unregister a callback
   *
   * @param id Entry ID returned from registerCallback
   */
  void unregister(EntryId id) {
    std::lock_guard<std::mutex> lock(mutex_);
    entries_.remove_if([id](const Entry& e) { return e.id == id; });

    // Signal thread to check if it should stop (when no more entries)
    if (entries_.empty()) {
      threadStop_ = true;
      cv_.notify_all();
    }
  }

  /**
   * @brief Check if async update is enabled
   */
  bool isAsyncUpdateEnabled() const {
    return asyncUpdateEnabled_.load();
  }

  /**
   * @brief Set async update enabled flag
   */
  void setAsyncUpdateEnabled(bool enabled) {
    asyncUpdateEnabled_.store(enabled);
  }

private:
  EcsScheduler() : threadRunning_(false), threadStop_(false), nextId_(1),
                   asyncUpdateEnabled_(true) {}

  ~EcsScheduler() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      threadStop_ = true;
      cv_.notify_all();
    }

    if (thread_.joinable()) {
      thread_.join();
    }
  }

  // Delete copy and move
  EcsScheduler(const EcsScheduler&) = delete;
  EcsScheduler& operator=(const EcsScheduler&) = delete;

  void run() {
    while (true) {
      // Wait for interval or stop signal
      {
        std::unique_lock<std::mutex> lock(mutex_);
        if (cv_.wait_for(lock, std::chrono::minutes(1),
                         [this]() { return threadStop_; })) {
          // Stop requested - check if we should really exit
          if (entries_.empty()) {
            threadRunning_.store(false);
            break;
          }
          // New entries added, reset stop flag
          threadStop_ = false;
        }
      }

      // Execute callbacks (outside lock)
      std::vector<RefreshCallback> callbacks;
      {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& entry : entries_) {
          callbacks.push_back(entry.callback);
        }
      }

      for (const auto& callback : callbacks) {
        try {
          callback();
        } catch (...) {
          // Ignore errors
        }
      }
    }
  }

  struct Entry {
    EntryId id;
    RefreshCallback callback;
  };

  std::mutex mutex_;
  std::condition_variable cv_;
  std::thread thread_;
  std::atomic<bool> threadRunning_;       // Is thread currently running?
  bool threadStop_;                       // Should thread stop? (protected by mutex_)
  std::list<Entry> entries_;
  EntryId nextId_;
  std::atomic<bool> asyncUpdateEnabled_;
};

} // namespace Credentials
} // namespace AlibabaCloud

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_POP

#endif