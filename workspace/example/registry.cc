#include "../common/etcd.hpp"
#include "../brpc_back/main.pb.h"
#include <brpc/server.h>
#include <butil/logging.h>
#include <chrono>
#include <gflags/gflags.h>
#include <memory>
#include <thread>


DEFINE_bool(run_mode, false, "程序运行模式, false-调试 true-发布;");
DEFINE_string(log_file, "", "发布模式下, 用于指定日志的输出文件;");
DEFINE_int32(log_level, 0, "发布模式下, 用于指定日志输出等级");

DEFINE_string(etcd_host, "http://127.0.0.1:2379", "服务注册中心地址");
DEFINE_string(base_service, "/service", "服务器监控目录");
DEFINE_string(instance_name, "/echo/instance", "当前实例名称");
DEFINE_string(access_host, "127.0.0.1:7070", "当前实例的外部访问地址");
DEFINE_int32(listen_port, 7070, "Rpc 服务器监听端口");

// 创建子类，继承于 EchoService 创建一个子类，并实现 rpc 调用的业务功能
class EchoServiceImpl : public example::EchoService
{
public:
    EchoServiceImpl(){};
    ~EchoServiceImpl(){};
    void Echo(google::protobuf::RpcController* controller, 
                    const ::example::EchoRequest* request, 
                    ::example::EchoResponse* response, 
                    ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard rpc_guard(done); // 智能管理对象
        std::cout << "收到消息: " << request->message() << std::endl;

        std::string str = request->message() + "--这个是响应!!";
        response->set_message(str);
        // done->Run(); 这里不需要了是因为前面使用了智能管理对象，析构的时候会自动调用 Run
    }
};

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);
    // ****************** 服务端改造 *********************
    // 1. 构造 Echo 服务
    // 2. 搭建 Rpc 服务器
    // 3. 运行 Rpc 服务
    // 4. 注册服务
    // ***************************************************
    brpc::Server server;
    logging::LoggingSettings settings;
    settings.logging_dest = logging::LoggingDestination::LOG_TO_NONE;
    logging::InitLogging(settings);

    // 向服务器对象中, 新增 EchoService 服务
    // brpc::ServiceOwnership::SERVER_DOESNT_OWN_SERVICE -- 添加服务失败时, 服务器不会删除服务对象
    EchoServiceImpl echo_service;
    int ret = server.AddService(&echo_service, brpc::ServiceOwnership::SERVER_DOESNT_OWN_SERVICE);
    if(ret == -1)
    {
        std::cout << "添加 rpc 服务失败!" << std::endl;
        return -1;
    }

    // 4. 可以进行一些参数配置，并且启动服务器
    brpc::ServerOptions options;
    options.idle_timeout_sec = -1; // 连接空闲超时时间 -- 超时后连接被关闭
    options.num_threads = 1; // io 线程数量
    ret = server.Start(FLAGS_listen_port, &options);
    if(ret == -1)
    {
        std::cout << "启动服务器失败!" << std::endl;
        return -1;
    }
    // 注册服务
    Registry::ptr rclient = std::make_shared<Registry>(FLAGS_etcd_host);
    rclient->registry(FLAGS_base_service + FLAGS_instance_name, FLAGS_access_host);

    server.RunUntilAskedToQuit(); // 休眠等待运行结束
    return 0;
}