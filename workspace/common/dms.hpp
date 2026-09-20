#pragma once
#include <alibabacloud/Dypnsapi20170525.hpp>
#include <memory>
#include <string>
#include "../common/logger.hpp"

// 阿里云号码认证服务(Dypnsapi)客户端封装
class DMS
{
public:
    using ptr = std::shared_ptr<DMS>;

    // signName / templateCode / templateParam 是业务必填配置，构造时必须传
    // regionId / endpoint / codeType / codeLength / validTime / interval / returnVerifyCode 有默认值
    DMS(const std::string &accessKeyId,
        const std::string &accessKeySecret,
        const std::string &signName,
        const std::string &templateCode,
        const std::string &templateParam = R"({"code":"##code##","min":"5"})",
        const std::string &regionId = "cn-hangzhou",
        const std::string &endpoint = "dypnsapi.aliyuncs.com")
        : _signName(signName), _templateCode(templateCode), _templateParam(templateParam)
    {
        AlibabaCloud::OpenApi::Utils::Models::Config config;
        config.setAccessKeyId(accessKeyId);
        config.setAccessKeySecret(accessKeySecret);
        config.setRegionId(regionId);
        config.setEndpoint(endpoint);
        _client = std::make_shared<AlibabaCloud::Dypnsapi20170525::Client>(config);
    }

    struct SendResult
    {
        bool success = false;
        std::string requestId;
        std::string bizId;
        std::string verifyCode;
        std::string code;
        std::string message;
    };

    // 发送短信验证码 —— 只传手机号即可，其他配置使用构造时传入的默认值
    SendResult sendSmsVerifyCode(const std::string &phoneNumber)
    {
        return sendSmsVerifyCode(phoneNumber, "", 1, 6, 300, 60, true);
    }

    // 重载版本：需要覆盖部分默认参数时使用（高级场景）
    SendResult sendSmsVerifyCode(const std::string &phoneNumber,
                                 const std::string &schemeName,
                                 int64_t codeType = 1,
                                 int64_t codeLength = 6,
                                 int64_t validTime = 300,
                                 int64_t interval = 60,
                                 bool returnVerifyCode = true)
    {
        SendResult result;
        try
        {
            AlibabaCloud::Dypnsapi20170525::Models::SendSmsVerifyCodeRequest request;
            request.setPhoneNumber(phoneNumber);
            request.setSignName(_signName);
            request.setTemplateCode(_templateCode);
            request.setTemplateParam(_templateParam);
            if(!schemeName.empty()) request.setSchemeName(schemeName);
            request.setCodeType(codeType);
            request.setCodeLength(codeLength);
            request.setValidTime(validTime);
            request.setInterval(interval);
            request.setReturnVerifyCode(returnVerifyCode);

            auto response = _client->sendSmsVerifyCode(request);
            auto body = response.body();
            result.code = body.code();
            result.message = body.message();
            result.success = body.success();
            result.requestId = body.requestId();
            if(body.hasModel())
            {
                auto model = body.model();
                result.bizId = model.bizId();
                result.verifyCode = model.verifyCode();
            }
            if(!result.success)
            {
                LOG_ERROR("发送短信验证码失败: code={}, message={}, requestId={}",
                          result.code, result.message, result.requestId);
            }
            else
            {
                LOG_INFO("发送短信验证码成功: requestId={}, bizId={}",
                         result.requestId, result.bizId);
            }
        }
        catch(const std::exception &e)
        {
            LOG_ERROR("发送短信验证码异常: {}", e.what());
        }
        return result;
    }

private:
    std::shared_ptr<AlibabaCloud::Dypnsapi20170525::Client> _client;
    std::string _signName;
    std::string _templateCode;
    std::string _templateParam;
};
