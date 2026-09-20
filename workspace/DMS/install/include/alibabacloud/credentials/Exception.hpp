#ifndef ALIBABACLOUD_CREDENTIALS_EXCEPTION_HPP_
#define ALIBABACLOUD_CREDENTIALS_EXCEPTION_HPP_

#include <exception>
#include <string>
#include <map>
#include <darabonba/Core.hpp>
#include <alibabacloud/credentials/Export.hpp>

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_PUSH

namespace AlibabaCloud {
namespace Credentials {

/**
 * @brief Custom exception class for credential operations
 * Supports multiple construction methods and comprehensive error information
 */
class ALIBABACLOUD_CREDENTIALS_EXPORT CredentialException : public std::exception {
public:
    /**
     * @brief Default constructor
     */
    CredentialException() {}

    /**
     * @brief Construct with message only - primary constructor for string literals
     * @param message Error message
     */
    explicit CredentialException(const std::string& message) 
        : message_(message) {}

    /**
     * @brief Construct with message, code and request_id
     * @param message Error message
     * @param code Error code (optional)
     * @param request_id Request ID (optional)
     */
    CredentialException(const std::string& message, 
                       const std::string& code, 
                       const std::string& request_id = "")
        : code_(code), message_(message), request_id_(request_id) {}

    /**
     * @brief Construct from JSON
     * @param json JSON object containing error information
     */
    explicit CredentialException(const Darabonba::Json& json) {
        if (json.contains("code") && !json["code"].is_null()) {
            code_ = json["code"].get<std::string>();
        }
        
        if (json.contains("message") && !json["message"].is_null()) {
            message_ = json["message"].get<std::string>();
        }
        
        if (json.contains("description") && !json["description"].is_null()) {
            description_ = json["description"].get<std::string>();
        }
        
        if (json.contains("requestId") && !json["requestId"].is_null()) {
            request_id_ = json["requestId"].get<std::string>();
        }
        
        if (json.contains("statusCode") && !json["statusCode"].is_null()) {
            statusCode_ = json["statusCode"].get<int>();
        }
        
        if (json.contains("data") && !json["data"].is_null()) {
            data_ = json["data"];
        }
        
        if (json.contains("accessDeniedDetail") && !json["accessDeniedDetail"].is_null()) {
            accessDeniedDetail_ = json["accessDeniedDetail"];
        }
    }

    CredentialException(const CredentialException&) = default;
    CredentialException(CredentialException&&) = default;
    CredentialException& operator=(const CredentialException&) = default;
    CredentialException& operator=(CredentialException&&) = default;

    /**
     * @brief Static factory method for JSON construction to avoid ambiguity
     * @param json JSON object containing error information
     * @return CredentialException instance
     */
    static CredentialException fromJson(const Darabonba::Json& json) {
        CredentialException ex;
        if (json.contains("code") && !json["code"].is_null()) {
            ex.code_ = json["code"].get<std::string>();
        }
        if (json.contains("message") && !json["message"].is_null()) {
            ex.message_ = json["message"].get<std::string>();
        }
        if (json.contains("description") && !json["description"].is_null()) {
            ex.description_ = json["description"].get<std::string>();
        }
        if (json.contains("requestId") && !json["requestId"].is_null()) {
            ex.request_id_ = json["requestId"].get<std::string>();
        }
        if (json.contains("statusCode") && !json["statusCode"].is_null()) {
            ex.statusCode_ = json["statusCode"].get<int>();
        }
        if (json.contains("data") && !json["data"].is_null()) {
            ex.data_ = json["data"];
        }
        if (json.contains("accessDeniedDetail") && !json["accessDeniedDetail"].is_null()) {
            ex.accessDeniedDetail_ = json["accessDeniedDetail"];
        }
        return ex;
    }

    virtual ~CredentialException() = default;

    /**
     * @brief Get the error message
     * @return const char* Error message
     */
    const char* what() const noexcept override {
        return message_.c_str();
    }

    // Getters
    const std::string& getCode() const { return code_; }
    const std::string& getMessage() const { return message_; }
    const std::string& getRequestId() const { return request_id_; }
    const std::string& getDescription() const { return description_; }
    int getStatusCode() const { return statusCode_; }
    const Darabonba::Json& getData() const { return data_; }
    const Darabonba::Json& getAccessDeniedDetail() const { return accessDeniedDetail_; }

    // Setters
    void setCode(const std::string& code) { code_ = code; }
    void setMessage(const std::string& message) { message_ = message; }
    void setRequestId(const std::string& request_id) { request_id_ = request_id; }
    void setDescription(const std::string& description) { description_ = description; }
    void setStatusCode(int statusCode) { statusCode_ = statusCode; }
    void setData(const Darabonba::Json& data) { data_ = data; }
    void setAccessDeniedDetail(const Darabonba::Json& detail) { accessDeniedDetail_ = detail; }

    /**
     * @brief Convert to JSON representation
     * @return Darabonba::Json JSON object
     */
    Darabonba::Json toJson() const {
        return Darabonba::Json{
            {"code", code_},
            {"message", message_},
            {"description", description_},
            {"requestId", request_id_},
            {"statusCode", statusCode_},
            {"data", data_},
            {"accessDeniedDetail", accessDeniedDetail_}
        };
    }

private:
    std::string code_;
    std::string message_;
    std::string description_;
    std::string request_id_;
    int statusCode_ = 0;
    Darabonba::Json data_;
    Darabonba::Json accessDeniedDetail_;
};

} // namespace Credentials
} // namespace AlibabaCloud

ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_POP

#endif // ALIBABACLOUD_CREDENTIALS_EXCEPTION_HPP_
