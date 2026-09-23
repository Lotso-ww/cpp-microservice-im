#include <chrono>
#include <iostream>
#include <iterator>
#include <sw/redis++/connection.h>
#include <sw/redis++/redis.h>
#include <gflags/gflags.h>
#include <thread>
#include <vector>

DEFINE_string(ip, "127.0.0.1", "这是服务的IP地址, 格式: 127.0.0.1");
DEFINE_int32(port, 6379, "这是服务的监听端口, 格式: 8080");
DEFINE_int32(db, 0, "库的编号: 默认0号");
DEFINE_bool(keep_alive, true, "是否进行长连接保活");

void print(sw::redis::Redis &client)
{
    auto user1 = client.get("会话ID1");
    if(user1) std::cout << *user1 << std::endl;
    auto user2 = client.get("会话ID2");
    if(user2) std::cout << *user2 << std::endl;
    auto user3 = client.get("会话ID3");
    if(user3) std::cout << *user3 << std::endl;
    auto user4 = client.get("会话ID4");
    if(user4) std::cout << *user4 << std::endl;
    auto user5 = client.get("会话ID5");
    if(user5) std::cout << *user5 << std::endl;
}
void add_string(sw::redis::Redis &client)
{
    // 新增
    client.set("会话ID1", "用户ID1");
    client.set("会话ID2", "用户ID2");
    client.set("会话ID3", "用户ID3");
    client.set("会话ID4", "用户ID4");
    client.set("会话ID5", "用户ID5");

    // 更新
    client.set("会话ID5", "用户ID2222");
    // 删除
    client.del("会话ID3");

    print(client);
}

void test_expired(sw::redis::Redis &client)
{
    // 这次的 set, 数据其实已经有了, 因此本次是修改
    // 不仅仅修改 val, 我们再给键值设置一下过期时间看看
    client.set("会话ID1", "用户ID111111", std::chrono::milliseconds(1000));

    print(client);
    std::cout << "--------休眠2s--------" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    print(client);
}

void test_list(sw::redis::Redis &client)
{
    // 1 2 3 4 5 -- rpush
    // 5 4 3 2 1 -- lpush
    client.rpush("群聊1", "用户1");
    client.rpush("群聊1", "用户2");
    client.rpush("群聊1", "用户3");
    client.rpush("群聊1", "用户4");
    client.rpush("群聊1", "用户5");

    std::vector<std::string> users;
    client.lrange("群聊1", 0, -1, std::back_inserter(users));

    for(auto user : users)
    {
        std::cout << user << std::endl;
    }
}

int main(int argc, char *argv[])
{
    // 功能接口演示中：
    // 1. 构造连接选项，实例化 redis 对象，连接服务器
    sw::redis::ConnectionOptions opts;
    opts.host = FLAGS_ip;
    opts.port = FLAGS_port;
    opts.db = FLAGS_db;
    opts.keep_alive = FLAGS_keep_alive;
    sw::redis::Redis client(opts);
    // 2. 添加字符串键值对，删除字符串键值对，获取字符串键值对
    std::cout << "添加字符串键值对，删除字符串键值对，获取字符串键值对测试" << std::endl;
    add_string(client);
    std::cout << "---------------------------------------------------" << std::endl;
    // 3. 实践控制数据有效时间的操作
    std::cout << "实践控制数据有效时间的操作" << std::endl;
    test_expired(client);
    std::cout << "---------------------------------------------------" << std::endl;
    // 4. 列表的操作，主要实现数据的右插，左获取
    std::cout << "列表的操作，主要实现数据的右插，左获取" << std::endl;
    test_list(client);
    std::cout << "---------------------------------------------------" << std::endl;
    return 0;
}