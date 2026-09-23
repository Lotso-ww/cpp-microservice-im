#include <iostream>
#include <functional>
#include <websocketpp/common/connection_hdl.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/connection.hpp>
#include <websocketpp/frame.hpp>
#include <websocketpp/logger/levels.hpp>
#include <websocketpp/server.hpp>

// 1. 定义 server 类型
typedef websocketpp::server<websocketpp::config::asio> server_t;

// 回调函数 -- 这里注意 hdl 不要使用引用, msg 也不要
void onOpen(websocketpp::connection_hdl hdl)
{
    std::cout << "websocket长连接建立成功!" << std::endl;
}
void onClose(websocketpp::connection_hdl hdl)
{
    std::cout << "websocket长连接断开!" << std::endl;
}
void onMessage(server_t *server, websocketpp::connection_hdl hdl, server_t::message_ptr msg)
{
    //  获取有效信息载荷数据
    std::string body = msg->get_payload();
    std::cout << "收到消息: " << body << std::endl;
    // 对客户端进行响应, 获取通信连接并发送数据
    auto conn = server->get_con_from_hdl(hdl);
    conn->send(body + "-Hello!", websocketpp::frame::opcode::value::text);
}

int main(int argc, char *argv[])
{
    // 2. 实例化服务器对象
    server_t server;
    // 3. 初始化日志输出 -- 关闭日志输出
    server.set_access_channels(websocketpp::log::alevel::none);
    // 4. 初始化 asio 框架
    server.init_asio();
    // 5. 设置消息处理/连接握手成功/连接关闭回调函数
    server.set_open_handler(onOpen);
    server.set_close_handler(onClose);
    auto msg_handler = std::bind(onMessage, &server, std::placeholders::_1, std::placeholders::_2);
    server.set_message_handler(msg_handler);
    // 6. 启用地址重用
    server.set_reuse_addr(true);
    // 7. 设置监听端口
    server.listen(9090);
    // 8. 开始监听
    server.start_accept();
    // 9. 启动服务器
    server.run();
    return 0;
}