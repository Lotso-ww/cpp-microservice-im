#include <cstdlib>
#include <iostream>
#include <string>
#include <ev.h>
#include <amqpcpp.h>
#include <amqpcpp/libev.h>
#include <functional>
#include <openssl/ssl.h>
#include <openssl/opensslv.h>

void MessageCb(AMQP::TcpChannel *channel, const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
{
    std::string msg;
    msg.assign(message.body(), message.bodySize());
    // 注意：不能直接 cout << message.body()，消息体不保证以 '\0' 结尾，
    // 直接当 C 字符串输出会多读越界字节（之前日志里的乱码就是这个原因）
    std::cout << msg << std::endl;
    channel->ack(deliveryTag);
}

int main()
{
    // 1. 实例化底层网络通信框架的 I/O 事件监控句柄
    auto *loop = EV_DEFAULT;
    // 2. 实例化 libEvHandler 句柄 -- 将 AMQP 框架与事件监控关联起来
    AMQP::LibEvHandler handler(loop);
    // 3. 实例化连接对象
    AMQP::Address address("amqp://lotso:123456@127.0.0.1:5672/");
    AMQP::TcpConnection connection(&handler, address);
    // 4. 实例化信道对象
    AMQP::TcpChannel channel(&connection);
    // 5. 声明交换机
    channel.declareExchange("test-exchange", AMQP::ExchangeType::direct)
        .onError([](const char *message)
        {
            std::cout << "声明交换机失败: " << message << std::endl;
            exit(0);
        })
        .onSuccess([]()
        {
            std::cout << "test-exchange 交换机创建成功! " << std::endl;
        });
    // 6. 声明队列
    channel.declareQueue("test-queue")
        .onError([](const char *message)
        {
            std::cout << "声明队列失败: " << message << std::endl;
            exit(0);
        })
        .onSuccess([]()
        {
            std::cout << "test-queue 队列创建成功! " << std::endl;
        });
    // 7. 针对交换机和队列进行绑定
    channel.bindQueue("test-exchange", "test-queue", "test-queue-key")
        .onError([](const char *message)
        {
            std::cout << "test-exchange - test-queue 绑定失败: " << message << std::endl;
            exit(0);
        })
        .onSuccess([]()
        {
            std::cout << "test-exchange - test-queue 绑定成功! " << std::endl;
        });
    // 8. 订阅队列消息 -- 设置消息处理回调函数
    auto callback = std::bind(MessageCb, &channel, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    channel.consume("test-queue", "consume-tag")
        .onReceived(callback)
        .onError([](const char *message){
            std::cout << "订阅消息失败: " << message << std::endl;
            exit(0);
        });
    // 9. 启动底层网络通信框架 -- 开启 I/O
    ev_run(loop, 0);
}