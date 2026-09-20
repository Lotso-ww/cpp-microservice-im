#ifndef ALIBABACLOUD_CREDENTIALS_MODEL_HPP_
#define ALIBABACLOUD_CREDENTIALS_MODEL_HPP_

#include <darabonba/Model.hpp>
#include <memory>
#include <alibabacloud/credentials/Export.hpp>

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_PUSH

using namespace std;
// Forward declaration to avoid circular dependency
namespace AlibabaCloud {
namespace Credentials {
class AuthUtil;
}
} // namespace AlibabaCloud

namespace AlibabaCloud {
namespace Credentials {
namespace Models {
class ALIBABACLOUD_CREDENTIALS_EXPORT CredentialModel : public Darabonba::Model {
public:
  friend void to_json(Darabonba::Json &j, const CredentialModel &obj) {
    DARABONBA_PTR_TO_JSON(accessKeyId, accessKeyId_);
    DARABONBA_PTR_TO_JSON(accessKeySecret, accessKeySecret_);
    DARABONBA_PTR_TO_JSON(securityToken, securityToken_);
    DARABONBA_PTR_TO_JSON(bearerToken, bearerToken_);
    DARABONBA_PTR_TO_JSON(type, type_);
    DARABONBA_PTR_TO_JSON(providerName, providerName_);
  };
  friend void from_json(const Darabonba::Json &j, CredentialModel &obj) {
    DARABONBA_PTR_FROM_JSON(accessKeyId, accessKeyId_);
    DARABONBA_PTR_FROM_JSON(accessKeySecret, accessKeySecret_);
    DARABONBA_PTR_FROM_JSON(securityToken, securityToken_);
    DARABONBA_PTR_FROM_JSON(bearerToken, bearerToken_);
    DARABONBA_PTR_FROM_JSON(type, type_);
    DARABONBA_PTR_FROM_JSON(providerName, providerName_);
  };
  CredentialModel() = default;
  CredentialModel(const CredentialModel &) = default;
  CredentialModel(CredentialModel &&) = default;
  CredentialModel(const Darabonba::Json &obj) { from_json(obj, *this); };
  virtual ~CredentialModel() = default;
  CredentialModel &operator=(const CredentialModel &) = default;
  CredentialModel &operator=(CredentialModel &&) = default;
  virtual void validate() const override {};
  virtual void fromMap(const Darabonba::Json &obj) override {
    from_json(obj, *this);
    validate();
  };
  virtual Darabonba::Json toMap() const override {
    Darabonba::Json obj;
    to_json(obj, *this);
    return obj;
  };
  virtual bool empty() const override {
    return this->accessKeyId_ == nullptr && this->accessKeySecret_ == nullptr &&
           this->securityToken_ == nullptr && this->bearerToken_ == nullptr &&
           this->type_ == nullptr && this->providerName_ == nullptr;
  };
  // accessKeyId Field Functions
  bool hasAccessKeyId() const { return this->accessKeyId_ != nullptr; };
  void deleteAccessKeyId() { this->accessKeyId_ = nullptr; };
  inline string getAccessKeyId() const {
    DARABONBA_PTR_GET_DEFAULT(accessKeyId_, "")
  };
  inline CredentialModel &setAccessKeyId(string accessKeyId) {
    DARABONBA_PTR_SET_VALUE(accessKeyId_, accessKeyId)
  };

  // accessKeySecret Field Functions
  bool hasAccessKeySecret() const { return this->accessKeySecret_ != nullptr; };
  void deleteAccessKeySecret() { this->accessKeySecret_ = nullptr; };
  inline string getAccessKeySecret() const {
    DARABONBA_PTR_GET_DEFAULT(accessKeySecret_, "")
  };
  inline CredentialModel &setAccessKeySecret(string accessKeySecret) {
    DARABONBA_PTR_SET_VALUE(accessKeySecret_, accessKeySecret)
  };

  // securityToken Field Functions
  bool hasSecurityToken() const { return this->securityToken_ != nullptr; };
  void deleteSecurityToken() { this->securityToken_ = nullptr; };
  inline string getSecurityToken() const {
    DARABONBA_PTR_GET_DEFAULT(securityToken_, "")
  };
  inline CredentialModel &setSecurityToken(string securityToken) {
    DARABONBA_PTR_SET_VALUE(securityToken_, securityToken)
  };

  // bearerToken Field Functions
  bool hasBearerToken() const { return this->bearerToken_ != nullptr; };
  void deleteBearerToken() { this->bearerToken_ = nullptr; };
  inline string getBearerToken() const {
    DARABONBA_PTR_GET_DEFAULT(bearerToken_, "")
  };
  inline CredentialModel &setBearerToken(string bearerToken) {
    DARABONBA_PTR_SET_VALUE(bearerToken_, bearerToken)
  };

  // type Field Functions
  bool hasType() const { return this->type_ != nullptr; };
  void deleteType() { this->type_ = nullptr; };
  inline string getType() const { DARABONBA_PTR_GET_DEFAULT(type_, "") };
  inline CredentialModel &setType(string type) {
    DARABONBA_PTR_SET_VALUE(type_, type)
  };

  // providerName Field Functions
  bool hasProviderName() const { return this->providerName_ != nullptr; };
  void deleteProviderName() { this->providerName_ = nullptr; };
  inline string getProviderName() const {
    DARABONBA_PTR_GET_DEFAULT(providerName_, "")
  };
  inline CredentialModel &setProviderName(string providerName) {
    DARABONBA_PTR_SET_VALUE(providerName_, providerName)
  };

protected:
  // accesskey id
  shared_ptr<string> accessKeyId_{};
  // accesskey secret
  shared_ptr<string> accessKeySecret_{};
  // security token
  shared_ptr<string> securityToken_{};
  // bearer token
  shared_ptr<string> bearerToken_{};
  // type
  shared_ptr<string> type_{};
  // provider name
  shared_ptr<string> providerName_{};
};

/**
 * Model for initing credential
 */
class ALIBABACLOUD_CREDENTIALS_EXPORT Config : public Darabonba::Model {
public:
  friend void to_json(Darabonba::Json &j, const Config &obj) {
    DARABONBA_PTR_TO_JSON(accessKeyId, accessKeyId_);
    DARABONBA_PTR_TO_JSON(accessKeySecret, accessKeySecret_);
    DARABONBA_PTR_TO_JSON(securityToken, securityToken_);
    DARABONBA_PTR_TO_JSON(bearerToken, bearerToken_);
    DARABONBA_PTR_TO_JSON(durationSeconds, durationSeconds_);
    DARABONBA_PTR_TO_JSON(roleArn, roleArn_);
    DARABONBA_PTR_TO_JSON(policy, policy_);
    DARABONBA_PTR_TO_JSON(roleSessionExpiration, roleSessionExpiration_);
    DARABONBA_PTR_TO_JSON(roleSessionName, roleSessionName_);
    DARABONBA_PTR_TO_JSON(publicKeyId, publicKeyId_);
    DARABONBA_PTR_TO_JSON(privateKeyFile, privateKeyFile_);
    DARABONBA_PTR_TO_JSON(roleName, roleName_);
    DARABONBA_PTR_TO_JSON(credentialsUri, credentialsUri_);
    DARABONBA_PTR_TO_JSON(type, type_);
    DARABONBA_PTR_TO_JSON(stsEndpoint, stsEndpoint_);
    DARABONBA_PTR_TO_JSON(stsRegionId, stsRegionId_);
    DARABONBA_PTR_TO_JSON(externalId, externalId_);
    DARABONBA_PTR_TO_JSON(regionId, regionId_);
    DARABONBA_PTR_TO_JSON(host, host_);
    DARABONBA_PTR_TO_JSON(oidcProviderArn, oidcProviderArn_);
    DARABONBA_PTR_TO_JSON(oidcTokenFilePath, oidcTokenFilePath_);
    DARABONBA_PTR_TO_JSON(proxy, proxy_);
    DARABONBA_PTR_TO_JSON(enableVpc, enableVpc_);
    DARABONBA_PTR_TO_JSON(timeout, timeout_);
    DARABONBA_PTR_TO_JSON(connectTimeout, connectTimeout_);
    DARABONBA_PTR_TO_JSON(disableIMDSv1, disableIMDSv1_);
    DARABONBA_PTR_TO_JSON(enableIMDSv2, enableIMDSv2_);
    DARABONBA_PTR_TO_JSON(metadataTokenDuration, metadataTokenDuration_);
    DARABONBA_PTR_TO_JSON(reuseLastProviderEnabled, reuseLastProviderEnabled_);
  };
  friend void from_json(const Darabonba::Json &j, Config &obj) {
    DARABONBA_PTR_FROM_JSON(accessKeyId, accessKeyId_);
    DARABONBA_PTR_FROM_JSON(accessKeySecret, accessKeySecret_);
    DARABONBA_PTR_FROM_JSON(securityToken, securityToken_);
    DARABONBA_PTR_FROM_JSON(bearerToken, bearerToken_);
    DARABONBA_PTR_FROM_JSON(durationSeconds, durationSeconds_);
    DARABONBA_PTR_FROM_JSON(roleArn, roleArn_);
    DARABONBA_PTR_FROM_JSON(policy, policy_);
    DARABONBA_PTR_FROM_JSON(roleSessionExpiration, roleSessionExpiration_);
    DARABONBA_PTR_FROM_JSON(roleSessionName, roleSessionName_);
    DARABONBA_PTR_FROM_JSON(publicKeyId, publicKeyId_);
    DARABONBA_PTR_FROM_JSON(privateKeyFile, privateKeyFile_);
    DARABONBA_PTR_FROM_JSON(roleName, roleName_);
    DARABONBA_PTR_FROM_JSON(credentialsUri, credentialsUri_);
    DARABONBA_PTR_FROM_JSON(type, type_);
    DARABONBA_PTR_FROM_JSON(stsEndpoint, stsEndpoint_);
    DARABONBA_PTR_FROM_JSON(stsRegionId, stsRegionId_);
    DARABONBA_PTR_FROM_JSON(externalId, externalId_);
    DARABONBA_PTR_FROM_JSON(regionId, regionId_);
    DARABONBA_PTR_FROM_JSON(host, host_);
    DARABONBA_PTR_FROM_JSON(oidcProviderArn, oidcProviderArn_);
    DARABONBA_PTR_FROM_JSON(oidcTokenFilePath, oidcTokenFilePath_);
    DARABONBA_PTR_FROM_JSON(proxy, proxy_);
    DARABONBA_PTR_FROM_JSON(enableVpc, enableVpc_);
    DARABONBA_PTR_FROM_JSON(timeout, timeout_);
    DARABONBA_PTR_FROM_JSON(connectTimeout, connectTimeout_);
    DARABONBA_PTR_FROM_JSON(disableIMDSv1, disableIMDSv1_);
    DARABONBA_PTR_FROM_JSON(enableIMDSv2, enableIMDSv2_);
    DARABONBA_PTR_FROM_JSON(metadataTokenDuration, metadataTokenDuration_);
    DARABONBA_PTR_FROM_JSON(reuseLastProviderEnabled,
                            reuseLastProviderEnabled_);
  };
  Config() = default;
  Config(const Config &) = default;
  Config(Config &&) = default;
  Config(const Darabonba::Json &obj) { from_json(obj, *this); };
  virtual ~Config() = default;
  Config &operator=(const Config &) = default;
  Config &operator=(Config &&) = default;
  virtual void validate() const override {};
  virtual void fromMap(const Darabonba::Json &obj) override {
    from_json(obj, *this);
    validate();
  };
  virtual Darabonba::Json toMap() const override {
    Darabonba::Json obj;
    to_json(obj, *this);
    return obj;
  };
  virtual bool empty() const override {
    return this->accessKeyId_ == nullptr && this->accessKeySecret_ == nullptr &&
           this->securityToken_ == nullptr && this->bearerToken_ == nullptr &&
           this->durationSeconds_ == nullptr && this->roleArn_ == nullptr &&
           this->policy_ == nullptr &&
           this->roleSessionExpiration_ == nullptr &&
           this->roleSessionName_ == nullptr && this->publicKeyId_ == nullptr &&
           this->privateKeyFile_ == nullptr && this->roleName_ == nullptr &&
           this->credentialsUri_ == nullptr && this->type_ == nullptr &&
           this->stsEndpoint_ == nullptr &&
           this->stsRegionId_ == nullptr && this->externalId_ == nullptr &&
           this->regionId_ == nullptr && this->host_ == nullptr &&
           this->oidcProviderArn_ == nullptr &&
           this->oidcTokenFilePath_ == nullptr && this->proxy_ == nullptr &&
           this->enableVpc_ == nullptr && this->timeout_ == nullptr &&
           this->connectTimeout_ == nullptr &&
           this->disableIMDSv1_ == nullptr &&
           this->enableIMDSv2_ == nullptr &&
           this->metadataTokenDuration_ == nullptr &&
           this->reuseLastProviderEnabled_ == nullptr;
  };
  // accessKeyId Field Functions
  bool hasAccessKeyId() const { return this->accessKeyId_ != nullptr; };
  void deleteAccessKeyId() { this->accessKeyId_ = nullptr; };
  inline string getAccessKeyId() const {
    DARABONBA_PTR_GET_DEFAULT(accessKeyId_, "")
  };
  inline Config &setAccessKeyId(string accessKeyId) {
    DARABONBA_PTR_SET_VALUE(accessKeyId_, accessKeyId)
  };

  // accessKeySecret Field Functions
  bool hasAccessKeySecret() const { return this->accessKeySecret_ != nullptr; };
  void deleteAccessKeySecret() { this->accessKeySecret_ = nullptr; };
  inline string getAccessKeySecret() const {
    DARABONBA_PTR_GET_DEFAULT(accessKeySecret_, "")
  };
  inline Config &setAccessKeySecret(string accessKeySecret) {
    DARABONBA_PTR_SET_VALUE(accessKeySecret_, accessKeySecret)
  };

  // securityToken Field Functions
  bool hasSecurityToken() const { return this->securityToken_ != nullptr; };
  void deleteSecurityToken() { this->securityToken_ = nullptr; };
  inline string getSecurityToken() const {
    DARABONBA_PTR_GET_DEFAULT(securityToken_, "")
  };
  inline Config &setSecurityToken(string securityToken) {
    DARABONBA_PTR_SET_VALUE(securityToken_, securityToken)
  };

  // bearerToken Field Functions
  bool hasBearerToken() const { return this->bearerToken_ != nullptr; };
  void deleteBearerToken() { this->bearerToken_ = nullptr; };
  inline string getBearerToken() const {
    DARABONBA_PTR_GET_DEFAULT(bearerToken_, "")
  };
  inline Config &setBearerToken(string bearerToken) {
    DARABONBA_PTR_SET_VALUE(bearerToken_, bearerToken)
  };

  // durationSeconds Field Functions
  bool hasDurationSeconds() const { return this->durationSeconds_ != nullptr; };
  void deleteDurationSeconds() { this->durationSeconds_ = nullptr; };
  inline int64_t getDurationSeconds() const {
    DARABONBA_PTR_GET_DEFAULT(durationSeconds_, 0)
  };
  inline Config &setDurationSeconds(int64_t durationSeconds) {
    DARABONBA_PTR_SET_VALUE(durationSeconds_, durationSeconds)
  };

  // roleArn Field Functions
  bool hasRoleArn() const { return this->roleArn_ != nullptr; };
  void deleteRoleArn() { this->roleArn_ = nullptr; };
  inline string getRoleArn() const { DARABONBA_PTR_GET_DEFAULT(roleArn_, "") };
  inline Config &setRoleArn(string roleArn) {
    DARABONBA_PTR_SET_VALUE(roleArn_, roleArn)
  };

  // policy Field Functions
  bool hasPolicy() const { return this->policy_ != nullptr; };
  void deletePolicy() { this->policy_ = nullptr; };
  inline string getPolicy() const { DARABONBA_PTR_GET_DEFAULT(policy_, "") };
  inline Config &setPolicy(string policy) {
    DARABONBA_PTR_SET_VALUE(policy_, policy)
  };

  // roleSessionExpiration Field Functions
  bool hasRoleSessionExpiration() const {
    return this->roleSessionExpiration_ != nullptr;
  };
  void deleteRoleSessionExpiration() {
    this->roleSessionExpiration_ = nullptr;
  };
  inline int64_t getRoleSessionExpiration() const {
    DARABONBA_PTR_GET_DEFAULT(roleSessionExpiration_, 0)
  };
  inline Config &setRoleSessionExpiration(int64_t roleSessionExpiration) {
    DARABONBA_PTR_SET_VALUE(roleSessionExpiration_, roleSessionExpiration)
  };

  // roleSessionName Field Functions
  bool hasRoleSessionName() const { return this->roleSessionName_ != nullptr; };
  void deleteRoleSessionName() { this->roleSessionName_ = nullptr; };
  std::string getRoleSessionName() const;  // Implemented in Model.cpp with dynamic default
  inline Config &setRoleSessionName(string roleSessionName) {
    DARABONBA_PTR_SET_VALUE(roleSessionName_, roleSessionName)
  };

  // publicKeyId Field Functions
  bool hasPublicKeyId() const { return this->publicKeyId_ != nullptr; };
  void deletePublicKeyId() { this->publicKeyId_ = nullptr; };
  inline string getPublicKeyId() const {
    DARABONBA_PTR_GET_DEFAULT(publicKeyId_, "")
  };
  inline Config &setPublicKeyId(string publicKeyId) {
    DARABONBA_PTR_SET_VALUE(publicKeyId_, publicKeyId)
  };

  // privateKeyFile Field Functions
  bool hasPrivateKeyFile() const { return this->privateKeyFile_ != nullptr; };
  void deletePrivateKeyFile() { this->privateKeyFile_ = nullptr; };
  inline string getPrivateKeyFile() const {
    DARABONBA_PTR_GET_DEFAULT(privateKeyFile_, "")
  };
  inline Config &setPrivateKeyFile(string privateKeyFile) {
    DARABONBA_PTR_SET_VALUE(privateKeyFile_, privateKeyFile)
  };

  // roleName Field Functions
  bool hasRoleName() const { return this->roleName_ != nullptr; };
  void deleteRoleName() { this->roleName_ = nullptr; };
  inline string getRoleName() const {
    DARABONBA_PTR_GET_DEFAULT(roleName_, "")
  };
  inline Config &setRoleName(string roleName) {
    DARABONBA_PTR_SET_VALUE(roleName_, roleName)
  };

  // credentialsUri Field Functions
  bool hasCredentialsUri() const { return this->credentialsUri_ != nullptr; };
  void deleteCredentialsUri() { this->credentialsUri_ = nullptr; };
  inline string getCredentialsUri() const {
    DARABONBA_PTR_GET_DEFAULT(credentialsUri_, "")
  };
  inline Config &setCredentialsUri(string credentialsUri) {
    DARABONBA_PTR_SET_VALUE(credentialsUri_, credentialsUri)
  };

  // type Field Functions
  bool hasType() const { return this->type_ != nullptr; };
  void deleteType() { this->type_ = nullptr; };
  inline string getType() const { DARABONBA_PTR_GET_DEFAULT(type_, "") };
  inline Config &setType(string type) { DARABONBA_PTR_SET_VALUE(type_, type) };

  // stsEndpoint Field Functions
  bool hasStsEndpoint() const { return this->stsEndpoint_ != nullptr; };
  void deleteStsEndpoint() { this->stsEndpoint_ = nullptr; };
  inline string getStsEndpoint() const {
    DARABONBA_PTR_GET_DEFAULT(stsEndpoint_, "")
  };
  inline Config &setStsEndpoint(string stsEndpoint) {
    DARABONBA_PTR_SET_VALUE(stsEndpoint_, stsEndpoint)
  };

  // stsRegionId Field Functions
  bool hasStsRegionId() const { return this->stsRegionId_ != nullptr; };
  void deleteStsRegionId() { this->stsRegionId_ = nullptr; };
  inline string getStsRegionId() const {
    DARABONBA_PTR_GET_DEFAULT(stsRegionId_, "")
  };
  inline Config &setStsRegionId(string stsRegionId) {
    DARABONBA_PTR_SET_VALUE(stsRegionId_, stsRegionId)
  };

  // externalId Field Functions
  bool hasExternalId() const { return this->externalId_ != nullptr; };
  void deleteExternalId() { this->externalId_ = nullptr; };
  inline string getExternalId() const {
    DARABONBA_PTR_GET_DEFAULT(externalId_, "")
  };
  inline Config &setExternalId(string externalId) {
    DARABONBA_PTR_SET_VALUE(externalId_, externalId)
  };

  // regionId Field Functions
  bool hasRegionId() const { return this->regionId_ != nullptr; };
  void deleteRegionId() { this->regionId_ = nullptr; };
  inline string getRegionId() const {
    DARABONBA_PTR_GET_DEFAULT(regionId_, "")
  };
  inline Config &setRegionId(string regionId) {
    DARABONBA_PTR_SET_VALUE(regionId_, regionId)
  };

  // host Field Functions
  bool hasHost() const { return this->host_ != nullptr; };
  void deleteHost() { this->host_ = nullptr; };
  inline string getHost() const { DARABONBA_PTR_GET_DEFAULT(host_, "") };
  inline Config &setHost(string host) { DARABONBA_PTR_SET_VALUE(host_, host) };

  // oidcProviderArn Field Functions
  bool hasOidcProviderArn() const { return this->oidcProviderArn_ != nullptr; };
  void deleteOidcProviderArn() { this->oidcProviderArn_ = nullptr; };
  inline string getOidcProviderArn() const {
    DARABONBA_PTR_GET_DEFAULT(oidcProviderArn_, "")
  };
  inline Config &setOidcProviderArn(string oidcProviderArn) {
    DARABONBA_PTR_SET_VALUE(oidcProviderArn_, oidcProviderArn)
  };

  // oidcTokenFilePath Field Functions
  bool hasOidcTokenFilePath() const {
    return this->oidcTokenFilePath_ != nullptr;
  };
  void deleteOidcTokenFilePath() { this->oidcTokenFilePath_ = nullptr; };
  inline string getOidcTokenFilePath() const {
    DARABONBA_PTR_GET_DEFAULT(oidcTokenFilePath_, "")
  };
  inline Config &setOidcTokenFilePath(string oidcTokenFilePath) {
    DARABONBA_PTR_SET_VALUE(oidcTokenFilePath_, oidcTokenFilePath)
  };

  // proxy Field Functions
  bool hasProxy() const { return this->proxy_ != nullptr; };
  void deleteProxy() { this->proxy_ = nullptr; };
  inline string getProxy() const { DARABONBA_PTR_GET_DEFAULT(proxy_, "") };
  inline Config &setProxy(string proxy) {
    DARABONBA_PTR_SET_VALUE(proxy_, proxy)
  };

  // enableVpc Field Functions
  bool hasEnableVpc() const { return this->enableVpc_ != nullptr; };
  void deleteEnableVpc() { this->enableVpc_ = nullptr; };
  inline bool getEnableVpc() const {
    DARABONBA_PTR_GET_DEFAULT(enableVpc_, false)
  };
  inline Config &setEnableVpc(bool enableVpc) {
    DARABONBA_PTR_SET_VALUE(enableVpc_, enableVpc)
  };

  // timeout Field Functions
  bool hasTimeout() const { return this->timeout_ != nullptr; };
  void deleteTimeout() { this->timeout_ = nullptr; };
  inline int64_t getTimeout() const { DARABONBA_PTR_GET_DEFAULT(timeout_, 5000) };
  inline Config &setTimeout(int64_t timeout) {
    DARABONBA_PTR_SET_VALUE(timeout_, timeout)
  };

  // connectTimeout Field Functions
  bool hasConnectTimeout() const { return this->connectTimeout_ != nullptr; };
  void deleteConnectTimeout() { this->connectTimeout_ = nullptr; };
  inline int64_t getConnectTimeout() const {
    DARABONBA_PTR_GET_DEFAULT(connectTimeout_, 10000)
  };
  inline Config &setConnectTimeout(int64_t connectTimeout) {
    DARABONBA_PTR_SET_VALUE(connectTimeout_, connectTimeout)
  };

  // disableIMDSv1 Field Functions
  bool hasDisableIMDSv1() const { return this->disableIMDSv1_ != nullptr; };
  void deleteDisableIMDSv1() { this->disableIMDSv1_ = nullptr; };
  inline bool getDisableIMDSv1() const {
    DARABONBA_PTR_GET_DEFAULT(disableIMDSv1_, false)
  };
  inline Config &setDisableIMDSv1(bool disableIMDSv1) {
    DARABONBA_PTR_SET_VALUE(disableIMDSv1_, disableIMDSv1)
  };

  // reuseLastProviderEnabled Field Functions
  bool hasReuseLastProviderEnabled() const {
    return this->reuseLastProviderEnabled_ != nullptr;
  };
  void deleteReuseLastProviderEnabled() {
    this->reuseLastProviderEnabled_ = nullptr;
  };
  inline bool getReuseLastProviderEnabled() const {
    DARABONBA_PTR_GET_DEFAULT(reuseLastProviderEnabled_, false)
  };
  inline Config &setReuseLastProviderEnabled(bool reuseLastProviderEnabled) {
    DARABONBA_PTR_SET_VALUE(reuseLastProviderEnabled_, reuseLastProviderEnabled)
  };

  // enableIMDSv2 Field Functions (aligned with Python version)
  bool hasEnableIMDSv2() const { return this->enableIMDSv2_ != nullptr; };
  void deleteEnableIMDSv2() { this->enableIMDSv2_ = nullptr; };
  inline bool getEnableIMDSv2() const {
    DARABONBA_PTR_GET_DEFAULT(enableIMDSv2_, false)
  };
  inline Config &setEnableIMDSv2(bool enableIMDSv2) {
    DARABONBA_PTR_SET_VALUE(enableIMDSv2_, enableIMDSv2)
  };

  // metadataTokenDuration Field Functions (aligned with Python version)
  bool hasMetadataTokenDuration() const { return this->metadataTokenDuration_ != nullptr; };
  void deleteMetadataTokenDuration() { this->metadataTokenDuration_ = nullptr; };
  inline int64_t getMetadataTokenDuration() const {
    DARABONBA_PTR_GET_DEFAULT(metadataTokenDuration_, 21600)  // 6 hours default
  };
  inline Config &setMetadataTokenDuration(int64_t metadataTokenDuration) {
    DARABONBA_PTR_SET_VALUE(metadataTokenDuration_, metadataTokenDuration)
  };

protected:
  // accesskey id
  shared_ptr<string> accessKeyId_{};
  // accesskey secret
  shared_ptr<string> accessKeySecret_{};
  // security token
  shared_ptr<string> securityToken_{};
  // bearer token
  shared_ptr<string> bearerToken_{};
  // duration seconds
  shared_ptr<int64_t> durationSeconds_ = make_shared<int64_t>(3600);
  // role arn
  shared_ptr<string> roleArn_{};
  // policy
  shared_ptr<string> policy_{};
  // role session expiration
  shared_ptr<int64_t> roleSessionExpiration_{};
  // role session name
  shared_ptr<string> roleSessionName_{};
  // publicKey id
  shared_ptr<string> publicKeyId_{};
  // privateKey file
  shared_ptr<string> privateKeyFile_{};
  // role name
  shared_ptr<string> roleName_{};
  // credentials uri
  shared_ptr<string> credentialsUri_{};
  // credential type
  shared_ptr<string> type_{};
  // sts endpoint
  shared_ptr<string> stsEndpoint_ = make_shared<std::string>("sts.aliyuncs.com");
  // sts region id
  shared_ptr<string> stsRegionId_{};
  // external id for ram role arn
  shared_ptr<string> externalId_{};
  // regionId
  shared_ptr<string> regionId_ = make_shared<std::string>("cn-hangzhou");
  // host
  shared_ptr<string> host_{};
  // oidc provider arn
  shared_ptr<string> oidcProviderArn_{};
  // oidc token file path
  shared_ptr<string> oidcTokenFilePath_{};
  // proxy
  shared_ptr<string> proxy_{};
  // enable vpc
  shared_ptr<bool> enableVpc_ = make_shared<bool>(false);
  // timeout
  shared_ptr<int64_t> timeout_ = make_shared<int64_t>(5000);
  // connect timeout
  shared_ptr<int64_t> connectTimeout_ = make_shared<int64_t>(10000);
  // disable IMDSv1
  shared_ptr<bool> disableIMDSv1_ = make_shared<bool>(false);
  // enable IMDSv2 (aligned with Python version)
  shared_ptr<bool> enableIMDSv2_ = make_shared<bool>(false);
  // metadata token duration (aligned with Python version)
  shared_ptr<int64_t> metadataTokenDuration_ = make_shared<int64_t>(21600);
  // reuse last provider enabled
  shared_ptr<bool> reuseLastProviderEnabled_ = make_shared<bool>(false);
};

} // namespace Models
} // namespace Credentials
} // namespace AlibabaCloud

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_POP

#endif