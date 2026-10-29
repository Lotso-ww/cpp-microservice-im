//实现用户管理子服务
#include <brpc/controller.h>
#include <brpc/server.h>
#include <butil/logging.h>
#include <brpc/closure_guard.h>
#include <cstddef>
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
        response->set_login_session_id(ssid);
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
        // 0. 定义一个错误处理函数, 当出错的时候进行调用 -- 这里在函数内部实现是因为每个接口的错误返回格式设置是不一样的
        auto err_response = [this, response](const std::string &rid, const std::string &errmsg){
            response->set_request_id(rid);
            response->set_success(false);
            response->set_errmsg(errmsg);
            return;
        };
        // 1. 从请求中取出手机号码和验证码ID以及验证码
        std::string phone = request->phone_number();
        std::string code_id = request->verify_code_id();
        std::string code = request->verify_code();
        // 2. 检查注册手机号码是否合法
        bool ret = phone_check(phone);
        if(ret == false) {
            LOG_ERROR("{} - 手机号码格式错误 - {}! ", request->request_id(), phone);
            return err_response(request->request_id(), "手机号码格式错误! ");
        }
        // 3. 根据手机号从数据库进行用户信息查询，判断用户是否存在
        auto user = _mysql_user->select_by_phone(phone);
        if(user) {
            LOG_ERROR("{} - 该手机号未注册过用户 - {}! ", request->request_id(), phone);
            return err_response(request->request_id(), "该手机号未注册过用户! ");
        }
        // 4. 从 redis 数据库中进行验证码ID -- 验证码一致性匹配
        auto vcode = _redis_codes->code(code_id);
        if(vcode != code) {
            LOG_ERROR("{} - 验证码错误 - {}-{}! ", request->request_id(), code_id, code);
            return err_response(request->request_id(), "验证码错误! ");
        }
        _redis_codes->remove(code_id);
        // 5. 根据 redis 中的登录标记信息是否存在判断用户是否已经登录
        ret = _redis_status->exists(user->user_id());
        if(ret == true) {
            LOG_ERROR("{} - 用户已在其他地方登录 - {}!", request->request_id(), phone);
            return err_response(request->request_id(), "用户已在其他地方登录! ");
        }
        // 6. 构造会话 ID, 生成会话键值对, 向 redis 中添加会话信息以及登录标记信息
        std::string ssid = uuid();
        _redis_session->append(ssid, user->user_id());
        // 7. 组织响应, 返回生成的会话 ID
        response->set_request_id(request->request_id());
        response->set_login_session_id(ssid);
        response->set_success(true);
    }
    virtual void GetUserInfo(::google::protobuf::RpcController* controller,
                        const ::lotso_im::GetUserInfoReq* request,
                        ::lotso_im::GetUserInfoRsp* response,
                        ::google::protobuf::Closure* done){
        brpc::ClosureGuard rpc_guard(done);
        // 0. 定义一个错误处理函数, 当出错的时候进行调用 -- 这里在函数内部实现是因为每个接口的错误返回格式设置是不一样的
        auto err_response = [this, response](const std::string &rid, const std::string &errmsg){
            response->set_request_id(rid);
            response->set_success(false);
            response->set_errmsg(errmsg);
            return;
        };
        // 1. 从请求中取出用户 ID
        std::string uid = request->user_id();
        // 2. 通过用户 ID, 从数据库中查询用户信息
        auto user = _mysql_user->select_by_id(uid);
        if(!user) {
            LOG_ERROR("{} - 未找到用户信息 - {}! ", request->request_id(), uid);
            return err_response(request->request_id(), "未找到用户信息! ");
        }
        // 3. 根据用户信息中的头像 ID, 从文件服务器获取头像文件数据, 组织完整用户信息
        UserInfo *user_info = response->mutable_user_info();
        user_info->set_user_id(user->user_id());
        if(user->nickname()) user_info->set_nickname(*user->nickname());
        if(user->description()) user_info->set_description(*user->description());
        if(user->phone()) user_info->set_phone(*user->phone());
        if(user->avatar_id()) {
            // 从信道管理对象中获取到连接了文件管理自服务的 channel
            auto channel = _mm_channels->choose(_file_service_name);
            if(!channel) {
                LOG_ERROR("{} - 未找到文件管理子服务节点 - {}! ", request->request_id(), uid);
                return err_response(request->request_id(), "未找到文件管理子服务节点! ");
            }
            // 进行文件子服务的 rpc 请求, 进行头像文件下载
            lotso_im::FileService_Stub stub(channel.get());
            lotso_im::GetSingleFileReq req;
            lotso_im::GetSingleFileRsp rsp;
            req.set_request_id(request->request_id());
            req.set_file_id(*user->avatar_id());
            brpc::Controller cntl;
            stub.GetSingleFile(&cntl, &req, &rsp, nullptr);
            if(cntl.Failed() == true || rsp.success() == false) {
                LOG_ERROR("{} - 文件子服务调用失败 - {}", request->request_id(), cntl.ErrorText());
                return err_response(request->request_id(), "文件子服务调用失败!");
            }
            user_info->set_avatar(rsp.file_data().file_content());
        }
        // 4. 组织响应, 返回生成的会话 ID
        response->set_request_id(request->request_id());
        response->set_success(true);
    }
    virtual void GetMultiUserInfo(::google::protobuf::RpcController* controller,
                        const ::lotso_im::GetMultiUserInfoReq* request,
                        ::lotso_im::GetMultiUserInfoRsp* response,
                        ::google::protobuf::Closure* done){
        brpc::ClosureGuard rpc_guard(done);
        // 0. 定义一个错误处理函数, 当出错的时候进行调用 -- 这里在函数内部实现是因为每个接口的错误返回格式设置是不一样的
        auto err_response = [this, response](const std::string &rid, const std::string &errmsg){
            response->set_request_id(rid);
            response->set_success(false);
            response->set_errmsg(errmsg);
            return;
        };
        // 1. 从请求中取出用户ID -- 列表
        std::vector<std::string> uid_lists;
        for(int i = 0; i < request->users_id_size(); i++) {
            uid_lists.push_back(request->users_id(i));
        }
        // 2. 从数据库进行批量用户信息查询
        auto users = _mysql_user->select_multi_users(uid_lists);
        if(users.size() != request->users_id_size()) {
            LOG_ERROR("{} - 从数据库中查找出来的用户信息数量不一致 {}-{}! ", request->request_id(), request->users_id_size(), users.size());
            return err_response(request->request_id(), "从数据库中查找出来的用户信息数量不一致!");
        }
        // 3. 批量从文件管理子服务进行文件下载
        auto channel = _mm_channels->choose(_file_service_name);
        if(!channel) {
            LOG_ERROR("{} - 未找到文件管理子服务节点!", request->request_id());
            return err_response(request->request_id(), "未找到文件管理子服务节点! ");
        }
        lotso_im::FileService_Stub stub(channel.get());
        lotso_im::GetMultiFileReq req;
        lotso_im::GetMultiFileRsp rsp;
        req.set_request_id(request->request_id());
        for(auto &user : users) {
            req.add_file_id_list(*user.avatar_id());
        }
        brpc::Controller cntl;
        stub.GetMultiFile(&cntl, &req, &rsp, nullptr);
        if(cntl.Failed() == true || rsp.success() == false) {
            LOG_ERROR("{} - 文件子服务调用失败 - {}", request->request_id(), cntl.ErrorText());
            return err_response(request->request_id(), "文件子服务调用失败!");
        }
        // 4. 组织响应
        for(auto &user : users) {
            auto user_map = response->mutable_users_info(); // 本次请求要响应的用户信息 map
            auto file_map = rsp.mutable_file_data(); // 这是批量文件请求响应中的 map
            UserInfo user_info;
            user_info.set_user_id(user.user_id());
            if(user.nickname()) user_info.set_nickname(*user.nickname());
            if(user.description()) user_info.set_description(*user.description());
            if(user.phone()) user_info.set_phone(*user.phone());
            if(user.avatar_id()) user_info.set_avatar((*file_map)[user.avatar_id()].file_content());
            (*user_map)[user_info.user_id()] = user_info;
        }
        response->set_request_id(request->request_id());
        response->set_success(true);
    }
    virtual void SetUserAvatar(::google::protobuf::RpcController* controller,
                        const ::lotso_im::SetUserAvatarReq* request,
                        ::lotso_im::SetUserAvatarRsp* response,
                        ::google::protobuf::Closure* done){
        brpc::ClosureGuard rpc_guard(done);
        // 0. 定义一个错误处理函数, 当出错的时候进行调用 -- 这里在函数内部实现是因为每个接口的错误返回格式设置是不一样的
        auto err_response = [this, response](const std::string &rid, const std::string &errmsg){
            response->set_request_id(rid);
            response->set_success(false);
            response->set_errmsg(errmsg);
            return;
        };
        // 1. 从请求中取出用户ID和头像数据 -- 头像数据在后续使用的时候取
        std::string uid = request->user_id();
        // 2. 从数据库通过用户ID进行用户信息查询, 判断用户是否存在
        auto user = _mysql_user->select_by_id(uid);
        if(!user) {
            LOG_ERROR("{} - 未找到用户信息 - {}! ", request->request_id(), uid);
            return err_response(request->request_id(), "未找到用户信息! ");
        }
        // 3. 上传头像文件到文件子服务
        auto channel = _mm_channels->choose(_file_service_name);
        if(!channel) {
            LOG_ERROR("{} - 未找到文件管理子服务节点!", request->request_id());
            return err_response(request->request_id(), "未找到文件管理子服务节点! ");
        }
        lotso_im::FileService_Stub stub(channel.get());
        lotso_im::PutSingleFileReq req;
        lotso_im::PutSingleFileRsp rsp;
        req.set_request_id(request->request_id());
        req.mutable_file_data()->set_file_name("");
        req.mutable_file_data()->set_file_size(request->avatar().size());
        req.mutable_file_data()->set_file_content(request->avatar());
        brpc::Controller cntl;
        stub.PutSingleFile(&cntl, &req, &rsp, nullptr);
        if(cntl.Failed() == true || rsp.success() == false) {
            LOG_ERROR("{} - 文件子服务调用失败 - {}", request->request_id(), cntl.ErrorText());
            return err_response(request->request_id(), "文件子服务调用失败!");
        }
        std::string avatar_id = rsp.file_info().file_id();
        // 4. 将返回的头像文件ID更新到数据库中
        user->avatar_id(avatar_id);
        bool ret = _mysql_user->update(user);
        if(ret == false) {
            LOG_ERROR("{} - 更新数据库用户头像ID失败: {}! ", request->request_id(), avatar_id);
            return err_response(request->request_id(), "更新数据库用户头像ID失败!");
        }
        // 5. 更新 ES 服务器中用户信息
        ret = _es_user->appendData(user->user_id(), *user->nickname(), *user->phone(), *user->description(),*user->avatar_id());
        if(ret == false) {
            LOG_ERROR("{} - 更新搜索引擎用户头像ID失败: {}! ", request->request_id(), avatar_id);
            return err_response(request->request_id(), "更新搜索引擎用户头像ID失败!");
        }
        // 6. 组织响应, 返回更新成功与否
        response->set_request_id(request->request_id());
        response->set_success(true);    
    }
    virtual void SetUserNickname(::google::protobuf::RpcController* controller,
                        const ::lotso_im::SetUserNicknameReq* request,
                        ::lotso_im::SetUserNicknameRsp* response,
                        ::google::protobuf::Closure* done){
        brpc::ClosureGuard rpc_guard(done);
        // 0. 定义一个错误处理函数, 当出错的时候进行调用 -- 这里在函数内部实现是因为每个接口的错误返回格式设置是不一样的
        auto err_response = [this, response](const std::string &rid, const std::string &errmsg){
            response->set_request_id(rid);
            response->set_success(false);
            response->set_errmsg(errmsg);
            return;
        };
        // 1. 从请求中取出用户ID和新的昵称
        std::string uid = request->user_id();
        std::string new_nickname = request->nickname();
        // 2. 判断昵称格式是否正确
        bool ret = nickname_check(new_nickname);
        if(ret == false) {
            LOG_ERROR("{} - 用户名长度不合法! ", request->request_id());
            return err_response(request->request_id(), "用户名长度不合法! ");
        }
        // 3. 从数据库通过用户ID进行用户信息查询, 判断用户是否存在
        auto user = _mysql_user->select_by_id(uid);
        if(!user) {
            LOG_ERROR("{} - 未找到用户信息 - {}! ", request->request_id(), uid);
            return err_response(request->request_id(), "未找到用户信息! ");
        }
        // 4. 将新的昵称更新到数据库中
        user->nickname(new_nickname);
        ret = _mysql_user->update(user);
        if(ret == false) {
            LOG_ERROR("{} - 更新数据库用户昵称失败: {}! ", request->request_id(), new_nickname);
            return err_response(request->request_id(), "更新数据库用户昵称失败!");
        }
        // 5. 更新 ES 服务器中用户信息
        ret = _es_user->appendData(user->user_id(), *user->nickname(), *user->phone(), *user->description(),*user->avatar_id());
        if(ret == false) {
            LOG_ERROR("{} - 更新搜索引擎用户昵称失败: {}! ", request->request_id(), new_nickname);
            return err_response(request->request_id(), "更新搜索引擎用户昵称失败!");
        }
        // 6. 组织响应, 返回更新成功与否
        response->set_request_id(request->request_id());
        response->set_success(true);    
    }
    virtual void SetUserDescription(::google::protobuf::RpcController* controller,
                        const ::lotso_im::SetUserDescriptionReq* request,
                        ::lotso_im::SetUserDescriptionRsp* response,
                        ::google::protobuf::Closure* done){
        brpc::ClosureGuard rpc_guard(done);
        // 0. 定义一个错误处理函数, 当出错的时候进行调用 -- 这里在函数内部实现是因为每个接口的错误返回格式设置是不一样的
        auto err_response = [this, response](const std::string &rid, const std::string &errmsg){
            response->set_request_id(rid);
            response->set_success(false);
            response->set_errmsg(errmsg);
            return;
        };
        // 1. 从请求中取出用户ID和新的签名
        std::string uid = request->user_id();
        std::string new_description = request->description();
        // 2. 从数据库通过用户ID进行用户信息查询, 判断用户是否存在
        auto user = _mysql_user->select_by_id(uid);
        if(!user) {
            LOG_ERROR("{} - 未找到用户信息 - {}! ", request->request_id(), uid);
            return err_response(request->request_id(), "未找到用户信息! ");
        }
        // 3. 将新的签名更新到数据库中
        user->description(new_description);
        bool ret = _mysql_user->update(user);
        if(ret == false) {
            LOG_ERROR("{} - 更新数据库用户签名失败: {}! ", request->request_id(), new_description);
            return err_response(request->request_id(), "更新数据库用户签名失败!");
        }
        // 5. 更新 ES 服务器中用户信息
        ret = _es_user->appendData(user->user_id(), *user->nickname(), *user->phone(), *user->description(),*user->avatar_id());
        if(ret == false) {
            LOG_ERROR("{} - 更新搜索引擎用户签名失败: {}! ", request->request_id(), new_description);
            return err_response(request->request_id(), "更新搜索引擎用户签名失败!");
        }
        // 6. 组织响应, 返回更新成功与否
        response->set_request_id(request->request_id());
        response->set_success(true);    
    }
    virtual void SetUserPhoneNumber(::google::protobuf::RpcController* controller,
                        const ::lotso_im::SetUserPhoneNumberReq* request,
                        ::lotso_im::SetUserPhoneNumberRsp* response,
                        ::google::protobuf::Closure* done){
        brpc::ClosureGuard rpc_guard(done);
        // 0. 定义一个错误处理函数, 当出错的时候进行调用 -- 这里在函数内部实现是因为每个接口的错误返回格式设置是不一样的
        auto err_response = [this, response](const std::string &rid, const std::string &errmsg){
            response->set_request_id(rid);
            response->set_success(false);
            response->set_errmsg(errmsg);
            return;
        };
        // 1. 从请求中取出用户ID和新的手机号, 验证码, 验证码ID
        std::string uid = request->user_id();
        std::string new_phone = request->phone_number();
        std::string code = request->phone_verify_code();
        std::string code_id = request->phone_verify_code_id();
        // 2. 判断手机号格式是否正确
        bool ret = phone_check(new_phone);
        if(ret == false) {
            LOG_ERROR("{} - 手机号长度不合法! ", request->request_id());
            return err_response(request->request_id(), "手机号长度不合法! ");
        }
        // 3. 对验证码进行验证
        auto vcode = _redis_codes->code(code_id);
        if(vcode != code) {
            LOG_ERROR("{} - 验证码错误 - {}-{}! ", request->request_id(), code_id, code);
            return err_response(request->request_id(), "验证码错误! ");
        }
        // 4. 从数据库通过用户ID进行用户信息查询, 判断用户是否存在
        auto user = _mysql_user->select_by_id(uid);
        if(!user) {
            LOG_ERROR("{} - 未找到用户信息 - {}! ", request->request_id(), uid);
            return err_response(request->request_id(), "未找到用户信息! ");
        }
        // 4. 将新的手机号更新到数据库中
        user->phone(new_phone);
        ret = _mysql_user->update(user);
        if(ret == false) {
            LOG_ERROR("{} - 更新数据库用户手机号失败: {}! ", request->request_id(), new_phone);
            return err_response(request->request_id(), "更新数据库用户手机号失败!");
        }
        // 5. 更新 ES 服务器中用户信息
        ret = _es_user->appendData(user->user_id(), *user->nickname(), *user->phone(), *user->description(),*user->avatar_id());
        if(ret == false) {
            LOG_ERROR("{} - 更新搜索引擎用户手机号失败: {}! ", request->request_id(), new_phone);
            return err_response(request->request_id(), "更新搜索引擎用户手机号失败!");
        }
        // 6. 组织响应, 返回更新成功与否
        response->set_request_id(request->request_id());
        response->set_success(true);    
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
