#ifndef ALIBABACLOUD_CREDENTIALS_DEFAULTPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIALS_DEFAULTPROVIDER_HPP_

#include <memory>
#include <mutex>
#include <string>

#include <alibabacloud/credentials/Exception.hpp>

#include <alibabacloud/credentials/Model.hpp>
#include <alibabacloud/credentials/provider/Provider.hpp>

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_PUSH

namespace AlibabaCloud {
namespace Credentials {
class ALIBABACLOUD_CREDENTIALS_EXPORT DefaultProvider : public Provider {
public:
  DefaultProvider();
  explicit DefaultProvider(std::shared_ptr<Models::Config> config);

  // Disable copy (contains unique_ptr)
  DefaultProvider(const DefaultProvider&) = delete;
  DefaultProvider& operator=(const DefaultProvider&) = delete;
  
  // Move is implicitly deleted due to std::mutex member
  DefaultProvider(DefaultProvider&&) = delete;
  DefaultProvider& operator=(DefaultProvider&&) = delete;

  virtual ~DefaultProvider() = default;

  virtual Models::CredentialModel getCredential() const override {
    std::lock_guard<std::mutex> lock(providerMutex_);
    return getCredentialLocked();
  }
  
  /**
   * @brief Get provider name
   */
  std::string getProviderName() const override {
    std::lock_guard<std::mutex> lock(providerMutex_);
    return getProviderNameLocked();
  }

protected:
  std::vector<std::unique_ptr<Provider>> providers_;
  bool reuseLastProviderEnabled_ = false;

private:
  Models::CredentialModel getCredentialLocked() const {
    // If reuse enabled and we have a cached successful provider
    if (reuseLastProviderEnabled_ && lastSuccessfulProvider_) {
      try {
        return lastSuccessfulProvider_->getCredential();
      } catch (CredentialException&) {
        // Cache failed, continue trying all providers
        lastSuccessfulProvider_ = nullptr;
      }
    }

    for (auto &provider : providers_) {
      if (provider) {
        try {
          auto credential = provider->getCredential();
          if (reuseLastProviderEnabled_) {
            lastSuccessfulProvider_ = provider.get();
          }
          return credential;
        } catch (CredentialException&) {
          continue;
        }
      }
    }
    throw CredentialException(std::string("Can't get the credential."));
  }

  std::string getProviderNameLocked() const {
    // If reuse enabled and we have a cached successful provider
    if (reuseLastProviderEnabled_ && lastSuccessfulProvider_) {
      try {
        return lastSuccessfulProvider_->getProviderName();
      } catch (CredentialException&) {
        // Cache failed, continue trying all providers
        lastSuccessfulProvider_ = nullptr;
      }
    }

    for (auto &provider : providers_) {
      if (provider) {
        try {
          auto name = provider->getProviderName();
          if (reuseLastProviderEnabled_) {
            lastSuccessfulProvider_ = provider.get();
          }
          return name;
        } catch (CredentialException&) {
          continue;
        }
      }
    }
    throw CredentialException(std::string("Can't get the provider name."));
  }

  mutable std::mutex providerMutex_;
  mutable Provider* lastSuccessfulProvider_ = nullptr;
};
} // namespace Credentials

} // namespace AlibabaCloud

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_POP

#endif