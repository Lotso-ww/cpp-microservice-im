#include "../../../common/data_mysql.hpp"
#include "../../../odb/chat_session_member.hxx"
#include "user-odb.hxx"
#include <memory>
#include <iostream>
#include <gflags/gflags.h>
#include <vector>

DEFINE_bool(run_mode, false, "程序的运行模式, false-调试； true-发布；");
DEFINE_string(log_file, "", "发布模式下，用于指定日志的输出文件");
DEFINE_int32(log_level, 0, "发布模式下，用于指定日志输出等级");

void append_test(lotso_im::ChatSessionMemberTable &tb)
{
    auto csm1 = std::make_shared<lotso_im::ChatSessionMember>("会话1", "用户ID1");
    tb.append(csm1);
    auto csm2 = std::make_shared<lotso_im::ChatSessionMember>("会话1", "用户ID2");
    tb.append(csm2);
    auto csm3 = std::make_shared<lotso_im::ChatSessionMember>("会话2", "用户ID3");
    tb.append(csm3);
}
void multi_append_test(lotso_im::ChatSessionMemberTable &tb)
{
    auto csm1 = std::make_shared<lotso_im::ChatSessionMember>("会话3", "用户ID1");
    auto csm2 = std::make_shared<lotso_im::ChatSessionMember>("会话3", "用户ID2");
    auto csm3 = std::make_shared<lotso_im::ChatSessionMember>("会话3", "用户ID3");
    tb.append({csm1, csm2, csm3});
}
void remove_test(lotso_im::ChatSessionMemberTable &tb)
{
    auto csm3 = std::make_shared<lotso_im::ChatSessionMember>("会话3", "用户ID3");
    tb.remove(csm3);
}
void ss_members(lotso_im::ChatSessionMemberTable &tb)
{
    auto ret = tb.members("会话3");
    for(auto &id : ret) {
        std::cout << id << std::endl;
    }
}
void remove_all_test(lotso_im::ChatSessionMemberTable &tb)
{
    tb.remove("会话3");
}


int main(int argc, char *argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    lotso_im::init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);
    auto db = lotso_im::ODBFactory::create("imuser", "imuser_666", "127.0.0.1", "my_im", "utf8", 0, 1);
    lotso_im::ChatSessionMemberTable csmt(db);

    // append_test(csmt);
    // multi_append_test(csmt);
    // remove_test(csmt);
    // ss_members(csmt);
    remove_all_test(csmt);
    return 0;
}