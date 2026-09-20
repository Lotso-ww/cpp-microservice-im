#ifndef ALIBABACLOUD_CREDENTIALS_RSAKEYPAIRPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIALS_RSAKEYPAIRPROVIDER_HPP_

#include <string>

#include <alibabacloud/credentials/Constant.hpp>
#include <alibabacloud/credentials/Model.hpp>
#include <alibabacloud/credentials/provider/RefreshableProvider.hpp>

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_PUSH

namespace AlibabaCloud {
namespace Credentials {

/**
 * @brief RSA Key Pair credential provider
 *
 * Uses RefreshableProvider for automatic credential refresh with:
 * - 15 minutes stale time window (refresh 15 minutes before expiration)
 * - Async background refresh support
 * - Failure retry and stale value handling
 */
class ALIBABACLOUD_CREDENTIALS_EXPORT RsaKeyPairProvider : public RefreshableProvider {
public:
  RsaKeyPairProvider(std::shared_ptr<Models::Config> config,
                     StaleValueBehavior behavior = StaleValueBehavior::STRICT_,
                     std::shared_ptr<PrefetchStrategy> strategy = std::make_shared<NonBlockingPrefetch>());

  RsaKeyPairProvider(const std::string &publicKeyId,
                     const std::string &privateKey,
                     int64_t durationSeconds = 3600,
                     const std::string &regionId = "cn-hangzhou",
                     const std::string &stsEndpoint = "sts.aliyuncs.com",
                     StaleValueBehavior behavior = StaleValueBehavior::STRICT_,
                     std::shared_ptr<PrefetchStrategy> strategy = std::make_shared<NonBlockingPrefetch>());

  virtual ~RsaKeyPairProvider() = default;

  /**
   * @brief Get provider name
   */
  std::string getProviderName() const override { return Constant::RSA_KEY_PAIR; }

protected:
  /**
   * @brief Implement credential refresh logic
   */
  virtual RefreshResult doRefresh() const override;

  /**
   * @brief Calculate stale time (expiration - 15 minutes)
   */
  int64_t getStaleTime(int64_t expiration) const;

private:
  std::string publicKeyId_;
  std::string privateKey_;
  int64_t durationSeconds_ = 3600;
  std::string regionId_ = "cn-hangzhou";
  std::string stsEndpoint_ = "sts.aliyuncs.com";
  int64_t connectTimeout_ = 5000;
  int64_t readTimeout_ = 10000;
};

} // namespace Credentials

} // namespace AlibabaCloud

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_POP

#endif