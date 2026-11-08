//实现用户管理子服务
#include <brpc/controller.h>
#include <brpc/server.h>
#include <butil/logging.h>
#include <brpc/closure_guard.h>
#include <cstdlib>
#include <elasticlient/client.h>
#include <linux/falloc.h>
#include <memory>
#include <odb/forward.hxx>
#include <string>
#include <sw/redis++/redis.h>
#include <vector>
#include "channel.hpp"    // 信道管理模块封装
#include "etcd.hpp"       // 服务注册模块封装
#include "rabbitmq.hpp"   // mq 消息队列客户端封装
#include "data_mysql.hpp"    // mysql 数据管理客户端封装
#include "logger.hpp"     // 日志模块封装
#include "utils.hpp"      // 基础工具接口
#include "user.hxx"
#include "user-odb.hxx"
#include "base.pb.h"    // protobuf 框架代码
#include "user.pb.h"    // protobuf 框架代码
#include "transmite.pb.h"    // protobuf 框架代码

namespace lotso_im {
// 创建子类，继承于 TransmiteService 创建一个子类，并实现 rpc 调用的业务功能
class TransmiteServiceImpl : public lotso_im::MsgTransmitService
{
public:
    TransmiteServiceImpl(const std::string &user_service_name, const ServiceManager::ptr &channels, const std::shared_ptr<odb::core::database> &mysql_client
                        ,const MQClient::ptr &mq_client)
                    : _user_service_name(user_service_name)
                    , _mm_channels(channels)
                    , _mysql_session_member_table(std::make_shared<ChatSessionMemberTable>(mysql_client))
                    , _mq_client(mq_client)
    {}
   ~TransmiteServiceImpl(){}
    virtual void GetTransmitTarget(::google::protobuf::RpcController* controller,
                        const ::lotso_im::NewMessageReq* request,
                        ::lotso_im::NewMessageRsp* response,
                    ::google::protobuf::Closure* done){
        brpc::ClosureGuard rpc_guard(done);
    }
private:
    // 用户子服务调用相关信息
    std::string _user_service_name;
    ServiceManager::ptr _mm_channels;

    // 聊天会话成员表的操作句柄
    ChatSessionMemberTable::ptr _mysql_session_member_table;

    // 消息队列客户端句柄
    MQClient::ptr _mq_client;
};

class TransmiteServer
{
public:
    using ptr = std::shared_ptr<TransmiteServer>;
            TransmiteServer(
            const std::shared_ptr<odb::core::database> &mysql_client,
            const Discovery::ptr &discovery_client,
            const Registry::ptr &registry_client,
            const std::shared_ptr<brpc::Server> &server):
            _service_discover(discovery_client),
            _registry_client(registry_client),
            _mysql_client(mysql_client),
            _rpc_server(server){}
    ~TransmiteServer(){}
    // 搭建 RPC 服务器, 并启动服务器
    void start()
    {
        _rpc_server->RunUntilAskedToQuit(); // 休眠等待运行结束
    }
private:
    Discovery::ptr _service_discover;  // 服务发现客户端
    Registry::ptr _registry_client;    // 服务注册客户端
    std::shared_ptr<odb::core::database> _mysql_client;  // mysql 数据库客户端
    std::shared_ptr<brpc::Server> _rpc_server;
};

// 使用了建造者模式, 把构造 TransmiteServer 的过程封装起来了
class TransmiteServerBuilder
{
public: 
    // 构成mq管理对象
    void make_mq_object(const std::string &user, std::string &pswd, std::string &host)
    {
        _mq_client = std::make_shared<MQClient>(user, pswd, host);
    }
    // 构造mysql管理对象
    void make_mysql_object(const std::string &user, std::string &pswd, std::string &host, std::string &db, std::string &cset, int port, int conn_pool_count)
    {
        _mysql_client = ODBFactory::create(user, pswd, host, db, cset, port, conn_pool_count);
    }
    // 构造服务发现客户端对象
    void make_discovery_object(const std::string &reg_host, const std::string &base_service_name, const std::string &user_service_name)
    {
        _user_service_name = user_service_name;
        _mm_channels = std::make_shared<ServiceManager>();
        _mm_channels->declared(user_service_name);
        auto put_cb = std::bind(&ServiceManager::onServiceOnline, _mm_channels.get(), std::placeholders::_1, std::placeholders::_2);
        auto del_cb = std::bind(&ServiceManager::onServiceOffline, _mm_channels.get(), std::placeholders::_1, std::placeholders::_2);
        _service_discover = std::make_shared<Discovery>(reg_host, base_service_name, put_cb, del_cb);
    }
    // 构造服务注册客户端对象
    void make_registry_object(const std::string &reg_host, const std::string &service_name, const std::string &access_host)
    {
        _registry_client = std::make_shared<Registry>(reg_host);
        _registry_client->registry(service_name, access_host);
    }
    // 构造 RPC 服务器对象
    void make_rpc_object(uint16_t port, int32_t timeout, uint8_t num_threads)
    {
        if(!_mq_client)
        {
            LOG_ERROR("还未初始化mq消息队列模块! ");
            abort();
        }
        if(!_mysql_client)
        {
            LOG_ERROR("还未初始化MySQL数据库模块! ");
            abort();
        }
        if(!_mm_channels)
        {
            LOG_ERROR("还未初始化信道管理模块! ");
            abort();
        }
        _rpc_server = std::make_shared<brpc::Server>();
        TransmiteServiceImpl *transmite_service = new TransmiteServiceImpl(_user_service_name, _mm_channels, _mysql_client, _mq_client);
        int ret = _rpc_server->AddService(transmite_service, brpc::ServiceOwnership::SERVER_OWNS_SERVICE);  // brpc::ServiceOwnership::SERVER_OWNS_SERVICE -- 添加服务失败时, 服务器将负责删除服务对象
        if(ret == -1)
        {
            LOG_ERROR("添加 RPC 服务失败! ");
            abort();
        }
        brpc::ServerOptions options;
        options.idle_timeout_sec = timeout; // 连接空闲超时时间 -- 超时后连接被关闭
        options.num_threads = num_threads; // io 线程数量
        ret = _rpc_server->Start(port, &options);
        if(ret == -1)
        {
            LOG_ERROR("服务器启动失败! ");
            abort();
        }
    }
    TransmiteServer::ptr build()
    {
        // 对上面的几个操作进行检验
        if(!_service_discover)
        {
            LOG_ERROR("还未初始化服务发现模块! ");
            abort();
        }
        if(!_registry_client)
        {
            LOG_ERROR("还未初始化服务注册模块! ");
            abort();
        }
        if(!_rpc_server)
        {
            LOG_ERROR("还未初始化RPC服务器模块! ");
            abort();
        }
        TransmiteServer::ptr server = std::make_shared<TransmiteServer>(_mysql_client, _service_discover, _registry_client, _rpc_server);
        return server;
    }
private:
    Registry::ptr _registry_client;
    std::shared_ptr<odb::core::database> _mysql_client;
    std::string _user_service_name;
    ServiceManager::ptr _mm_channels;
    Discovery::ptr _service_discover;
    MQClient::ptr _mq_client;
    std::shared_ptr<brpc::Server> _rpc_server;
};
}