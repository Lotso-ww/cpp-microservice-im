#ifndef ALIBABACLOUD_CREDENTIALS_CLIENT_HPP_
#define ALIBABACLOUD_CREDENTIALS_CLIENT_HPP_

#include <alibabacloud/credentials/Model.hpp>
#include <alibabacloud/credentials/Export.hpp>
#include <alibabacloud/credentials/provider/Provider.hpp>

#include <memory>
#include <string>

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_PUSH

namespace AlibabaCloud {
namespace Credentials {
class ALIBABACLOUD_CREDENTIALS_EXPORT Client : public Darabonba::Model {
  friend void to_json(Darabonba::Json &j, const Client &obj) {
    DARABONBA_PTR_TO_JSON(config, config_);
  }
  friend void from_json(const Darabonba::Json &j, Client &obj) {
    DARABONBA_PTR_FROM_JSON(config, config_);
    obj.provider_ = makeProvider(obj.config_);
  }

public:
  // Default constructor
  Client();

  // Config-based constructors (3 overloads for different config types)
  Client(const Models::Config &obj);
  Client(Models::Config &&obj);
  Client(std::shared_ptr<Models::Config> config);

  // Provider-based constructor
  Client(std::shared_ptr<Provider> provider);
  
  Client(const Client &) = default;
  Client(Client &&) = default;
  Client &operator=(const Client &) = default;
  Client &operator=(Client &&) = default;
  virtual ~Client() = default;

  virtual void validate() const override {}

  virtual void fromMap(const Darabonba::Json &obj) override {
    from_json(obj, *this);
    validate();
  }

  virtual Darabonba::Json toMap() const override {
    Darabonba::Json obj;
    to_json(obj, *this);
    return obj;
  }

  virtual bool empty() const override {
    // a special implementation
    return provider_ == nullptr;
  }

  /**
   * @deprecated Use getCredential() to avoid AK/SK misalignment due to refresh
   * @brief Get access key ID (deprecated method)
   */
  std::string getAccessKeyId() {
    return provider_->getCredential().getAccessKeyId();
  };

  /**
   * @deprecated Use getCredential() to avoid AK/SK misalignment due to refresh
   * @brief Get access key secret (deprecated method)
   */
  std::string getAccessKeySecret() {
    return provider_->getCredential().getAccessKeySecret();
  }

  /**
   * @deprecated Use getCredential() to avoid AK/SK misalignment due to refresh
   * @brief Get security token (deprecated method)
   */
  std::string getSecurityToken() {
    return provider_->getCredential().getSecurityToken();
  }

  /**
   * @deprecated Use getCredential() to avoid AK/SK misalignment due to refresh
   * @brief Get bearer token (deprecated method)
   */
  std::string getBearerToken() {
    return provider_->getCredential().getBearerToken();
  }

  /**
   * @deprecated Use getCredential() to avoid AK/SK misalignment due to refresh
   * @brief Get credential type (deprecated method)
   */
  std::string getType() { return provider_->getProviderName(); }

  /**
   * @brief Get credential model (recommended method)
   * @note Return a copy to avoid inconsistencies
   * @return CredentialModel containing all credential information
   */
  Models::CredentialModel getCredential() const { 
    return provider_->getCredential(); 
  }


  /**
   * @brief Get provider name
   * @return Provider name string
   */
  std::string getProviderName() const {
    return provider_->getProviderName();
  }

private:
  static std::shared_ptr<Provider> makeProvider(std::shared_ptr<Models::Config> config);

  std::shared_ptr<Models::Config> config_ = nullptr;

  // using shared_ptr to enable copy
  std::shared_ptr<Provider> provider_ = nullptr;
};

} // namespace Credentials
} // namespace Alibabacloud



namespace nlohmann {
  template <>
  struct adl_serializer<std::shared_ptr<AlibabaCloud::Credentials::Client>> {
    static void to_json(json &j, const std::shared_ptr<AlibabaCloud::Credentials::Client> &client) {
      if (client) {
        j = client->toMap();
      } else {
        j = nullptr;
      }
    }

    static std::shared_ptr<AlibabaCloud::Credentials::Client> from_json(const json &j) {
      if (!j.is_null() && j.is_object()) {
        auto client = std::make_shared<AlibabaCloud::Credentials::Client>();
        client->fromMap(j);
        return client;
      }
      return nullptr;
    }
  };
}

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_POP

#endif