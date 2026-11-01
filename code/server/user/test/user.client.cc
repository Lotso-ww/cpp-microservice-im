#include "etcd.hpp"
#include "channel.hpp"
#include "utils.hpp"
#include <brpc/controller.h>
#include <gflags/gflags.h>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include "user.pb.h"
#include "base.pb.h"

DEFINE_bool(run_mode, false, "程序的运行模式, false-调试； true-发布；");
DEFINE_string(log_file, "", "发布模式下，用于指定日志的输出文件");
DEFINE_int32(log_level, 0, "发布模式下，用于指定日志输出等级");

DEFINE_string(etcd_host, "http://127.0.0.1:2379", "服务注册中心地址");
DEFINE_string(base_service, "/service", "服务监控根目录");
DEFINE_string(user_service, "/service/user_service", "服务监控根目录");

lotso_im::ServiceManager::ptr um;
lotso_im::UserInfo user_info;
std::string login_ssid;
std::string code_id;
std::string new_nickname = "亲爱的猪妈妈";

// 接口
void set_user_avatar(const std::string &uid, const std::string &avatar)
{
    auto channel = um->choose(FLAGS_user_service);
    ASSERT_TRUE(channel);
    lotso_im::SetUserAvatarReq req;
    req.set_request_id(lotso_im::uuid());
    req.set_user_id(uid);
    req.set_session_id(login_ssid);
    req.set_avatar(avatar);
    lotso_im::SetUserAvatarRsp rsp;
    brpc::Controller cntl;
    lotso_im::UserService_Stub stub(channel.get());
    stub.SetUserAvatar(&cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl.Failed());
    ASSERT_TRUE(rsp.success());
}
void get_code()
{
    auto channel = um->choose(FLAGS_user_service);
    ASSERT_TRUE(channel);

    lotso_im::PhoneVerifyCodeReq req;
    req.set_request_id(lotso_im::uuid());
    req.set_phone_number(user_info.phone());
    lotso_im::PhoneVerifyCodeRsp rsp;
    brpc::Controller cntl;
    lotso_im::UserService_Stub stub(channel.get());
    stub.GetPhoneVerifyCode(&cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl.Failed());
    ASSERT_TRUE(rsp.success());
    code_id = rsp.verify_code_id();
}

TEST(用户子服务测试, 用户注册测试) {
    auto channel = um->choose(FLAGS_user_service);
    ASSERT_TRUE(channel);
    user_info.set_nickname("猪妈妈");

    lotso_im::UserRegisterReq req;
    req.set_request_id(lotso_im::uuid());
    req.set_nickname(user_info.nickname());
    req.set_password("123456");
    lotso_im::UserRegisterRsp rsp;
    brpc::Controller cntl;
    lotso_im::UserService_Stub stub(channel.get());
    stub.UserRegister(&cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl.Failed());
    ASSERT_TRUE(rsp.success());
}

TEST(用户子服务测试, 用户登录测试) {
    auto channel = um->choose(FLAGS_user_service);
    ASSERT_TRUE(channel);

    lotso_im::UserLoginReq req;
    req.set_request_id(lotso_im::uuid());
    req.set_nickname(new_nickname);
    req.set_password("123456");
    lotso_im::UserLoginRsp rsp;
    brpc::Controller cntl;
    lotso_im::UserService_Stub stub(channel.get());
    stub.UserLogin(&cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl.Failed());
    ASSERT_TRUE(rsp.success());
    login_ssid = rsp.login_session_id(); // 把登录会话id给存进来
}

TEST(用户子服务测试, 用户头像设置测试) {
    auto channel = um->choose(FLAGS_user_service);
    ASSERT_TRUE(channel);

    lotso_im::SetUserAvatarReq req;
    req.set_request_id(lotso_im::uuid());
    req.set_user_id(user_info.user_id());
    req.set_session_id(login_ssid);
    req.set_avatar(user_info.avatar());
    lotso_im::SetUserAvatarRsp rsp;
    brpc::Controller cntl;
    lotso_im::UserService_Stub stub(channel.get());
    stub.SetUserAvatar(&cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl.Failed());
    ASSERT_TRUE(rsp.success());
}

TEST(用户子服务测试, 用户昵称设置测试) {
    auto channel = um->choose(FLAGS_user_service);
    ASSERT_TRUE(channel);

    lotso_im::SetUserNicknameReq req;
    req.set_request_id(lotso_im::uuid());
    req.set_user_id(user_info.user_id());
    req.set_session_id(login_ssid);
    req.set_nickname(new_nickname);
    lotso_im::SetUserNicknameRsp rsp;
    brpc::Controller cntl;
    lotso_im::UserService_Stub stub(channel.get());
    stub.SetUserNickname(&cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl.Failed());
    ASSERT_TRUE(rsp.success());
}

TEST(用户子服务测试, 用户签名设置测试) {
    auto channel = um->choose(FLAGS_user_service);
    ASSERT_TRUE(channel);

    lotso_im::SetUserDescriptionReq req;
    req.set_request_id(lotso_im::uuid());
    req.set_user_id(user_info.user_id());
    req.set_session_id(login_ssid);
    req.set_description(user_info.description());
    lotso_im::SetUserDescriptionRsp rsp;
    brpc::Controller cntl;
    lotso_im::UserService_Stub stub(channel.get());
    stub.SetUserDescription(&cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl.Failed());
    ASSERT_TRUE(rsp.success());
}

TEST(用户子服务测试, 用户信息获取测试) {
    auto channel = um->choose(FLAGS_user_service);
    ASSERT_TRUE(channel);

    lotso_im::GetUserInfoReq req;
    req.set_request_id(lotso_im::uuid());
    req.set_user_id(user_info.user_id());
    req.set_session_id(login_ssid);
    lotso_im::GetUserInfoRsp rsp;
    brpc::Controller cntl;
    lotso_im::UserService_Stub stub(channel.get());
    stub.GetUserInfo(&cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl.Failed());
    ASSERT_TRUE(rsp.success());
    ASSERT_EQ(user_info.user_id(), rsp.user_info().user_id());
    ASSERT_EQ(new_nickname, rsp.user_info().nickname());
    ASSERT_EQ(user_info.description(), rsp.user_info().description());
    ASSERT_EQ(user_info.avatar(), rsp.user_info().avatar());
    // ASSERT_EQ(user_info.phone(), rsp.user_info().phone()); //手机号我们没设置所以这里就不比对了
}

TEST(用户子服务测试, 用户信息批量获取测试) {
    set_user_avatar("用户ID1", "小猪佩奇的头像数据");
    set_user_avatar("用户ID2", "小猪乔治的头像数据");
    auto channel = um->choose(FLAGS_user_service);
    ASSERT_TRUE(channel);

    lotso_im::GetMultiUserInfoReq req;
    req.set_request_id(lotso_im::uuid());
    req.add_users_id("用户ID1");
    req.add_users_id("用户ID2");
    req.add_users_id("cdf3-5fb374c3-0000");
    lotso_im::GetMultiUserInfoRsp rsp;
    brpc::Controller cntl;
    lotso_im::UserService_Stub stub(channel.get());
    stub.GetMultiUserInfo(&cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl.Failed());
    ASSERT_TRUE(rsp.success());
    
    auto users_map = rsp.mutable_users_info();
    lotso_im::UserInfo fuser = (*users_map)["cdf3-5fb374c3-0000"];
    ASSERT_EQ(fuser.user_id(), "cdf3-5fb374c3-0000");
    ASSERT_EQ(fuser.nickname(), "猪爸爸");
    ASSERT_EQ(fuser.description(), "");
    ASSERT_EQ(fuser.phone(), "");
    ASSERT_EQ(fuser.avatar(), "");
    lotso_im::UserInfo puser = (*users_map)["用户ID1"];
    ASSERT_EQ(puser.user_id(), "用户ID1");
    ASSERT_EQ(puser.nickname(), "小猪佩奇");
    ASSERT_EQ(puser.description(), "这是一只小猪");
    ASSERT_EQ(puser.phone(), "手机号1");
    ASSERT_EQ(puser.avatar(), "小猪佩奇的头像数据");
    lotso_im::UserInfo quser = (*users_map)["用户ID2"];
    ASSERT_EQ(quser.user_id(), "用户ID2");
    ASSERT_EQ(quser.nickname(), "小猪乔治");
    ASSERT_EQ(quser.description(), "这是一只小小猪");
    ASSERT_EQ(quser.phone(), "手机号2");
    ASSERT_EQ(quser.avatar(), "小猪乔治的头像数据");
}

TEST(用户子服务测试, 手机号注册) {
    get_code();
    auto channel = um->choose(FLAGS_user_service);
    ASSERT_TRUE(channel);

    lotso_im::PhoneRegisterReq req;
    req.set_request_id(lotso_im::uuid());
    req.set_phone_number(user_info.phone());
    req.set_verify_code_id(code_id);
    std::cout << "手机号注册, 输入验证码: " << std::endl;
    std::string code;
    std::cin >> code;
    req.set_verify_code(code);
    lotso_im::PhoneRegisterRsp rsp;
    brpc::Controller cntl;
    lotso_im::UserService_Stub stub(channel.get());
    stub.PhoneRegister(&cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl.Failed());
    ASSERT_TRUE(rsp.success());
}

TEST(用户子服务测试, 手机号登录) {
    get_code();
    auto channel = um->choose(FLAGS_user_service);
    ASSERT_TRUE(channel);

    lotso_im::PhoneLoginReq req;
    req.set_request_id(lotso_im::uuid());
    req.set_phone_number(user_info.phone());
    req.set_verify_code_id(code_id);
    std::cout << "手机号登录, 输入验证码: " << std::endl;
    std::string code;
    std::cin >> code;
    req.set_verify_code(code);
    lotso_im::PhoneLoginRsp rsp;
    brpc::Controller cntl;
    lotso_im::UserService_Stub stub(channel.get());
    stub.PhoneLogin(&cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl.Failed());
    ASSERT_TRUE(rsp.success());
    std::cout << "手机号登录会话 ID: " << rsp.login_session_id() << std::endl;
}

TEST(用户子服务测试, 手机号设置) {
    get_code();
    auto channel = um->choose(FLAGS_user_service);
    ASSERT_TRUE(channel);

    lotso_im::SetUserPhoneNumberReq req;
    req.set_request_id(lotso_im::uuid());
    std::cout << "手机号设置, 输入用户ID: " << std::endl;
    std::string user_id;
    std::cin >> user_id;
    req.set_user_id(user_id);
    req.set_phone_number("18888888888");
    req.set_phone_verify_code_id(code_id);
    std::cout << "手机号注册, 输入验证码: " << std::endl;
    std::string code;
    std::cin >> code;
    req.set_phone_verify_code(code);
    lotso_im::SetUserPhoneNumberRsp rsp;
    brpc::Controller cntl;
    lotso_im::UserService_Stub stub(channel.get());
    stub.SetUserPhoneNumber(&cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl.Failed());
    ASSERT_TRUE(rsp.success());
}

int main(int argc, char* argv[])
{
    // 单元测试框架的初始化
    testing::InitGoogleTest(&argc, argv);
    google::ParseCommandLineFlags(&argc, &argv, true);
    lotso_im::init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    // 先构造 Rpc 信道管理对象
    um = std::make_shared<lotso_im::ServiceManager>();
    um->declared(FLAGS_user_service);
    auto put_cb = std::bind(&lotso_im::ServiceManager::onServiceOnline, um.get(), std::placeholders::_1, std::placeholders::_2);
    auto del_cb = std::bind(&lotso_im::ServiceManager::onServiceOffline, um.get(), std::placeholders::_1, std::placeholders::_2);

    // 构造服务发现对象
    lotso_im::Discovery::ptr dclient = std::make_shared<lotso_im::Discovery>(FLAGS_etcd_host, FLAGS_base_service, put_cb, del_cb);

    // 设置用户信息
    user_info.set_nickname("猪妈妈");
    user_info.set_user_id("0d34-1355e6bc-0000");
    user_info.set_description("这是一个美丽的猪妈妈");
    user_info.set_phone("15827502236");
    user_info.set_avatar("猪妈妈头像数据");
    return RUN_ALL_TESTS();
}
