// 主要实现语音识别子服务的服务器的搭建
#include "speech_server.hpp"
#include <gflags/gflags.h>

DEFINE_bool(run_mode, false, "程序运行模式, false-调试 true-发布;");
DEFINE_string(log_file, "", "发布模式下, 用于指定日志的输出文件;");
DEFINE_int32(log_level, 0, "发布模式下, 用于指定日志输出等级");

DEFINE_string(registry_host, "http://127.0.0.1:2379", "服务注册中心地址");
DEFINE_string(base_service, "/service", "服务器监控目录");
DEFINE_string(instance_name, "/speech_service/instance", "当前实例名称");
DEFINE_string(access_host, "127.0.0.1:10001", "当前实例的外部访问地址");

DEFINE_int32(listen_port, 10001, "Rpc 服务器监听端口");
DEFINE_int32(rpc_timeout, -1, "RPC 调用超时时间");
DEFINE_int32(rpc_threads, 1, "RPC 的IO线程数量");

DEFINE_string(app_id, "8012068", "语音识别平台应用 ID");
DEFINE_string(api_key, "pz31dgyMWksAIgFtlgY6mG1A", "语音识别平台应用 API Key");
DEFINE_string(secret_key, "fSna8Sbtc3jBYKPUSTfK6IMSAHJwX2ma", "语音识别平台应用加密密钥");

int main(int argc, char *argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    lotso_im::init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);
    // 关闭默认的日志输出
    // logging::LoggingSettings settings;
    // settings.logging_dest = logging::LoggingDestination::LOG_TO_NONE;
    // logging::InitLogging(settings);

    lotso_im::SpeechServerBuilder ssb;
    ssb.make_asr_object(FLAGS_app_id, FLAGS_api_key, FLAGS_secret_key);
    ssb.make_reg_object(FLAGS_registry_host, FLAGS_base_service + FLAGS_instance_name, FLAGS_access_host);
    ssb.make_rpc_object(FLAGS_listen_port, FLAGS_rpc_timeout, FLAGS_rpc_threads);

    auto server = ssb.build();
    server->start();
    return 0;
}
