#include "../../common/dms.hpp"
#include <gflags/gflags.h>
#include <cstdlib>

DEFINE_bool(run_mode, false, "程序运行模式, false-调试 true-发布");
DEFINE_string(log_file, "", "发布模式下, 用于指定日志的输出文件");
DEFINE_int32(log_level, 0, "发布模式下, 用于指定日志输出等级");

DEFINE_string(access_key_id, "", "阿里云 AccessKey ID");
DEFINE_string(access_key_secret, "", "阿里云 AccessKey Secret");

DEFINE_string(phone_number, "15827502236", "手机号码");
DEFINE_string(sign_name, "恒创联众", "短信签名名称");
DEFINE_string(template_code, "100001", "短信模板CODE");
DEFINE_string(template_param, R"({"code":"##code##","min":"5"})", "模板变量JSON");

int main(int argc, char *argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    std::string ak = FLAGS_access_key_id;
    std::string sk = FLAGS_access_key_secret;
    if(ak.empty()) ak = getenv("ALIBABA_CLOUD_ACCESS_KEY_ID") ? getenv("ALIBABA_CLOUD_ACCESS_KEY_ID") : "";
    if(sk.empty()) sk = getenv("ALIBABA_CLOUD_ACCESS_KEY_SECRET") ? getenv("ALIBABA_CLOUD_ACCESS_KEY_SECRET") : "";
    if(ak.empty() || sk.empty())
    {
        LOG_ERROR("请设置 AccessKey (--access_key_id/--access_key_secret 或环境变量)");
        return -1;
    }

    DMS dms(ak, sk, FLAGS_sign_name, FLAGS_template_code, FLAGS_template_param);

    LOG_INFO("发送短信验证码到 {}", FLAGS_phone_number);
    auto result = dms.sendSmsVerifyCode(FLAGS_phone_number);

    if(result.success)
        LOG_INFO("发送成功! bizId={}, verifyCode={}", result.bizId, result.verifyCode);
    else
    {
        LOG_ERROR("发送失败: code={}, message={}", result.code, result.message);
        return -1;
    }
    return 0;
}
