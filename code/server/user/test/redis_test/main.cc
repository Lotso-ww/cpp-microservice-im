#include "../../../common/data_redis.hpp"
#include <chrono>
#include <gflags/gflags.h>
#include <memory>
#include <sw/redis++/redis.h>
#include <thread>

DEFINE_string(ip, "127.0.0.1", "这是服务的IP地址, 格式: 127.0.0.1");
DEFINE_int32(port, 6379, "这是服务的监听端口, 格式: 8080");
DEFINE_int32(db, 0, "库的编号: 默认0号");
DEFINE_bool(keep_alive, true, "是否进行长连接保活");

void session_test(std::shared_ptr<sw::redis::Redis> &client)
{
    lotso_im::Session ss(client);
    ss.append("会话ID1", "用户ID1");
    ss.append("会话ID2", "用户ID2");
    ss.append("会话ID3", "用户ID3");
    ss.append("会话ID4", "用户ID4");

    ss.remove("会话ID2");
    ss.remove("会话ID3");

    auto res1 = ss.uid("会话ID1");
    if(res1) std::cout << *res1 << std::endl;
    auto res2 = ss.uid("会话ID2");
    if(res2) std::cout << *res2 << std::endl;
    auto res3 = ss.uid("会话ID3");
    if(res3) std::cout << *res3 << std::endl;
    auto res4 = ss.uid("会话ID4");
    if(res4) std::cout << *res4 << std::endl;
}

void status_test(std::shared_ptr<sw::redis::Redis> &client)
{
    lotso_im::Status st(client);

    st.append("用户ID1");
    st.append("用户ID2");
    st.append("用户ID3");

    st.remove("用户ID2");

    if(st.exists("用户ID1")) std::cout << "用户1在线!" << std::endl;
    if(st.exists("用户ID2")) std::cout << "用户2在线!" << std::endl;
    if(st.exists("用户ID3")) std::cout << "用户3在线!" << std::endl;
}

void code_test(std::shared_ptr<sw::redis::Redis> &client)
{
    lotso_im::Codes cd(client);

    // 测试的时候我们给的默认过期时间是 3s -- 正常业务场景是 60s
    cd.append("验证码ID1", "验证码1");
    cd.append("验证码ID2", "验证码2");
    cd.append("验证码ID3", "验证码3");

    cd.remove("验证码ID2");

    auto y1 = cd.code("验证码ID1");
    auto y2 = cd.code("验证码ID2");
    auto y3 = cd.code("验证码ID3");
    if(y1) std::cout << *y1 << std::endl;
    if(y2) std::cout << *y2 << std::endl;
    if(y3) std::cout << *y3 << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(4));
    auto y4 = cd.code("验证码ID1");
    auto y5 = cd.code("验证码ID2");
    auto y6 = cd.code("验证码ID3");
    if(y4) std::cout << *y4 << std::endl;
    if(y5) std::cout << *y5 << std::endl;
    if(y6) std::cout << *y6 << std::endl;
}
int main(int argc, char *argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    auto client = lotso_im::RedisClientFactory::create(FLAGS_ip, FLAGS_port, FLAGS_db, FLAGS_keep_alive);

    // session_test(client);
    // status_test(client);
    code_test(client);
    return 0;
}