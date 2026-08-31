#include <brpc/channel.h>
#include <brpc/controller.h>
#include <chrono>
#include <google/protobuf/stubs/callback.h>
#include <iostream>
#include <memory>
#include <thread>
#include "main.pb.h"

void callback(brpc::Controller* controller, ::example::EchoResponse* response)
{
    // 用智能指针管理起来, 防止忘记释放
    std::unique_ptr<brpc::Controller> cntl_guard(controller);
    std::unique_ptr<example::EchoResponse> rsp_guard(response);
    if(controller->Failed() == true)
    {
        std::cout << "Rpc 调用失败: " << controller->ErrorText() << std::endl;
        return;
    }
    std::cout << "收到响应: " << response->message() << std::endl;
}

int main(int argc, char* argv[])
{
    // 1. 构造一个 Channel 信道，连接服务器
    brpc::ChannelOptions options;
    options.connect_timeout_ms = -1; // 连接等待超时时间, -1 表示一直等待
    options.timeout_ms = -1; // rpc 请求等待超时时间, -1 表示一直等待
    options.max_retry = 3; // 请求重试次数
    options.protocol = "baidu_std"; // 序列化协议, 默认使用 baidu_std 
    brpc::Channel channel;
    int ret = channel.Init("127.0.0.1:8080", &options);
    if(ret == -1)
    {
        std::cout << "初始化信道失败!" << std::endl;
        return -1;
    }

    // 2. 构造 EchoService_Stub 对象, 用于进行 rpc 调用
    example::EchoService_Stub stub(&channel);

    // 3. 进行 Rpc 调用
    example::EchoRequest req;
    req.set_message("你好, Lotso!");

    // ******************** 同步调用 *********************************//
    // brpc::Controller *cntl = new brpc::Controller;
    // example::EchoResponse *rsp = new example::EchoResponse();
    // stub.Echo(cntl, &req, rsp, nullptr); // 同步
    // if(cntl->Failed() == true)
    // {
    //     std::cout << "Rpc 调用失败: " << cntl->ErrorText() << std::endl;
    //     return -1;
    // }
    // std::cout << "收到响应: " << rsp->message() << std::endl;
    // delete cntl;
    // delete rsp;
    // **************************************************************//

    // ******************** 异步调用 *********************************//
    brpc::Controller *cntl = new brpc::Controller;
    example::EchoResponse *rsp = new example::EchoResponse();
    auto clusure = google::protobuf::NewCallback(callback, cntl, rsp);
    stub.Echo(cntl, &req, rsp, clusure); 
    std::cout << "异步调用结束" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    // **************************************************************//
    return 0;
}