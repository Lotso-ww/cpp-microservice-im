#ifndef ALIBABACLOUD_CREDENTIALS_PROFILEPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIALS_PROFILEPROVIDER_HPP_

#include <memory>

#include <alibabacloud/credentials/Exception.hpp>

#include <alibabacloud/credentials/provider/Provider.hpp>

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_PUSH

namespace AlibabaCloud {
namespace Credentials {
class ALIBABACLOUD_CREDENTIALS_EXPORT ProfileProvider : public Provider {
public:
  ProfileProvider() = default;
  
  // Disable copy (contains unique_ptr)
  ProfileProvider(const ProfileProvider&) = delete;
  ProfileProvider& operator=(const ProfileProvider&) = delete;
  
  // Allow move
  ProfileProvider(ProfileProvider&&) = default;
  ProfileProvider& operator=(ProfileProvider&&) = default;
  
  virtual ~ProfileProvider() = default;

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
};

} // namespace Credentials
} // namespace AlibabaCloud

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_POP

#endif