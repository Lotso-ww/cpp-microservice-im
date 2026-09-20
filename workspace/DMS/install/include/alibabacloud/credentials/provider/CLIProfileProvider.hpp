#ifndef ALIBABACLOUD_CREDENTIALS_CLIPROFILEPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIALS_CLIPROFILEPROVIDER_HPP_

#include <memory>

#include <alibabacloud/credentials/Exception.hpp>

#include <alibabacloud/credentials/provider/Provider.hpp>

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_PUSH

namespace AlibabaCloud {
namespace Credentials {

/**
 * @brief CLI Profile credential provider
 * 
 * Specialized credential provider for Alibaba Cloud CLI configuration files
 * 
 * Configuration file path priority:
 * 1. ALIBABA_CLOUD_CLI_PROFILE_PATH environment variable
 * 2. ~/.aliyun/config.json (new path, recommended)
 * 3. ~/.alibabaclouds.ini (old path, fallback)
 * 
 * Cross-platform support:
 * - Windows: %USERPROFILE%\.aliyun\config.json
 * - Linux/macOS: ~/.aliyun/config.json
 * 
 * Difference from ProfileProvider:
 * - CLIProfileProvider: Specialized for CLI config files, supports JSON and INI formats
 * - ProfileProvider: General purpose config file provider (maintains backward compatibility)
 */
class ALIBABACLOUD_CREDENTIALS_EXPORT CLIProfileProvider : public Provider {
public:
  /**
   * @brief Default constructor
   * 
   * Uses default profile name "default"
   */
  CLIProfileProvider();
  
  /**
   * @brief Constructor with profile name
   * 
   * @param profileName Profile name (e.g. "default", "production")
   */
  explicit CLIProfileProvider(const std::string& profileName);
  
  /**
   * @brief Destructor
   */
  virtual ~CLIProfileProvider() = default;

  /**
   * @brief Get credential
   */
  virtual Models::CredentialModel getCredential() const override;
  
  /**
   * @brief Get provider name
   */
  std::string getProviderName() const override;

protected:
  /**
   * @brief Create actual credential provider
   * 
   * Create corresponding Provider based on config file content
   * 
   * @return Credential provider pointer
   */
  std::unique_ptr<Provider> createProvider() const;
  
  /**
   * @brief Get CLI config file path
   * 
   * Priority:
   * 1. ALIBABA_CLOUD_CLI_PROFILE_PATH environment variable
   * 2. ~/.aliyun/config.json
   * 3. ~/.alibabaclouds.ini
   * 
   * @return Config file path
   */
  static std::string getCliProfilePath();
  
  /**
   * @brief Parse JSON format config file
   * 
   * @param filePath File path
   * @param profileName Profile name
   * @return Config object
   */
  static std::shared_ptr<Models::Config> parseJsonProfile(
      const std::string& filePath,
      const std::string& profileName);
  
  /**
   * @brief Parse INI format config file
   * 
   * @param filePath File path
   * @param profileName Profile name
   * @return Config object
   */
  static std::shared_ptr<Models::Config> parseIniProfile(
      const std::string& filePath,
      const std::string& profileName);

private:
  std::string profileName_;                        // Profile name
  mutable std::unique_ptr<Provider> provider_;     // Actual credential provider
};

} // namespace Credentials
} // namespace AlibabaCloud

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_POP

#endif
