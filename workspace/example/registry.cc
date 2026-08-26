#include "../common/etcd.hpp"
#include <chrono>
#include <gflags/gflags.h>
#include <memory>
#include <thread>


DEFINE_bool(run_mode, false, "程序运行模式, false-调试 true-发布;");
DEFINE_string(log_file, "", "发布模式下, 用于指定日志的输出文件;");
DEFINE_int32(log_level, 0, "发布模式下, 用于指定日志输出等级");

DEFINE_string(etcd_host, "http://127.0.0.1:2379", "服务注册中心地址");
DEFINE_string(base_service, "/service", "服务器监控目录");
DEFINE_string(instance_name, "/user/instance", "当前实例名称");
DEFINE_string(access_host, "127.0.0.1:8081", "当前实例的外部访问地址");

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    Registry::ptr rclient = std::make_shared<Registry>(FLAGS_etcd_host);
    rclient->registry(FLAGS_base_service + FLAGS_instance_name, FLAGS_access_host);

    std::this_thread::sleep_for(std::chrono::seconds(600));
    return 0;
}