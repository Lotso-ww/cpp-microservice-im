//实现用户管理子服务
#include <boost/date_time/posix_time/conversion.hpp>
#include <brpc/controller.h>
#include <brpc/server.h>
#include <butil/logging.h>
#include <brpc/closure_guard.h>
#include <cstdint>
#include <cstdlib>
#include <elasticlient/client.h>
#include <functional>
#include <linux/falloc.h>
#include <memory>
#include <odb/forward.hxx>
#include <string>
#include <vector>
#include "channel.hpp"    // 信道管理模块封装
#include "dms.hpp"        // 短信平台模块封装
#include "etcd.hpp"       // 服务注册模块封装
#include "data_es.hpp"    // es 数据管理客户端封装
#include "message.hxx"
#include "mysql_factory.hpp"    // mysql 连接工厂封装
#include "mysql_message.hpp"    // mysql 消息表管理封装
#include "rabbitmq.hpp"   // mq 消息队列客户端封装
#include "data_redis.hpp"    // redis 数据管理客户端封装
#include "logger.hpp"     // 日志模块封装
#include "utils.hpp"      // 基础工具接口
#include "base.pb.h"    // protobuf 框架代码
#include "message.pb.h"    // protobuf 框架代码
#include "file.pb.h"    // protobuf 框架代码

namespace lotso_im {
// 创建子类，继承于 MessageService 创建一个子类，并实现 rpc 调用的业务功能
class MessageServiceImpl : public lotso_im::MsgStorageService
{
public:
    MessageServiceImpl(const std::shared_ptr<elasticlient::Client> &es_client, const std::shared_ptr<odb::core::database> &mysql_client, 
                    const ServiceManager::ptr &mm_channels, const std::string &file_service_name)
                    : _es_user(std::make_shared<ESMessage>(es_client))
                    , _mysql_message(std::make_shared<MessageTable>(mysql_client))
                    , _mm_channels(mm_channels)
                    , _file_service_name(file_service_name)
    {}
   ~MessageServiceImpl(){}
    virtual void GetHistoryMsg(::google::protobuf::RpcController* controller,
                       const ::lotso_im::GetHistoryMsgReq* request,
                       ::lotso_im::GetHistoryMsgRsp* response,
                       ::google::protobuf::Closure* done) {
        brpc::ClosureGuard rpc_guard(done);
        // 0. 定义一个错误处理函数, 当出错的时候进行调用 -- 这里在函数内部实现是因为每个接口的错误返回格式设置是不一样的
        auto err_response = [this, response](const std::string &rid, const std::string &errmsg){
            response->set_request_id(rid);
            response->set_success(false);
            response->set_errmsg(errmsg);
            return;
        };
    }
  virtual void GetRecentMsg(::google::protobuf::RpcController* controller,
                       const ::lotso_im::GetRecentMsgReq* request,
                       ::lotso_im::GetRecentMsgRsp* response,
                    ::google::protobuf::Closure* done) {
        brpc::ClosureGuard rpc_guard(done);
        // 0. 定义一个错误处理函数, 当出错的时候进行调用 -- 这里在函数内部实现是因为每个接口的错误返回格式设置是不一样的
        auto err_response = [this, response](const std::string &rid, const std::string &errmsg){
            response->set_request_id(rid);
            response->set_success(false);
            response->set_errmsg(errmsg);
            return;
        };
    }
  virtual void MsgSearch(::google::protobuf::RpcController* controller,
                       const ::lotso_im::MsgSearchReq* request,
                       ::lotso_im::MsgSearchRsp* response,
                       ::google::protobuf::Closure* done) {
        brpc::ClosureGuard rpc_guard(done);
        // 0. 定义一个错误处理函数, 当出错的时候进行调用 -- 这里在函数内部实现是因为每个接口的错误返回格式设置是不一样的
        auto err_response = [this, response](const std::string &rid, const std::string &errmsg){
            response->set_request_id(rid);
            response->set_success(false);
            response->set_errmsg(errmsg);
            return;
        };
    }
public:
    // MQ 消息消费回调, 由 MessageServerBuilder 在订阅队列时绑定, 因此必须是 public
    void onMessage(const char *body, size_t sz)
    {
        // 1. 取出序列化的消息内容，进行反序列化
        lotso_im::MessageInfo message;
        bool ret = message.ParseFromArray(body, sz);
        if(ret == false) {
            LOG_ERROR("对消费的数据进行反序列化失败! ");
        }
        // 2. 根据不同的消息类型进行不同的处理
        std::string file_id, file_name, content;
        int64_t file_size;
        switch (message.message().message_type()) {
            // 如果是一个文本类型消息，取元信息存储到 ES
            case MessageType::STRING:
            {
                const auto &msg = message.message().string_message();
                content = msg.content();
                ret = _es_user->appendData(message.sender().user_id(),message.message_id(), message.timestamp(), message.chat_session_id(), content);
                if(ret == false) {
                    LOG_ERROR("文本消息向存储引擎进行存储失败! ");
                    return;
                }
            }
            break;
            // 如果是一个图片/语音/文件消息，则取出数据存储到文件子服务中，并获取文件ID
            case MessageType::IMAGE:
            {
                const auto &msg = message.message().image_message();
                ret = _PutFile("", msg.image_content(), msg.image_content().size(), file_id);
                if(ret == false) {
                    LOG_ERROR("上传图片到文件子服务失败! ");
                    return;
                }
                break;
            }
            case MessageType::FILE:
            {
                const auto &msg = message.message().file_message();
                file_name = msg.file_name();
                file_size = msg.file_size();
                ret = _PutFile(file_name, msg.file_contents(), file_size, file_id);
                if(ret == false) {
                    LOG_ERROR("上传文件到文件子服务失败! ");
                    return;
                }
                break;
            }
            case MessageType::SPEECH:
            {
                const auto &msg = message.message().speech_message();
                ret = _PutFile("", msg.file_contents(), msg.file_contents().size(), file_id);
                if(ret == false) {
                    LOG_ERROR("上传语音到文件子服务失败! ");
                    return;
                }
                break;
            }
            default:
                LOG_ERROR("消息类型错误! ");
                return;
        }
        // 3. 提取消息的元信息，存储到mysql数据库中
        lotso_im::Message msg(message.message_id(), message.chat_session_id(), message.sender().user_id(), message.message().message_type(), boost::posix_time::from_time_t(message.timestamp()));
        msg.content(content);
        msg.file_id(file_id);
        msg.file_size(file_size);
        ret = _mysql_message->insert(msg);
        if(ret == false) {
            LOG_ERROR("向数据库插入数据失败! ");
            return;
        }
    }
private:
    bool _PutFile(const std::string &filename, const std::string &body, const int64_t fsize, std::string &file_id)
    {
        // 实现文件的上传
        auto channel = _mm_channels->choose(_file_service_name);
        if(!channel) {
            LOG_ERROR("{} 没有可以访问的文件子服务节点! ", _file_service_name);
            return false;
        }
        FileService_Stub stub(channel.get());
        PutSingleFileReq req;
        PutSingleFileRsp rsp;
        req.mutable_file_data()->set_file_name(filename);
        req.mutable_file_data()->set_file_size(fsize);
        req.mutable_file_data()->set_file_content(body);
        brpc::Controller cntl;
        stub.PutSingleFile(&cntl, &req, &rsp, nullptr);
        if(cntl.Failed() == true || rsp.success() == false) {
            LOG_ERROR("文件子服务调用失败 - {}", cntl.ErrorText());
            return false;
        }
        file_id = rsp.file_info().file_id();
        return true;
    }
private:
    ESMessage::ptr _es_user;
    MessageTable::ptr _mysql_message;
    // 这边是 rpc 调用客户端相关对象
    std::string _file_service_name;
    ServiceManager::ptr _mm_channels;
};

class MessageServer
{
public:
    using ptr = std::shared_ptr<MessageServer>;
    MessageServer(const Discovery::ptr &service_discover, const Registry::ptr &reg_client, const std::shared_ptr<elasticlient::Client> &es_client, 
               const std::shared_ptr<odb::core::database> &mysql_client, const MQClient::ptr &mq_client, const std::shared_ptr<brpc::Server> &server)
        : _service_discover(service_discover)
        , _registry_client(reg_client)
        , _es_client(es_client)
        , _mysql_client(mysql_client)
        , _mq_client(mq_client)
        , _rpc_server(server)
    {}
    ~MessageServer(){}
    // 搭建 RPC 服务器, 并启动服务器
    void start()
    {
        _rpc_server->RunUntilAskedToQuit(); // 休眠等待运行结束
    }
private:
    Discovery::ptr _service_discover;
    Registry::ptr _registry_client;
    std::shared_ptr<elasticlient::Client> _es_client;
    MQClient::ptr _mq_client;
    std::shared_ptr<odb::core::database> _mysql_client;
    std::shared_ptr<brpc::Server> _rpc_server;
};

// 使用了建造者模式, 把构造 MessageServer 的过程封装起来了
class MessageServerBuilder
{
public: 
    // 构成mq管理对象
    void make_mq_object(const std::string &user, std::string &pswd, std::string &host
                     , const std::string &exchange_name, const std::string &queue_name, const std::string &binding_key)
    {
        _exchange_name = exchange_name;
        _queue_name = queue_name;
        _mq_client = std::make_shared<MQClient>(user, pswd, host);
        _mq_client->declareComponents(exchange_name, queue_name, binding_key);
    }
    // 构造es客户端对象
    void make_es_object(const std::vector<std::string> &host_list)
    {
        _es_client = ESClientFactory::create(host_list);
    }
    // 构造短信验证客户端对象
    void make_dms_object()
    {
        _dms_client = std::make_shared<DMSClient>(); // 自己内部会去读环境变量, 我设置好了, 如果没环境变量的话可能得自己配置一下
    }
    // 构造mysql管理对象
    void make_mysql_object(const std::string &user, std::string &pswd, std::string &host, std::string &db, std::string &cset, int port, int conn_pool_count)
    {
        _mysql_client = ODBFactory::create(user, pswd, host, db, cset, port, conn_pool_count);
    }
    // 构成redis管理对象
    void make_redis_object(const std::string &host, int port, int db, int conn_pool_count)
    {
        _redis_client = RedisClientFactory::create(host, port, db, conn_pool_count);
    }
    // 构造服务发现客户端对象
    void make_discovery_object(const std::string &reg_host, const std::string &base_service_name, const std::string &file_service_name)
    {
        _file_service_name = file_service_name;
        _mm_channels = std::make_shared<ServiceManager>();
        _mm_channels->declared(file_service_name);
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
            LOG_ERROR("还未初始化mq模块! ");
            abort();
        }
        if(!_es_client)
        {
            LOG_ERROR("还未初始化ES搜索引擎模块! ");
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
        MessageServiceImpl *message_service = new MessageServiceImpl(_es_client, _mysql_client, _mm_channels, _file_service_name);
        int ret = _rpc_server->AddService(message_service, brpc::ServiceOwnership::SERVER_OWNS_SERVICE);  // brpc::ServiceOwnership::SERVER_OWNS_SERVICE -- 添加服务失败时, 服务器将负责删除服务对象
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

        auto callback = std::bind(&MessageServiceImpl::onMessage, message_service, std::placeholders::_1, std::placeholders::_2);
        _mq_client->consume(_queue_name, callback);
    }
    MessageServer::ptr build()
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
        MessageServer::ptr server = std::make_shared<MessageServer>(_service_discover, _registry_client, _es_client, _mysql_client, _mq_client, _rpc_server);
        return server;
    }
private:
    Registry::ptr _registry_client;

    std::shared_ptr<elasticlient::Client> _es_client;
    std::shared_ptr<odb::core::database> _mysql_client;
    std::shared_ptr<sw::redis::Redis> _redis_client;

    std::string _file_service_name;
    ServiceManager::ptr _mm_channels;
    Discovery::ptr _service_discover;
    DMSClient::ptr _dms_client;

    std::string _exchange_name;
    std::string _queue_name;
    MQClient::ptr _mq_client;

    std::shared_ptr<brpc::Server> _rpc_server;
};
}
