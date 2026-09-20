#ifndef ALIBABACLOUD_CREDENTIALS_URLPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIALS_URLPROVIDER_HPP_

#include <string>

#include <alibabacloud/credentials/Exception.hpp>
#include <alibabacloud/credentials/Constant.hpp>
#include <alibabacloud/credentials/Model.hpp>
#include <alibabacloud/credentials/provider/RefreshableProvider.hpp>

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_PUSH

namespace AlibabaCloud {
namespace Credentials {

class ALIBABACLOUD_CREDENTIALS_EXPORT URLProvider : public RefreshableProvider {
public:
  URLProvider(std::shared_ptr<Models::Config> config,
              StaleValueBehavior behavior = StaleValueBehavior::STRICT_,
              std::shared_ptr<PrefetchStrategy> strategy = std::make_shared<NonBlockingPrefetch>());

  URLProvider(const std::string &url,
              StaleValueBehavior behavior = StaleValueBehavior::STRICT_,
              std::shared_ptr<PrefetchStrategy> strategy = std::make_shared<NonBlockingPrefetch>());

  virtual ~URLProvider() = default;

  /**
   * @brief Get provider name
   */
  std::string getProviderName() const override { return Constant::URL_STS; }

protected:

  virtual RefreshResult doRefresh() const override;


  int64_t getStaleTime(int64_t expiration) const;

private:
  std::string url_;
  int64_t connectTimeout_ = 10000;  // Connection timeout in milliseconds
  int64_t readTimeout_ = 5000;      // Read timeout in milliseconds
};

} // namespace Credentials

} // namespace AlibabaCloud

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_POP

#endif