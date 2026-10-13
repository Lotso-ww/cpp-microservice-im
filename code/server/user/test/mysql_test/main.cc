#include "../../../common/data_mysql.hpp"
#include "../../../odb/user.hxx"
#include "user-odb.hxx"
#include <memory>
#include <iostream>
#include <gflags/gflags.h>
#include <vector>

DEFINE_bool(run_mode, false, "程序的运行模式, false-调试； true-发布；");
DEFINE_string(log_file, "", "发布模式下，用于指定日志的输出文件");
DEFINE_int32(log_level, 0, "发布模式下，用于指定日志输出等级");


void insert(lotso_im::UserTable &user_tb)
{
    auto user1 = std::make_shared<lotso_im::User>("uid1", "昵称1", "123456");
    user_tb.insert(user1);

    auto user2 = std::make_shared<lotso_im::User>("uid2", "155567666671");
    user_tb.insert(user2);
}
void update_by_id(lotso_im::UserTable &user_tb)
{
    auto user = user_tb.select_by_id("uid1");
    user->description("我是Lotso!");
    user_tb.update(user);   
}
void update_by_phone(lotso_im::UserTable &user_tb)
{
    auto user = user_tb.select_by_phone("155567666671");
    user->password("lotso123");
    user_tb.update(user);   
}
void update_by_nickname(lotso_im::UserTable &user_tb)
{
    auto user = user_tb.select_by_nickname("uid2"); // 用手机号注册的号默认昵称为 id
    user->nickname("昵称2");
    user_tb.update(user);   
}
void select_users(lotso_im::UserTable &user_tb)
{
    std::vector<std::string> id_list = {"uid1", "uid2"};
    auto res = user_tb.select_multi_users(id_list);
    for(auto user : res)
    {
        if(user.nickname()) std::cout << *user.nickname() << std::endl; 
    }
}

int main(int argc, char *argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    lotso_im::init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);
    auto db = lotso_im::ODBFactory::create("imuser", "imuser_666", "127.0.0.1", "my_im", "utf8", 0, 1);
    lotso_im::UserTable user(db);
    // insert(user);
    // update_by_id(user);
    // update_by_phone(user);
    // update_by_nickname(user);
    select_users(user);
    return 0;
}