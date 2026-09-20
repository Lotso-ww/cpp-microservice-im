#ifndef ALIBABACLOUD_CREDENTIALS_AUTHUTIL_HPP_
#define ALIBABACLOUD_CREDENTIALS_AUTHUTIL_HPP_

#include <string>
#include <alibabacloud/credentials/Export.hpp>

namespace Darabonba {
namespace Http {
class Request;
}
}

namespace AlibabaCloud {

namespace Credentials {

class ALIBABACLOUD_CREDENTIALS_EXPORT AuthUtil {
public:
  /**
   * @brief Get client type from environment variable
   * 
   * Reads ALIBABA_CLOUD_PROFILE environment variable each time it's called.
   * Returns "default" if the environment variable is not set or empty.
   * 
   * @return Client type string
   */
  static std::string clientType();
  
  // Generate default session name: "credentials-cpp-{timestamp}"
  static std::string generateSessionName();
  
  /**
   * @brief Get User-Agent string
   * 
   * Format: "AlibabaCloud ({os}; {machine}) C++/{cpp_version} Credentials/{credentials_version} TeaDSL/2 {custom_ua}"
   * Corresponds to Python SDK's get_user_agent() function
   * 
   * Example: "AlibabaCloud (macOS; arm64) C++/11 Credentials/0.1.0 TeaDSL/2"
   * 
   * @param customUserAgent Custom user agent suffix (optional)
   * @return Complete User-Agent string
   */
  static std::string getUserAgent(const std::string& customUserAgent = "");
  
  /**
   * @brief Create a new HTTP request with User-Agent header
   * 
   * Corresponds to Python SDK's getNewRequest() helper function
   * Used by providers like EcsRamRole, RsaKeyPair to ensure requests have proper User-Agent
   * 
   * @param url Request URL
   * @param customUserAgent Custom user agent suffix (optional)
   * @return HTTP Request object with User-Agent header set
   */
  static Darabonba::Http::Request getNewRequest(const std::string& url, 
                                                  const std::string& customUserAgent = "");
  
  /**
   * @brief Get SDK version string
   * @return Version string in format "x.y.z" (e.g., "0.1.0")
   */
  static std::string getSDKVersion();
  
  /**
   * @brief Get operating system name
   * @return OS name (e.g., "macOS", "Linux", "Windows")
   */
  static std::string getOSName();
  
  /**
   * @brief Get machine architecture
   * @return Machine architecture (e.g., "x86_64", "arm64")
   */
  static std::string getMachineName();
  
  /**
   * @brief Get C++ standard version
   * @return C++ version (e.g., "11", "14", "17", "20")
   */
  static std::string getCppVersion();
};

} // namespace Credentials
} // namespace AlibabaCloud

#endif