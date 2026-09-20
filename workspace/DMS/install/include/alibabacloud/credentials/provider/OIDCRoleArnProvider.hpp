#ifndef ALIBABACLOUD_CREDENTIALS_OIDCROLEARNPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIALS_OIDCROLEARNPROVIDER_HPP_

#include <darabonba/Env.hpp>

#include <alibabacloud/credentials/Constant.hpp>
#include <alibabacloud/credentials/Model.hpp>
#include <alibabacloud/credentials/provider/RefreshableProvider.hpp>

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_PUSH

namespace AlibabaCloud {
namespace Credentials {

class ALIBABACLOUD_CREDENTIALS_EXPORT OIDCRoleArnProvider : public RefreshableProvider {
public:
  OIDCRoleArnProvider(std::shared_ptr<Models::Config> config,
                      StaleValueBehavior behavior = StaleValueBehavior::STRICT_,
                      std::shared_ptr<PrefetchStrategy> strategy = std::make_shared<NonBlockingPrefetch>());

  OIDCRoleArnProvider(const std::string &roleArn,
                      const std::string &oidcProviderArn,
                      const std::string &oidcTokenFilePath,
                      const std::string &roleSessionName = "credentials-cpp-session",
                      std::shared_ptr<std::string> policy = nullptr,
                      int64_t durationSeconds = 3600,
                      const std::string &regionId = "cn-hangzhou",
                      const std::string &stsEndpoint = "sts.aliyuncs.com",
                      StaleValueBehavior behavior = StaleValueBehavior::STRICT_,
                      std::shared_ptr<PrefetchStrategy> strategy = std::make_shared<NonBlockingPrefetch>());

  virtual ~OIDCRoleArnProvider() = default;

  /**
   * @brief Get provider name
   */
  std::string getProviderName() const override { return Constant::OIDC_ROLE_ARN; }

protected:
  /**
   * @brief Implement credential refresh logic
   */
  virtual RefreshResult doRefresh() const override;

  int64_t getStaleTime(int64_t expiration) const;

private:
  std::string roleArn_;
  std::string oidcProviderArn_;
  std::string oidcTokenFilePath_;
  std::string roleSessionName_;
  std::shared_ptr<std::string> policy_ = nullptr;
  int64_t durationSeconds_ = 3600;
  std::string regionId_ = "cn-hangzhou";
  std::string stsEndpoint_ = "sts.aliyuncs.com";
  bool enableVpc_ = false;
  int64_t connectTimeout_ = 10000;  // Connection timeout in milliseconds
  int64_t readTimeout_ = 5000;      // Read timeout in milliseconds
};

} // namespace Credentials
} // namespace AlibabaCloud

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_POP

#endif