// 实现文件存储子服务
// 1. 实现文件 rpc 服务类 -- 实现 rpc 调用的业务处理接口
// 2. 实现文件存储子服务的服务器类
// 3. 实现文件存储子服务类的建造者

#include <brpc/server.h>
#include <butil/logging.h>
#include "etcd.hpp"      // 服务注册模块封装
#include "logger.hpp"    // 日志模块封装
#include "base.pb.h"     // protobuf 框架代码
#include "file.pb.h"

namespace lotso_im {
// 创建子类，继承于 FileService 创建一个子类，并实现 rpc 调用的业务功能
class FileServiceImpl : public lotso_im::FileService
{
public:
    FileServiceImpl(){};
   ~FileServiceImpl(){};
    void GetSingleFile(google::protobuf::RpcController* controller,
                    const ::lotso_im::GetSingleFileReq* request,
                    ::lotso_im::GetSingleFileRsp* response,
                    ::google::protobuf::Closure* done){
        brpc::ClosureGuard rpc_guard(done);
    }
    void GetMultiFile(google::protobuf::RpcController* controller,
                    const ::lotso_im::GetMultiFileReq* request,
                    ::lotso_im::GetMultiFileRsp* response,
                    ::google::protobuf::Closure* done){
        brpc::ClosureGuard rpc_guard(done);
    }
    void PutSingleFile(google::protobuf::RpcController* controller,
                    const ::lotso_im::PutSingleFileReq* request,
                    ::lotso_im::PutSingleFileRsp* response,
                    ::google::protobuf::Closure* done){
        brpc::ClosureGuard rpc_guard(done);
    }
    void PutMultiFile(google::protobuf::RpcController* controller,
                    const ::lotso_im::PutMultiFileReq* request,
                    ::lotso_im::PutMultiFileRsp* response,
                    ::google::protobuf::Closure* done){
        brpc::ClosureGuard rpc_guard(done);
    }
private:
};

class FileServer
{
public:
    using ptr = std::shared_ptr<FileServer>;
    FileServer(const Registry::ptr &reg_client, const std::shared_ptr<brpc::Server> &server)
    {}
    ~FileServer(){}
    // 搭建 RPC 服务器, 并启动服务器
    void start()
    {
        _rpc_server->RunUntilAskedToQuit(); // 休眠等待运行结束
    }
private:
    Registry::ptr _reg_client;
    std::shared_ptr<brpc::Server> _rpc_server;
};

// 使用了建造者模式, 把构造 FileServer 的过程封装起来了
class FileServerBuilder
{
public: 
    // 构造服务注册客户端对象
    void make_reg_object(const std::string &reg_host, const std::string &service_name, const std::string &access_host)
    {
        _reg_client = std::make_shared<Registry>(reg_host);
        _reg_client->registry(service_name, access_host);
    }
    // 构造 RPC 服务器对象
    void make_rpc_object(uint16_t port, int32_t timeout, uint8_t num_threads)
    {
        _rpc_server = std::make_shared<brpc::Server>();
        FileServiceImpl *file_service = new FileServiceImpl();
        int ret = _rpc_server->AddService(file_service, brpc::ServiceOwnership::SERVER_OWNS_SERVICE);  // brpc::ServiceOwnership::SERVER_OWNS_SERVICE -- 添加服务失败时, 服务器将负责删除服务对象
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
    FileServer::ptr build()
    {
        // 对上面的几个操作进行检验
        if(!_reg_client)
        {
            LOG_ERROR("还未初始化服务注册模块! ");
            abort();
        }
        if(!_rpc_server)
        {
            LOG_ERROR("还未初始化RPC服务器模块! ");
            abort();
        }
        FileServer::ptr server = std::make_shared<FileServer>(_reg_client, _rpc_server);
        return server;
    }
private:
    Registry::ptr _reg_client;
    std::shared_ptr<brpc::Server> _rpc_server;
};
}