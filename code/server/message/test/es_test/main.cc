#include "../../../common/data_es.hpp"
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include <chrono>
#include <elasticlient/client.h>
#include <memory>
#include <iostream>
#include <gflags/gflags.h>
#include <thread>
#include <vector>

DEFINE_bool(run_mode, false, "程序的运行模式, false-调试； true-发布；");
DEFINE_string(log_file, "", "发布模式下，用于指定日志的输出文件");
DEFINE_int32(log_level, 0, "发布模式下，用于指定日志输出等级");

DEFINE_string(es_host, "http://127.0.0.1:9200/", "es服务器URL");

int main(int argc, char *argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    lotso_im::init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);
    
    auto es_client = lotso_im::ESClientFactory::create({FLAGS_es_host});
    auto es_msg = std::make_shared<lotso_im::ESMessage>(es_client);

    es_msg->createIndex();
    es_msg->appendData("用户ID1", "消息ID1", 1795009637, "会话ID1", "吃饭了吗？");
    es_msg->appendData("用户ID2", "消息ID2", 1795009637 - 100, "会话ID1", "吃的盖浇饭! ");
    es_msg->appendData("用户ID3", "消息ID3", 1795009637, "会话ID2", "吃饭了吗？");
    es_msg->appendData("用户ID4", "消息ID4", 1795009637 - 100, "会话ID2", "吃的盖浇饭! ");
    std::this_thread::sleep_for(std::chrono::seconds(10)); // 10s 后再去查
    auto ret = es_msg->search("盖浇饭","会话ID1");
    for(auto &m : ret)
    {
        std::cout << "--------------------------------" << std::endl;
        std::cout << m.user_id() << std::endl;
        std::cout << m.message_id() << std::endl;
        std::cout << m.session_id() << std::endl;
        std::cout << boost::posix_time::to_simple_string(m.create_time()) << std::endl;
        std::cout << m.content() << std::endl;
    }
    return 0;
}