#ifndef ALIBABACLOUD_CREDENTIALS_ACCESSKEYPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIALS_ACCESSKEYPROVIDER_HPP_

#include <memory>
#include <string>

#include <alibabacloud/credentials/Constant.hpp>
#include <alibabacloud/credentials/Model.hpp>
#include <alibabacloud/credentials/provider/Provider.hpp>

namespace AlibabaCloud {
namespace Credentials {

class ALIBABACLOUD_CREDENTIALS_EXPORT AccessKeyProvider : public Provider {
public:
  AccessKeyProvider(std::shared_ptr<Models::Config> config) {
    credential_.setAccessKeyId(config->getAccessKeyId())
        .setAccessKeySecret(config->getAccessKeySecret())
        .setType(Constant::ACCESS_KEY);
  }
  AccessKeyProvider(const std::string &accessKeyId,
                    const std::string &accessKeySecret) {
    credential_.setAccessKeyId(accessKeyId)
        .setAccessKeySecret(accessKeySecret)
        .setType(Constant::ACCESS_KEY);
  }
  virtual ~AccessKeyProvider() {}

  virtual Models::CredentialModel getCredential() const override {
    return credential_;
  }
  
  /**
   * @brief Get provider name
   */
  std::string getProviderName() const override { return Constant::ACCESS_KEY; }

protected:
  mutable Models::CredentialModel credential_;
};
} // namespace Credentials

} // namespace AlibabaCloud
#endif