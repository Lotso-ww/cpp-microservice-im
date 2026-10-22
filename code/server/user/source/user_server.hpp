//实现用户管理子服务
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
#include "asr.hpp"        // 语音识别模块封装
#include "channel.hpp"
#include "dms.hpp"
#include "etcd.hpp"       // 服务注册模块封装
#include "data_es.hpp"    // es 数据管理客户端封装
#include "data_mysql.hpp"    // mysql 数据管理客户端封装
#include "data_redis.hpp"    // redis 数据管理客户端封装
#include "logger.hpp"     // 日志模块封装
#include "utils.hpp"      // 基础工具接口
#include "user.hxx"
#include "user-odb.hxx"
#include "base.pb.h"    // protobuf 框架代码
#include "user.pb.h"    // protobuf 框架代码
#include "file.pb.h"    // protobuf 框架代码

namespace lotso_im {
// 创建子类，继承于 UserService 创建一个子类，并实现 rpc 调用的业务功能
class UserServiceImpl : public lotso_im::UserService
{
public:
    UserServiceImpl(const std::shared_ptr<elasticlient::Client> &es_client, const std::shared_ptr<odb::core::database> &mysql_client, const std::shared_ptr<sw::redis::Redis> &redis_client, 
                    const ServiceManager::ptr &mm_channels, const std::string &file_service_name, const DMSClient::ptr dms_client)
                    : _es_user(std::make_shared<ESUser>(es_client))
                    , _mysql_user(std::make_shared<UserTable>(mysql_client))
                    , _redis_session(std::make_shared<Session>(redis_client))
                    , _redis_status(std::make_shared<Status>(redis_client))
                    , _redis_codes(std::make_shared<Codes>(redis_client))
                    , _mm_channels(mm_channels)
                    , _file_service_name(file_service_name)
                    , _dms_client(dms_client)
    {}
   ~UserServiceImpl(){}
    virtual void UserRegister(::google::protobuf::RpcController* controller,
                        const ::lotso_im::UserRegisterReq* request,
                        ::lotso_im::UserRegisterRsp* response,
                    ::google::protobuf::Closure* done){
        brpc::ClosureGuard rpc_guard(done);
        // 0. 定义一个错误处理函数, 当出错的时候进行调用 -- 这里在函数内部实现是因为每个接口的错误返回格式设置是不一样的
        auto err_response = [this, response](const std::string &rid, const std::string &errmsg){
            response->set_request_id(rid);
            response->set_success(false);
            response->set_errmsg(errmsg);
            return;
        };
        // 1. 从请求中取出昵称和密码
        std::string nickname = request->nickname();
        std::string password = request->password();
        // 2. 检查昵称是否合法(不超过22位即可)
        bool ret = nickname_check(nickname);
        if(ret == false) {
            LOG_ERROR("{} - 用户名长度不合法! ", request->request_id());
            return err_response(request->request_id(), "用户名长度不合法! ");
        }
        // 3. 检查密码是否合法(只能包含字母, 数字, 长度限制 6 - 15 之间, 还可以用 '-' 和 '_')
        ret = password_check(password);
        if(ret == false) {
            LOG_ERROR("{} - 密码格式不合法! ", request->request_id());
            return err_response(request->request_id(), "密码格式不合法! ");
        }
        // 4. 根据昵称在数据库进行判断是否已存在
        auto user = _mysql_user->select_by_nickname(nickname);
        if(user) {
            LOG_ERROR("{} - 用户名被占用 - {}! ", request->request_id(), nickname);
            return err_response(request->request_id(), "用户名被占用! ");
        }
        // 5. 向数据库新增数据
        std::string uid = uuid();
        user = std::make_shared<User>(uid, nickname, password);
        ret = _mysql_user->insert(user);
        if(user) {
            LOG_ERROR("{} - MySQL数据库新增数据失败! ", request->request_id());
            return err_response(request->request_id(), "MySQL数据库新增数据失败! ");
        }
        // 6. 向 ES 服务器中新增用户信息
        ret = _es_user->appendData(uid, nickname, "", "", "");
        if(user) {
            LOG_ERROR("{} - ES搜索引擎新增数据失败! ", request->request_id());
            return err_response(request->request_id(), "ES搜索引擎新增数据失败! ");
        }
        // 7. 组织响应, 进行成功与否的响应即可
        response->set_request_id(request->request_id());
        response->set_success(true);
    }
                        
    virtual void UserLogin(::google::protobuf::RpcController* controller,
                        const ::lotso_im::UserLoginReq* request,
                        ::lotso_im::UserLoginRsp* response,
                        ::google::protobuf::Closure* done){
        brpc::ClosureGuard rpc_guard(done);
        // 0. 定义一个错误处理函数, 当出错的时候进行调用 -- 这里在函数内部实现是因为每个接口的错误返回格式设置是不一样的
        auto err_response = [this, response](const std::string &rid, const std::string &errmsg){
            response->set_request_id(rid);
            response->set_success(false);
            response->set_errmsg(errmsg);
            return;
        };
        // 1. 从请求中取出昵称和密码
        std::string nickname = request->nickname();
        std::string password = request->password();
        // 2. 通过昵称获取用户信息, 进行密码是否一致的判断
        auto user = _mysql_user->select_by_nickname(nickname);
        if(!user || password != *user->password()) {
            LOG_ERROR("{} - 用户名或者密码错误 - {}-{}", request->request_id(), nickname, password);
            return err_response(request->request_id(), "用户名或密码错误! ");
        }
        // 3. 根据 redis 中的登录标记信息是否存在判断用户是否已经登录
        bool ret = _redis_status->exists(user->user_id());
        if(ret == true) {
            LOG_ERROR("{} - 用户已在其他地方登录 - {}! ", request->request_id(), nickname);
            return err_response(request->request_id(), "用户已在其他地方登录! ");
        }
        // 4. 构造会话ID, 生成会话键值对, 向 redis 中添加会话信息以及登录标记信息
        std::string ssid = uuid();
        _redis_session->append(ssid, user->user_id());
        _redis_status->append(user->user_id());
        // 5. 组织响应, 返回生成的会话ID
        response->set_request_id(request->request_id());
        response->set_success(true);
    }
    virtual void GetPhoneVerifyCode(::google::protobuf::RpcController* controller,
                        const ::lotso_im::PhoneVerifyCodeReq* request,
                        ::lotso_im::PhoneVerifyCodeRsp* response,
                        ::google::protobuf::Closure* done){
        brpc::ClosureGuard rpc_guard(done);
        // 0. 定义一个错误处理函数, 当出错的时候进行调用 -- 这里在函数内部实现是因为每个接口的错误返回格式设置是不一样的
        auto err_response = [this, response](const std::string &rid, const std::string &errmsg){
            response->set_request_id(rid);
            response->set_success(false);
            response->set_errmsg(errmsg);
            return;
        };
        // 1. 从请求中取出手机号
        std::string phone = request->phone_number();
        // 2. 验证手机号码格式是否正确(必须是 1 开始, 第二位 3-9 之间, 后面 9 个数字字符)
        bool ret = phone_check(phone);
        if(ret == false) {
            LOG_ERROR("{} - 手机号码格式错误 - {}! ", request->request_id(), phone);
            return err_response(request->request_id(), "手机号码格式错误! ");
        }
        // 3. 基于短信平台 SDK 发送验证码 -- 这个可以直接传手机号, 平台那边自动生成, 但是要随机生成一个 code_id
        std::string code_id = uuid();
        std::string code = _dms_client->send(phone);
        if(code == "") {
            LOG_ERROR("{} - 短信验证码发送失败 - {}! ", request->request_id(), phone);
            return err_response(request->request_id(), "短信验证码发送失败! ");
        }
        // 4. 构造验证码 ID, 添加到 redis 验证码映射键值索引中
        _redis_codes->append(code_id, code);
        // 5. 组织响应, 返回生成的验证码 ID
        response->set_request_id(request->request_id());
        response->set_success(true);
        response->set_verify_code_id(code_id);
    }
    virtual void PhoneRegister(::google::protobuf::RpcController* controller,
                        const ::lotso_im::PhoneRegisterReq* request,
                        ::lotso_im::PhoneRegisterRsp* response,
                        ::google::protobuf::Closure* done){
        brpc::ClosureGuard rpc_guard(done);
        // 0. 定义一个错误处理函数, 当出错的时候进行调用 -- 这里在函数内部实现是因为每个接口的错误返回格式设置是不一样的
        auto err_response = [this, response](const std::string &rid, const std::string &errmsg){
            response->set_request_id(rid);
            response->set_success(false);
            response->set_errmsg(errmsg);
            return;
        };
        // 1. 从请求中取出手机号, 验证码ID, 验证码
        std::string phone = request->phone_number();
        std::string code_id = request->verify_code_id();
        std::string code = request->verify_code();
        // 2. 检查注册手机号码是否合法
        bool ret = phone_check(phone);
        if(ret == false) {
            LOG_ERROR("{} - 手机号码格式错误 - {}! ", request->request_id(), phone);
            return err_response(request->request_id(), "手机号码格式错误! ");
        }
        // 3. 从 redis 中进行 验证码ID - 验证码 一致性验证
        auto vcode = _redis_codes->code(code_id);
        if(vcode != code) {
            LOG_ERROR("{} - 验证码错误 - {}-{}! ", request->request_id(), code_id, code);
            return err_response(request->request_id(), "验证码错误! ");
        }
        // 4. 通过数据库查询判断手机号是否已经注册过
        auto user = _mysql_user->select_by_phone(phone);
        if(user) {
            LOG_ERROR("{} - 该手机号已注册过用户 - {}! ", request->request_id(), phone);
            return err_response(request->request_id(), "该手机号已注册过用户! ");
        }
        // 5. 向数据库新增用户信息
        std::string uid = uuid();
        user = std::make_shared<User>(uid, phone);
        ret = _mysql_user->insert(user);
        if(user) {
            LOG_ERROR("{} - 向数据库添加用户信息失败 - {}! ", request->request_id(), phone);
            return err_response(request->request_id(), "向数据库添加用户信息失败! ");
        }
        // 6. 向 ES 服务器中新增用户信息
        ret = _es_user->appendData(uid, uid, phone, "", "");
        if(user) {
            LOG_ERROR("{} - ES搜索引擎新增数据失败! ", request->request_id());
            return err_response(request->request_id(), "ES搜索引擎新增数据失败! ");
        }
        // 7. 组织响应, 进行成功与否的响应即可
        response->set_request_id(request->request_id());
        response->set_success(true);
    }
    virtual void PhoneLogin(::google::protobuf::RpcController* controller,
                        const ::lotso_im::PhoneLoginReq* request,
                        ::lotso_im::PhoneLoginRsp* response,
                        ::google::protobuf::Closure* done){
        brpc::ClosureGuard rpc_guard(done);
    }
    virtual void GetUserInfo(::google::protobuf::RpcController* controller,
                        const ::lotso_im::GetUserInfoReq* request,
                        ::lotso_im::GetUserInfoRsp* response,
                        ::google::protobuf::Closure* done){
        brpc::ClosureGuard rpc_guard(done);
    }
    virtual void GetMultiUserInfo(::google::protobuf::RpcController* controller,
                        const ::lotso_im::GetMultiUserInfoReq* request,
                        ::lotso_im::GetMultiUserInfoRsp* response,
                        ::google::protobuf::Closure* done){
        brpc::ClosureGuard rpc_guard(done);
    }
    virtual void SetUserAvatar(::google::protobuf::RpcController* controller,
                        const ::lotso_im::SetUserAvatarReq* request,
                        ::lotso_im::SetUserAvatarRsp* response,
                        ::google::protobuf::Closure* done){
        brpc::ClosureGuard rpc_guard(done);
    }
    virtual void SetUserNickname(::google::protobuf::RpcController* controller,
                        const ::lotso_im::SetUserNicknameReq* request,
                        ::lotso_im::SetUserNicknameRsp* response,
                        ::google::protobuf::Closure* done){
        brpc::ClosureGuard rpc_guard(done);
    }
    virtual void SetUserDescription(::google::protobuf::RpcController* controller,
                        const ::lotso_im::SetUserDescriptionReq* request,
                        ::lotso_im::SetUserDescriptionRsp* response,
                        ::google::protobuf::Closure* done){
        brpc::ClosureGuard rpc_guard(done);
    }
    virtual void SetUserPhoneNumber(::google::protobuf::RpcController* controller,
                        const ::lotso_im::SetUserPhoneNumberReq* request,
                        ::lotso_im::SetUserPhoneNumberRsp* response,
                        ::google::protobuf::Closure* done){
        brpc::ClosureGuard rpc_guard(done);
    }
private:
    bool nickname_check(const std::string &nickname) {
        return nickname.size() < 22;
    }
    bool password_check(const std::string &password) {
        if(password.size() < 6 || password.size() > 15) {
            LOG_ERROR("密码长度不合法: {}-{}", password, password.size());
            return false;
        }
        for(int i = 0; i < password.size(); i++)
        {
            if(!((password[i] > 'a' && password[i] < 'z') ||
                (password[i] > 'A' && password[i] < 'Z') ||
                (password[i] > '0' && password[i] < '9') ||
                (password[i] == '_' && password[i] < '-')
            )) {
                LOG_ERROR("密码字符不合法: {}", password);
                return false;
            }
        }
        return true;
    }
    bool phone_check(const std::string &phone) {
        if(phone.size() < 1) return false;
        if(phone[0] != '1') return false;
        if(phone[1] < '3' || phone[1] > '9') return false;
        for(int i = 2; i < 11; i++) {
            if(phone[i] < '0' || phone[i] > '9') return false;
        }
        return true;
    }
private:
    ESUser::ptr _es_user;
    UserTable::ptr _mysql_user;
    Session::ptr _redis_session;
    Status::ptr _redis_status;
    Codes::ptr _redis_codes;
    // 这边是 rpc 调用客户端相关对象
    std::string _file_service_name;
    ServiceManager::ptr _mm_channels;
    DMSClient::ptr _dms_client;
};

class UserServer
{
public:
    using ptr = std::shared_ptr<UserServer>;
    UserServer(const Discovery::ptr &service_discover, const Registry::ptr &reg_client, const std::shared_ptr<elasticlient::Client> &es_client, 
               const std::shared_ptr<odb::core::database> &mysql_client, const std::shared_ptr<sw::redis::Redis> &redis_client, const std::shared_ptr<brpc::Server> &server)
        : _service_discover(service_discover)
        , _registry_client(reg_client)
        , _es_client(es_client)
        , _mysql_client(mysql_client)
        , _redis_client(redis_client)
        , _rpc_server(server)
    {}
    ~UserServer(){}
    // 搭建 RPC 服务器, 并启动服务器
    void start()
    {
        _rpc_server->RunUntilAskedToQuit(); // 休眠等待运行结束
    }
private:
    Discovery::ptr _service_discover;
    Registry::ptr _registry_client;
    std::shared_ptr<elasticlient::Client> _es_client;
    std::shared_ptr<odb::core::database> _mysql_client;
    std::shared_ptr<sw::redis::Redis> _redis_client;
    std::shared_ptr<brpc::Server> _rpc_server;
};

// 使用了建造者模式, 把构造 UserServer 的过程封装起来了
class UserServerBuilder
{
public: 
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
        if(!_redis_client)
        {
            LOG_ERROR("还未初始化Redis数据库模块! ");
            abort();
        }
        if(!_mm_channels)
        {
            LOG_ERROR("还未初始化信道管理模块! ");
            abort();
        }
        if(!_dms_client)
        {
            LOG_ERROR("还未初始化短信平台模块! ");
            abort();
        }
        _rpc_server = std::make_shared<brpc::Server>();
        UserServiceImpl *user_service = new UserServiceImpl(_es_client, _mysql_client, _redis_client, _mm_channels, _file_service_name, _dms_client);
        int ret = _rpc_server->AddService(user_service, brpc::ServiceOwnership::SERVER_OWNS_SERVICE);  // brpc::ServiceOwnership::SERVER_OWNS_SERVICE -- 添加服务失败时, 服务器将负责删除服务对象
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
    UserServer::ptr build()
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
        UserServer::ptr server = std::make_shared<UserServer>(_service_discover, _registry_client, _es_client, _mysql_client, _redis_client, _rpc_server);
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

    std::shared_ptr<brpc::Server> _rpc_server;
};
}
