#include "logger.hpp"
#include <gflags/gflags.h>
#include <iostream>

DEFINE_bool(run_mode, false, "程序运行模式, false-调试 true-发布;");
DEFINE_string(log_file, "", "发布模式下, 用于指定日志的输出文件;");
DEFINE_int32(log_level, 0, "发布模式下, 用于指定日志输出等级");

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level); // 注意不要忘记前缀

    // 打印内容
    LOG_TRACE("你好: {}", "Lotso");
    LOG_DEBUG("你好: {}", "Lotso");
    LOG_INFO("你好: {}", "Lotso");
    LOG_WARN("你好: {}", "Lotso");
    LOG_ERROR("你好: {}", "Lotso");
    LOG_FATAL("你好: {}", "Lotso");
    LOG_DEBUG("这是测试##作用的");
    return -1;
}