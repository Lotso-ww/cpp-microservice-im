#include "../../common/dms.hpp"
#include <gflags/gflags.h>

DEFINE_bool(run_mode, false, "程序运行模式, false-调试 true-发布");
DEFINE_string(log_file, "", "发布模式下, 用于指定日志的输出文件");
DEFINE_int32(log_level, 0, "发布模式下, 用于指定日志输出等级");

DEFINE_string(phone_number, "15827502236", "手机号码");

int main(int argc, char *argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    try
    {
        DMS dms;
        auto code = dms.send(FLAGS_phone_number);
        if(code.empty()) return -1;
        LOG_INFO("发送成功, 验证码: {}", code);
    }
    catch(const std::exception &e)
    {
        LOG_ERROR("DMS 初始化失败: {}", e.what());
        return -1;
    }
    return 0;
}
