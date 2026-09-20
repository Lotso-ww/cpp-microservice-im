#ifndef ALIBABACLOUD_CREDENTIALS_PROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIALS_PROVIDER_HPP_

#include <memory>
#include <string>

#include <alibabacloud/credentials/Model.hpp>
#include <alibabacloud/credentials/Export.hpp>

namespace AlibabaCloud {
namespace Credentials {
class ALIBABACLOUD_CREDENTIALS_EXPORT Provider {
public:
  Provider() = default;
  Provider(const Provider &) = default;
  Provider(Provider &&) = default;
  Provider &operator=(const Provider &) = default;
  Provider &operator=(Provider &&) = default;
  virtual ~Provider() = default;

  /**
   * @brief Get credential model
   * @note Returns a copy to ensure thread safety and data consistency.
   *       The copy overhead is minimal (6 shared_ptr copies).
   * @return CredentialModel containing all credential information
   */
  virtual Models::CredentialModel getCredential() const = 0;
  
  /**
   * @brief Get provider name
   * @return Provider name string
   */
  virtual std::string getProviderName() const = 0;
};
} // namespace Credentials
} // namespace AlibabaCloud
#endif