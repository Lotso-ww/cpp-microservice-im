#ifndef ALIBABACLOUD_CREDENTIALS_EcsRamRoleProvider_HPP_
#define ALIBABACLOUD_CREDENTIALS_EcsRamRoleProvider_HPP_

#include <atomic>
#include <memory>
#include <string>

#include <alibabacloud/credentials/Constant.hpp>
#include <alibabacloud/credentials/Model.hpp>
#include <alibabacloud/credentials/provider/RefreshableProvider.hpp>

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_PUSH

namespace AlibabaCloud {
namespace Credentials {

/**
 * @brief ECS RAM role credential provider (async refresh version)
 *
 * Features:
 * - Background scheduled refresh (shared global thread, checks every 1 minute)
 * - IMDSv2/IMDSv1 auto-switch
 * - ALLOW mode tolerates refresh failures
 */
class ALIBABACLOUD_CREDENTIALS_EXPORT EcsRamRoleProvider : public RefreshableProvider {
public:
  // Default configuration constants
  static constexpr int DEFAULT_METADATA_TOKEN_DURATION = 21600;  // 6 hours
  static constexpr int DEFAULT_CONNECT_TIMEOUT = 1000;            // 1 second
  static constexpr int DEFAULT_READ_TIMEOUT = 1000;               // 1 second

  /**
   * @brief Construct with config
   */
  EcsRamRoleProvider(
      std::shared_ptr<Models::Config> config,
      bool asyncUpdateEnabled = true,
      StaleValueBehavior behavior = StaleValueBehavior::ALLOW_,
      std::shared_ptr<PrefetchStrategy> strategy = std::make_shared<NonBlockingPrefetch>());

  /**
   * @brief Construct with role name
   */
  EcsRamRoleProvider(
      const std::string& roleName = "",
      bool disableIMDSv1 = false,
      bool asyncUpdateEnabled = true,
      StaleValueBehavior behavior = StaleValueBehavior::ALLOW_,
      std::shared_ptr<PrefetchStrategy> strategy = std::make_shared<NonBlockingPrefetch>());

  virtual ~EcsRamRoleProvider();

  /**
   * @brief Get provider name
   */
  std::string getProviderName() const override { return Constant::ECS_RAM_ROLE; }

  /**
   * @brief Check if async credential update is enabled
   */
  bool isAsyncCredentialUpdateEnabled() const { return asyncUpdateEnabled_; }

  /**
   * @brief Whether IMDSv1 fallback is disabled
   */
  bool getDisableIMDSv1() const { return disableIMDSv1_; }

protected:
  /**
   * @brief Override RefreshableProvider's isAsyncUpdateEnabled
   */
  bool isAsyncUpdateEnabled() const override { return asyncUpdateEnabled_; }

  /**
   * @brief Implement credential refresh logic
   */
  virtual RefreshResult doRefresh() const override;

  /**
   * @brief Calculate stale_time (expiration - 15 minutes)
   */
  int64_t getStaleTime(int64_t expiration) const;

  /**
   * @brief Calculate prefetch_time
   */
  static int64_t getPrefetchTime(int64_t expiration);

private:
  /**
   * @brief Register with global scheduler
   */
  void registerWithScheduler();

  /**
   * @brief Unregister from global scheduler
   */
  void unregisterFromScheduler();

  /**
   * @brief Scheduled refresh callback (called by global scheduler)
   */
  void scheduledRefresh();

  /**
   * @brief Get role name from metadata service
   */
  std::string getRoleName() const;

  /**
   * @brief Get metadata with IMDSv2 token and optional IMDSv1 fallback
   */
  std::string getMetadata(const std::string& url) const;

  /**
   * @brief Perform a single metadata GET (optional IMDSv2 token header)
   */
  std::string doGetMetadata(const std::string& url,
                            const std::string& metadataToken) const;

  /**
   * @brief Whether to retry without token after IMDSv2 GET failure
   */
  bool shouldFallbackToIMDSv1(const std::string& metadataToken) const;

  /**
   * @brief Get IMDSv2 Token (empty if unavailable and fallback allowed)
   */
  std::string getMetadataToken() const;

  // URL constants
  static const std::string URL_IN_ECS_META_DATA;
  static const std::string URL_IN_ECS_METADATA_TOKEN;
  static const std::string META_DATA_SERVICE_HOST;

  // Error messages
  static const std::string ECS_METADATA_FETCH_ERROR_MSG;
  static const std::string ECS_METADATA_TOKEN_FETCH_ERROR_MSG;

  // Member variables
  mutable std::string roleName_;
  mutable bool disableIMDSv1_;
  mutable std::atomic<bool> shouldRefresh_;
  bool asyncUpdateEnabled_;
  int64_t connectTimeout_;
  int64_t readTimeout_;

  // Scheduler entry ID (for unregistration)
  size_t schedulerEntryId_{0};
};

} // namespace Credentials
} // namespace AlibabaCloud

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_POP

#endif
