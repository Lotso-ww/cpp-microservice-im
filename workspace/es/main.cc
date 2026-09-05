#include "../common/icindex.hpp"
#include <cstdint>
#include <elasticlient/client.h>
#include <gflags/gflags.h>
#include <memory>

DEFINE_bool(run_mode, false, "程序运行模式, false-调试 true-发布;");
DEFINE_string(log_file, "", "发布模式下, 用于指定日志的输出文件;");
DEFINE_int32(log_level, 0, "发布模式下, 用于指定日志输出等级");

int main(int argc, char *argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    std::vector<std::string> host_list = {"http://127.0.0.1:9200/"};
    auto client = std::make_shared<elasticlient::Client>(host_list);
    std::cout << (uint64_t)client.get() << std::endl;
    bool ret = ESIndex("test_user", "_doc", client).append("nickname")
                                                   .append("phone", "keyword", "standard", true)
                                                   .create();                            
    return 0;
}
