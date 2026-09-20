#ifndef ALIBABACLOUD_CREDENTIALS_STSPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIALS_STSPROVIDER_HPP_

#include <memory>
#include <string>

#include <alibabacloud/credentials/Constant.hpp>
#include <alibabacloud/credentials/Model.hpp>
#include <alibabacloud/credentials/provider/Provider.hpp>

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_PUSH

namespace AlibabaCloud {
namespace Credentials {
class ALIBABACLOUD_CREDENTIALS_EXPORT StsProvider : public Provider {
public:
  StsProvider(std::shared_ptr<Models::Config> config) {
    credential_.setAccessKeyId(config->getAccessKeyId())
        .setAccessKeySecret(config->getAccessKeySecret())
        .setSecurityToken(config->getSecurityToken())
        .setType(Constant::STS);
  }
  StsProvider(const std::string &accessKeyId,
              const std::string &accessKeySecret,
              const std::string &securityToken) {
    credential_.setAccessKeyId(accessKeyId)
        .setAccessKeySecret(accessKeySecret)
        .setSecurityToken(securityToken)
        .setType(Constant::STS);
  }

  virtual ~StsProvider() {}

  virtual Models::CredentialModel getCredential() const override {
    return credential_;
  }
  
  /**
   * @brief Get provider name
   */
  std::string getProviderName() const override { return Constant::STS; }

protected:
  mutable Models::CredentialModel credential_;
};
} // namespace Credentials

} // namespace AlibabaCloud

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_POP

#endif