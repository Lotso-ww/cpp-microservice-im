#pragma once
#include <alibabacloud/Dypnsapi20170525.hpp>
#include <memory>
#include <string>
#include <cstdlib>
#include "../common/logger.hpp"

// 阿里云号码认证服务(Dypnsapi) —— 业务配置硬编码，AK/SK 自动从环境变量读
class DMS
{
public:
    using ptr = std::shared_ptr<DMS>;

    // 业务固定配置
    static constexpr const char *SIGN_NAME      = "恒创联众";
    static constexpr const char *TEMPLATE_CODE  = "100001";
    static constexpr const char *TEMPLATE_PARAM = R"({"code":"##code##","min":"5"})";

    // 零参数构造 —— AK/SK 自动从环境变量 ALIBABA_CLOUD_ACCESS_KEY_ID / SECRET 读
    DMS()
    {
        const char *ak = getenv("ALIBABA_CLOUD_ACCESS_KEY_ID");
        const char *sk = getenv("ALIBABA_CLOUD_ACCESS_KEY_SECRET");
        if(!ak || !*ak || !sk || !*sk)
            throw std::runtime_error("环境变量 ALIBABA_CLOUD_ACCESS_KEY_ID / ACCESS_KEY_SECRET 未设置");

        AlibabaCloud::OpenApi::Utils::Models::Config config;
        config.setAccessKeyId(ak);
        config.setAccessKeySecret(sk);
        config.setRegionId("cn-hangzhou");
        config.setEndpoint("dypnsapi.aliyuncs.com");
        _client = std::make_shared<AlibabaCloud::Dypnsapi20170525::Client>(config);
    }

    // 发送短信验证码 —— 只传手机号，阿里云自动生成验证码
    // 返回值：阿里云生成的验证码（returnVerifyCode=true），空串表示发送失败
    std::string send(const std::string &phone)
    {
        try
        {
            AlibabaCloud::Dypnsapi20170525::Models::SendSmsVerifyCodeRequest request;
            request.setPhoneNumber(phone);
            request.setSignName(SIGN_NAME);
            request.setTemplateCode(TEMPLATE_CODE);
            request.setTemplateParam(TEMPLATE_PARAM);
            request.setReturnVerifyCode(true);  // 阿里云生成验证码并回传

            auto body = _client->sendSmsVerifyCode(request).body();
            if(!body.success())
            {
                LOG_ERROR("短信发送失败: code={}, message={}", body.code(), body.message());
                return "";
            }
            std::string code = body.hasModel() ? body.model().verifyCode() : "";
            LOG_INFO("短信发送成功: phone={}, bizId={}, verifyCode={}", phone,
                     body.hasModel() ? body.model().bizId() : "", code);
            return code;
        }
        catch(const std::exception &e)
        {
            LOG_ERROR("短信发送异常: {}", e.what());
            return "";
        }
    }

private:
    std::shared_ptr<AlibabaCloud::Dypnsapi20170525::Client> _client;
};
