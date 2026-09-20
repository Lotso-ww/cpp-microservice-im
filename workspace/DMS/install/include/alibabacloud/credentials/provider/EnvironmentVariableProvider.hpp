#ifndef ALIBABACLOUD_CREDENTIALS_ENVIRONMENTVARIABLEPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIALS_ENVIRONMENTVARIABLEPROVIDER_HPP_

#include <memory>

#include <alibabacloud/credentials/Exception.hpp>

#include <alibabacloud/credentials/Constant.hpp>
#include <alibabacloud/credentials/Model.hpp>
#include <alibabacloud/credentials/provider/Provider.hpp>

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_PUSH

namespace AlibabaCloud {

namespace Credentials {

class ALIBABACLOUD_CREDENTIALS_EXPORT EnvironmentVariableProvider : public Provider {
public:
  EnvironmentVariableProvider() = default;
  
  // Disable copy (contains unique_ptr)
  EnvironmentVariableProvider(const EnvironmentVariableProvider&) = delete;
  EnvironmentVariableProvider& operator=(const EnvironmentVariableProvider&) = delete;
  
  // Allow move
  EnvironmentVariableProvider(EnvironmentVariableProvider&&) = default;
  EnvironmentVariableProvider& operator=(EnvironmentVariableProvider&&) = default;
  
  virtual ~EnvironmentVariableProvider() {}

  virtual Models::CredentialModel getCredential() const override {
    provider_ = createProvider();
    if (provider_ == nullptr) {
      throw CredentialException(std::string("Can't create the ProfileProvider."));
    }
    return provider_->getCredential();
  }
  
  /**
   * @brief Get provider name
   */
  std::string getProviderName() const override {
    provider_ = createProvider();
    if (provider_ == nullptr) {
      throw CredentialException(std::string("Can't create the ProfileProvider."));
    }
    return provider_->getProviderName();
  }

protected:
  static std::unique_ptr<Provider> createProvider();

  mutable std::unique_ptr<Provider> provider_ = nullptr;
  mutable Models::CredentialModel credential_;
};

} // namespace Credentials
} // namespace AlibabaCloud

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_POP

#endif
