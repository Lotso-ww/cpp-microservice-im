#ifndef ALIBABACLOUD_CREDENTIALS_BEARERTOKENPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIALS_BEARERTOKENPROVIDER_HPP_

#include <memory>
#include <string>

#include <alibabacloud/credentials/Constant.hpp>
#include <alibabacloud/credentials/Model.hpp>
#include <alibabacloud/credentials/provider/Provider.hpp>

namespace AlibabaCloud {
namespace Credentials {
class ALIBABACLOUD_CREDENTIALS_EXPORT BearerTokenProvider : public Provider {
public:
  BearerTokenProvider(std::shared_ptr<Models::Config> config) {
    credential_.setBearerToken(config->getBearerToken()).setType(Constant::BEARER);
  }
  BearerTokenProvider(const std::string &bearToken) {
    credential_.setBearerToken(bearToken).setType(Constant::BEARER);
  }

  virtual ~BearerTokenProvider() {}

  virtual Models::CredentialModel getCredential() const override {
    return credential_;
  }
  
  /**
   * @brief Get provider name
   */
  std::string getProviderName() const override { return Constant::BEARER; }

protected:
  mutable Models::CredentialModel credential_;
};
} // namespace Credentials

} // namespace AlibabaCloud
#endif