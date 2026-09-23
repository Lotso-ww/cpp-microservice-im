#include <brpc/closure_guard.h>
#include <brpc/server.h>
#include <butil/logging.h>
#include <google/protobuf/service.h>
#include "main.pb.h"

// 1. 创建子类，继承于 EchoService 创建一个子类，并实现 rpc 调用的业务功能
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
    // 2. 构造服务器对象，关闭 brpc 的默认日志输出
    brpc::Server server;
    logging::LoggingSettings settings;
    settings.logging_dest = logging::LoggingDestination::LOG_TO_NONE;
    logging::InitLogging(settings);

    // 3. 向服务器对象中, 新增 EchoService 服务
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
    ret = server.Start(8080, &options);
    if(ret == -1)
    {
        std::cout << "启动服务器失败!" << std::endl;
        return -1;
    }
    server.RunUntilAskedToQuit(); // 修改等待运行结束
    return 0;
}
