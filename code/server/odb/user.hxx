#pragma once
#include <gtest/internal/gtest-internal.h>
#include <odb/forward.hxx>
#include <string>
#include <cstddef>
#include <odb/nullable.hxx>
#include <odb/core.hxx>

#pragma db object table("user")
class User 
{
public:
    User(){}
    // 用户名 -- 新增用户 -- 用户ID, 昵称, 密码
    User(const std::string &uid, const std::string &nickname, const std::string &password)
        : _user_id(uid), _nickname(nickname), _password(password){}
    //手机号 -- 新增用户 -- 用户ID, 随机昵称, 手机号
    User(const std::string &uid, const std::string &phone)
        : _user_id(uid), _nickname(uid), _phone(phone){}
    
    // 获取和设置属性接口
    std::string user_id() { return _user_id; }

    odb::nullable<std::string> nickname() { return _nickname; }
    void nickname(const std::string &val) { _nickname = val; }

    odb::nullable<std::string> description() { return _description; }
    void description(const std::string &val) { _description = val; }

    odb::nullable<std::string> password() { return _password; }
    void password(const std::string &val) { _password = val; }

    odb::nullable<std::string> phone() { return _phone; }
    void phone(const std::string &val) { _phone = val; }

    odb::nullable<std::string> avarta_id() { return _avarta_id; }
    void avarta_id(const std::string &val) { _avarta_id = val; }
private:
    friend class odb::access;
    #pragma db id auto
    unsigned long _id;
    #pragma db index
    std::string _user_id;
    #pragma db index
    odb::nullable<std::string> _nickname;       // 用户昵称 -- 不一定存在 -- 手机号注册的时候就是随机的
    #pragma db index
    odb::nullable<std::string> _description;    // 用户签名 -- 不一定存在 -- 可以不设置
    odb::nullable<std::string> _password;       // 用户密码 -- 不一定存在 -- 手机号注册的默认就没有
    #pragma db index
    odb::nullable<std::string> _phone;          // 用户手机号 -- 不一定存在 -- 用户名注册
    odb::nullable<std::string> _avarta_id;      // 用户头像ID -- 不一定存在
};