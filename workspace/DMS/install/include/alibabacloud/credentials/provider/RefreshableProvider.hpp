#ifndef ALIBABACLOUD_CREDENTIALS_REFRESHABLEPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIALS_REFRESHABLEPROVIDER_HPP_

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <ctime>
#include <future>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <sstream>
#include <thread>

#ifndef _WIN32
#include <time.h>
#include <cstring>
#endif

#include <alibabacloud/credentials/provider/Provider.hpp>
#include <darabonba/Logger.hpp>

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_PUSH

namespace AlibabaCloud {
namespace Credentials {

/**
 * @brief Refresh result wrapper class
 * 
 * Contains credential value, expiration time and prefetch time
 */
struct RefreshResult {
  Models::CredentialModel credential;
  int64_t staleTime = 0;     // Expiration time (seconds timestamp)
  int64_t prefetchTime = 0;  // Prefetch time (seconds timestamp)
  
  RefreshResult() = default;
  RefreshResult(const Models::CredentialModel& cred, int64_t stale, int64_t prefetch)
      : credential(cred), staleTime(stale), prefetchTime(prefetch) {}
};

/**
 * @brief Stale value behavior policy
 */

// Avoid Windows macro name conflicts (e.g., STRICT, ALLOW)
#ifdef STRICT_
#undef STRICT_
#endif
#ifdef ALLOW_
#undef ALLOW_
#endif
enum class StaleValueBehavior {
  STRICT_,  // Strict mode: never return stale cached values
  ALLOW_    // Allow mode: allow returning stale values to avoid service overload
};

/**
 * @brief Prefetch strategy base class
 */
class PrefetchStrategy {
public:
  virtual ~PrefetchStrategy() = default;
  virtual void prefetch(std::function<void()> action) = 0;
  virtual void close() {}  // Clean up resources
};

/**
 * @brief Non-blocking prefetch strategy (async refresh in background thread)
 * 
 * Uses a managed thread that can be properly shutdown.
 */
class NonBlockingPrefetch : public PrefetchStrategy {
public:
  NonBlockingPrefetch() : shutdown_(false) {}
  
  ~NonBlockingPrefetch() override {
    close();
  }
  
  void prefetch(std::function<void()> action) override {
    if (shutdown_.load()) {
      return;
    }
    
    // Wait for any previous async task to complete
    if (asyncThread_.joinable()) {
      asyncThread_.join();
    }
    
    // Start new async task
    asyncThread_ = std::thread([this, action]() {
      if (shutdown_.load()) {
        return;
      }
      try {
        action();
      } catch (const std::exception& e) {
        // Log error but don't throw exception
        std::cerr << "NonBlockingPrefetch error: " << e.what() << std::endl;
      }
    });
  }
  
  void close() override {
    shutdown_.store(true);
    if (asyncThread_.joinable()) {
      asyncThread_.join();
    }
  }

private:
  std::atomic<bool> shutdown_;
  std::thread asyncThread_;
};

/**
 * @brief One caller blocks prefetch strategy (synchronous refresh)
 */
class OneCallerBlocksPrefetch : public PrefetchStrategy {
public:
  void prefetch(std::function<void()> action) override {
    action();  // Synchronous execution
  }
  
  void close() override {
    // Nothing to clean up for synchronous strategy
  }
};

/**
 * @brief Refreshable credential provider base class
 *
 * Provides async background refresh mechanism with:
 * - 180 seconds prefetch refresh
 * - 15 minutes expiration time window
 * - Failure retry and stale value handling
 * - Configurable refresh strategy (blocking/non-blocking)
 */
class ALIBABACLOUD_CREDENTIALS_EXPORT RefreshableProvider : public Provider {
public:
  // Constants
  static constexpr int64_t STALE_TIME_WINDOW = 15 * 60;      // 15 minutes stale window
  static constexpr int64_t PREFETCH_THRESHOLD = 180;          // 180 seconds prefetch threshold
  static constexpr int64_t REFRESH_BLOCKING_MAX_WAIT_MS = 10000;  // Max wait 10 seconds (in milliseconds)

  /**
   * @brief 构造函数
   * 
   * @param staleValueBehavior 过期值处理策略
   * @param prefetchStrategy 预取策略（默认非阻塞）
   */
  explicit RefreshableProvider(
      StaleValueBehavior staleValueBehavior = StaleValueBehavior::STRICT_,
      std::shared_ptr<PrefetchStrategy> prefetchStrategy = 
          std::make_shared<NonBlockingPrefetch>())
      : staleValueBehavior_(staleValueBehavior),
        prefetchStrategy_(prefetchStrategy),
        consecutiveRefreshFailures_(0),
        cachedValue_(nullptr) {}

  virtual ~RefreshableProvider() {
    close();
  }

  /**
   * @brief Get credential (thread safe)
   * @note Returns a copy for safety. Uses copy-on-write pattern:
   *       - Read: lock-free, just copy the cached shared_ptr
   *       - Write: atomic replacement of the entire shared_ptr
   */
  virtual Models::CredentialModel getCredential() const override {
    // Fast path: check if we need to refresh (lock-free)
    // Copy the shared_ptr first (atomic operation)
    auto cached = cachedValue_;
    
    bool needsRefresh = !cached || cacheIsStale(*cached);
    bool needsPrefetch = !cached || shouldInitiateCachePrefetch(*cached);
    
    if (needsRefresh || needsPrefetch) {
      // Slow path: need to refresh, acquire lock
      std::lock_guard<std::mutex> lock(accessMutex_);
      
      // Re-check after acquiring lock (double-checked locking)
      cached = cachedValue_;
      needsRefresh = !cached || cacheIsStale(*cached);
      needsPrefetch = !cached || shouldInitiateCachePrefetch(*cached);
      
      if (needsRefresh) {
        refreshCache();
      } else if (needsPrefetch) {
        if (isAsyncUpdateEnabled()) {
          prefetchCache();
        } else {
          refreshCache();
        }
      }
      cached = cachedValue_;
    }
    
    if (!cached) {
      throw std::runtime_error("No cached credential available");
    }
    
    return cached->credential;
  }

protected:
  /**
   * @brief Subclass implemented credential refresh logic
   * 
   * @return RefreshResult containing new credential and expiration time
   */
  virtual RefreshResult doRefresh() const = 0;

  /**
   * @brief Check if async update is enabled (to be overridden by subclasses)
   * 
   * Default implementation returns true for backward compatibility.
   * Subclasses like EcsRamRoleProvider should override this method.
   */
  virtual bool isAsyncUpdateEnabled() const { return true; }

  /**
   * @brief Time utility: convert GMT time string to timestamp
   * 
   * Accepts format: "%Y-%m-%dT%H:%M:%SZ"
   */
  static int64_t strtotime(const std::string& gmt) {
    std::tm tm{};
#ifdef _WIN32
    std::istringstream ss(gmt);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    if (ss.fail()) {
      throw std::runtime_error("Failed to parse GMT datetime: " + gmt);
    }
    // Convert to time_t in UTC
    time_t t = _mkgmtime(&tm);
    if (t == -1) {
      throw std::runtime_error("Failed to convert tm to time_t (UTC) for: " + gmt);
    }
    return static_cast<int64_t>(t);
#else
    // Use strptime + timegm on POSIX
    if (strptime(gmt.c_str(), "%Y-%m-%dT%H:%M:%SZ", &tm) == nullptr) {
      throw std::runtime_error("Failed to parse GMT datetime: " + gmt);
    }
    time_t t = timegm(&tm);
    if (t == -1) {
      throw std::runtime_error("Failed to convert tm to time_t (UTC) for: " + gmt);
    }
    return static_cast<int64_t>(t);
#endif
  }

  /**
   * @brief Time utility: get current GMT time string in ISO8601 format
   * 
   * Returns: "%Y-%m-%dT%H:%M:%SZ"
   */
  static std::string gmt_datetime() {
    time_t now;
    time(&now);
#ifdef _WIN32
    std::tm tm{};
    if (gmtime_s(&tm, &now) != 0) {
      throw std::runtime_error("gmtime_s failed");
    }
    char buf[21]; // "YYYY-MM-DDTHH:MM:SSZ" + '\0'
    if (std::strftime(buf, sizeof(buf), "%FT%TZ", &tm) == 0) {
      throw std::runtime_error("strftime failed");
    }
    return std::string(buf);
#else
    char buf[21];
    std::tm* gmt = gmtime(&now);
    if (!gmt) {
      throw std::runtime_error("gmtime failed");
    }
    if (std::strftime(buf, sizeof(buf), "%FT%TZ", gmt) == 0) {
      throw std::runtime_error("strftime failed");
    }
    return std::string(buf);
#endif
  }

  /**
   * @brief Time utility: get current timestamp (seconds)
   */
  static int64_t getCurrentTime() {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
  }

private:
  /**
   * @brief Check if cache is stale (with cached value)
   */
  bool cacheIsStale(const RefreshResult& cached) const {
    return getCurrentTime() >= cached.staleTime;
  }

  /**
   * @brief Check if cache is stale (from member)
   */
  bool cacheIsStale() const {
    if (!cachedValue_) {
      return true;
    }
    return cacheIsStale(*cachedValue_);
  }

  /**
   * @brief Check if prefetch should be initiated (with cached value)
   */
  bool shouldInitiateCachePrefetch(const RefreshResult& cached) const {
    return getCurrentTime() >= cached.prefetchTime;
  }

  /**
   * @brief Check if prefetch should be initiated (from member)
   */
  bool shouldInitiateCachePrefetch() const {
    if (!cachedValue_) {
      return true;
    }
    return shouldInitiateCachePrefetch(*cachedValue_);
  }

  /**
   * @brief Async prefetch refresh
   */
  void prefetchCache() const {
    prefetchStrategy_->prefetch([this]() {
      refreshCache();
    });
  }

  /**
   * @brief Synchronous refresh cache (with lock protection)
   */
  void refreshCache() const {
    std::unique_lock<std::timed_mutex> lock(refreshMutex_, std::defer_lock);

    // Try to acquire lock, wait max REFRESH_BLOCKING_MAX_WAIT_MS milliseconds
    if (!lock.try_lock_for(std::chrono::milliseconds(REFRESH_BLOCKING_MAX_WAIT_MS))) {
      // Lock timeout, return using existing cache
      return;
    }

    // Double check: another thread may have already refreshed
    if (!cacheIsStale() && !shouldInitiateCachePrefetch()) {
      return;
    }

    try {
      RefreshResult result = doRefresh();
      cachedValue_ = std::make_shared<RefreshResult>(
          handleFetchedSuccess(result));
    } catch (...) {
      // Use std::rethrow_exception to preserve original exception type
      std::exception_ptr eptr = std::current_exception();
      cachedValue_ = std::make_shared<RefreshResult>(
          handleFetchedFailure(eptr));
    }
  }

  /**
   * @brief Handle successful refresh
   */
  RefreshResult handleFetchedSuccess(const RefreshResult& value) const {
    consecutiveRefreshFailures_ = 0;
    int64_t now = getCurrentTime();

    // Case 1: expiration time is more than 15 minutes away, normal case
    if (now < value.staleTime) {
      return value;
    }

    // Case 2: expiration within 15 minutes but not expired, will refresh next time
    if (now < value.staleTime + STALE_TIME_WINDOW) {
      return RefreshResult(value.credential, now, value.prefetchTime);
    }

    // Case 3: credential expired, try using cache
    if (!cachedValue_) {
      throw std::runtime_error("Retrieved expired credential and no cached value available");
    }

    if (now < cachedValue_->staleTime) {
      // Cache not expired, use cache
      return *cachedValue_;
    }

    // Decide how to handle expired cache based on policy
    if (staleValueBehavior_ == StaleValueBehavior::STRICT_) {
      // Strict mode: return cache but set very short expiration (1 second)
      return RefreshResult(cachedValue_->credential, now + 1, cachedValue_->prefetchTime);
    } else {
      // Allow mode: extend expiration time with random jitter
      int64_t jitter = randomInt(50, 70);  // 50-70 seconds
      return RefreshResult(cachedValue_->credential, now + jitter, cachedValue_->prefetchTime);
    }
  }

  /**
   * @brief Handle refresh failure
   * @param eptr Exception pointer to preserve original exception type
   */
  RefreshResult handleFetchedFailure(std::exception_ptr eptr) const {
    // Extract error message for logging
    std::string errorMsg;
    try {
      std::rethrow_exception(eptr);
    } catch (const std::exception& e) {
      errorMsg = e.what();
    } catch (...) {
      errorMsg = "Unknown exception";
    }

    if (!cachedValue_) {
      Darabonba::Logger::warning(
          "Refresh credentials failed, cached value is None, error: " + errorMsg);
      std::rethrow_exception(eptr);  // No cache, rethrow original exception
    }

    int64_t now = getCurrentTime();
    if (now < cachedValue_->staleTime) {
      Darabonba::Logger::warning(
          "Refresh credentials failed, using cached value. error: " + errorMsg);
      return *cachedValue_;  // Cache not expired, return cache
    }

    consecutiveRefreshFailures_++;

    if (staleValueBehavior_ == StaleValueBehavior::STRICT_) {
      Darabonba::Logger::warning(
          "Refresh credentials failed, cached value is expired. error: " + errorMsg);
      std::rethrow_exception(eptr);  // Strict mode: rethrow original exception
    } else {
      // Allow mode: extend expiration time with exponential backoff
      Darabonba::Logger::warning(
          "Refresh credentials failed, using expired cached value with backoff. error: " + errorMsg);

      // maxJitter: max(10000ms, 2^(n-1)*100ms)
      int64_t maxJitterMillis = std::max(10000LL, (1LL << (consecutiveRefreshFailures_ - 1)) * 100);
      // jitter: [1000ms, maxJitterMillis-1ms]
      int64_t jitterMillis = randomInt(1000, maxJitterMillis - 1);
      int64_t newStaleTime = now + jitterMillis / 1000;  // convert to seconds

      return RefreshResult(cachedValue_->credential, newStaleTime, cachedValue_->prefetchTime);
    }
  }

  /**
   * @brief Close and clean up resources
   *
   * This method should be called before the provider is destroyed
   * to ensure any background threads are properly stopped.
   */
  void close() {
    if (prefetchStrategy_) {
      prefetchStrategy_->close();
    }
    // Clear cached value
    cachedValue_ = nullptr;
  }

private:
  // Thread-safe random number generator
  static int64_t randomInt(int64_t min, int64_t max) {
    static thread_local std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int64_t> dist(min, max);
    return dist(gen);
  }

  // Member variables
  StaleValueBehavior staleValueBehavior_;
  std::shared_ptr<PrefetchStrategy> prefetchStrategy_;

  mutable std::atomic<int> consecutiveRefreshFailures_;
  mutable std::shared_ptr<RefreshResult> cachedValue_;

  mutable std::timed_mutex refreshMutex_;  // Refresh lock
  mutable std::mutex accessMutex_;   // Access lock
};

} // namespace Credentials
} // namespace AlibabaCloud

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_POP

#endif
