#include "transmite_server.hpp"
#include <gflags/gflags.h>

// 日志的配置
DEFINE_bool(run_mode, false, "程序运行模式, false-调试 true-发布;");
DEFINE_string(log_file, "", "发布模式下, 用于指定日志的输出文件;");
DEFINE_int32(log_level, 0, "发布模式下, 用于指定日志输出等级");
// 服务注册的配置
DEFINE_string(registry_host, "http://127.0.0.1:2379", "服务注册中心地址");
DEFINE_string(instance_name, "/transmite_service/instance", "当前实例名称");
DEFINE_string(access_host, "127.0.0.1:10004", "当前实例的外部访问地址");
// Rpc的配置
DEFINE_int32(listen_port, 10004, "Rpc 服务器监听端口");
DEFINE_int32(rpc_timeout, -1, "RPC 调用超时时间");
DEFINE_int32(rpc_threads, 1, "RPC 的IO线程数量");
// 服务发现的配置
DEFINE_string(base_service, "/service", "服务器监控目录");
DEFINE_string(file_service, "/service/user_service", "用户管理子服务名称");
// mysql的配置
DEFINE_string(mysql_user, "imuser", "mysql服务器访问用户名");
DEFINE_string(mysql_pwd, "imuser_666", "mysql服务器访问密码");
DEFINE_string(mysql_host, "127.0.0.1", "mysql服务器访问地址");
DEFINE_string(mysql_db, "my_im", "mysql默认库名称");
DEFINE_string(mysql_cset, "utf8", "mysql客户端字符集");
DEFINE_int32(mysql_port, 0, "mysql服务器访问端口");
DEFINE_int32(mysql_pool_count, 4, "mysql连接池最大连接数量");
// mq的配置
DEFINE_string(mq_user, "lotso", "mq服务器访问用户名");
DEFINE_string(mq_pwd, "123456", "mq服务器访问密码");
DEFINE_string(mq_host, "127.0.0.1:5672", "mq服务器访问地址");

int main(int argc, char *argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    lotso_im::init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);
    // 关闭默认的日志输出
    // logging::LoggingSettings settings;
    // settings.logging_dest = logging::LoggingDestination::LOG_TO_NONE;
    // logging::InitLogging(settings);

    lotso_im::TransmiteServerBuilder tsb;
    tsb.make_mq_object(FLAGS_mq_user, FLAGS_mq_pwd, FLAGS_mq_host);
    tsb.make_mysql_object(FLAGS_mysql_user, FLAGS_mysql_pwd, FLAGS_mysql_host, FLAGS_mysql_db, FLAGS_mysql_cset, FLAGS_mysql_port, FLAGS_mysql_pool_count);
    tsb.make_discovery_object(FLAGS_registry_host, FLAGS_base_service, FLAGS_file_service);
    // 先 rpc调用再服务注册
    tsb.make_rpc_object(FLAGS_listen_port, FLAGS_rpc_timeout, FLAGS_rpc_threads);
    tsb.make_registry_object(FLAGS_registry_host, FLAGS_base_service + FLAGS_instance_name, FLAGS_access_host);
    
    auto server = tsb.build();
    server->start();
    return 0;
}
