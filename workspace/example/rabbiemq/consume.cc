#include "../../common/rabbitmq.hpp"
#include <chrono>
#include <gflags/gflags.h>
#include <thread>

DEFINE_string(user, "lotso", "rabbitmq访问用户名");
DEFINE_string(passwd, "123456", "rabbitmq访问密码");
DEFINE_string(host, "127.0.0.1:5672", "rabbirmq服务器地址信息 host::port");

DEFINE_bool(run_mode, false, "程序运行模式, false-调试 true-发布;");
DEFINE_string(log_file, "", "发布模式下, 用于指定日志的输出文件;");
DEFINE_int32(log_level, 0, "发布模式下, 用于指定日志输出等级");


void callback(const char *body, size_t sz)
{
    // 必须从消息体指针 body 拷贝 sz 个字节;
    // 若误写成 msg.assign(msg, sz), 会命中 assign(const string&, pos) 子串重载,
    // 对空串按位置 sz 取子串而抛 out_of_range
    std::string msg(body, sz);
    std::cout << msg << std::endl;
}
int main(int argc, char *argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    MQClient client(FLAGS_user, FLAGS_passwd, FLAGS_host);
    client.declareComponents("test-exchange", "test-queue");
    
    client.consume("test-queue", callback);

    std::this_thread::sleep_for(std::chrono::seconds(60));
    return 0;
}