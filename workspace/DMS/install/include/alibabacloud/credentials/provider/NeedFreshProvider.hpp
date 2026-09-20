#ifndef ALIBABACLOUD_CREDENTIALS_NEEDFRESHPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIALS_NEEDFRESHPROVIDER_HPP_

#include <ctime>
#include <mutex>
#include <sstream>
#include <iomanip>

#include <alibabacloud/credentials/provider/Provider.hpp>
namespace AlibabaCloud {
namespace Credentials {
class ALIBABACLOUD_CREDENTIALS_EXPORT NeedFreshProvider : public Provider {
public:
  NeedFreshProvider() = default;
  NeedFreshProvider(long long expiration) : expiration_(expiration) {}
  virtual ~NeedFreshProvider() {}

  /**
   * @brief Get credential (thread safe)
   * @note Returns a copy to ensure consistency between AK and SK.
   */
  virtual Models::CredentialModel getCredential() const override {
    std::lock_guard<std::mutex> lock(refreshMutex_);
    refresh();
    return credential_;
  }

protected:
  virtual bool needFresh() const {
    auto now = static_cast<decltype(expiration_)>(time(nullptr));
    return expiration_ - now <= 180;
  }

  /**
   * @brief Refresh credential
   * @return true if a new credential was fetched, false otherwise
   * @note Subclasses should create a complete CredentialModel with matching
   *       AK/SK pair before assigning to credential_. Do not modify credential_
   *       field by field as this can cause inconsistency during copy.
   */
  virtual bool refreshCredential() const = 0;

  virtual void refresh() const {
    if (needFresh()) {
      refreshCredential();
    }
  }

  static int64_t strtotime(const std::string &gmt) {
#ifndef _WIN32
    tm tm{};
    if (strptime(gmt.c_str(), "%Y-%m-%dT%H:%M:%SZ", &tm) == nullptr) {
      return 0; // parse failed
    }
    time_t t = timegm(&tm);
    return static_cast<int64_t>(t);
#else
    std::tm tm{};
    std::istringstream ss(gmt);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    if (ss.fail()) {
      return 0; // parse failed
    }
    time_t t = _mkgmtime(&tm);
    return static_cast<int64_t>(t);
#endif
  }

  static std::string gmt_datetime() {
    time_t now;
    time(&now);
#ifndef _WIN32
    char buf[21];
    if (strftime(buf, sizeof buf, "%FT%TZ", gmtime(&now)) == 0) {
      return std::string();
    }
    return buf;
#else
    std::tm tm{};
    if (gmtime_s(&tm, &now) != 0) {
      return std::string();
    }
    char buf[21];
    if (std::strftime(buf, sizeof(buf), "%FT%TZ", &tm) == 0) {
      return std::string();
    }
    return std::string(buf);
#endif
  }

  mutable std::mutex refreshMutex_;
  mutable Models::CredentialModel credential_;
  mutable int64_t expiration_ = 0;
};
} // namespace Credentials
} // namespace AlibabaCloud

#endif