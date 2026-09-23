#include <chrono>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <iostream>

int main()
{
    // 设置全局的刷新策略
    // 1. 每秒刷新
    spdlog::flush_every(std::chrono::seconds(1));
    // 2. 遇到 debug 以上等级的日志立即刷新(包含 debug)
    spdlog::flush_on(spdlog::level::level_enum::debug);

    // 设置全局的日志输出等级 -- 其实是无所谓的, 因为每个日志器可以独立进行设置
    spdlog::set_level(spdlog::level::level_enum::debug);

    // 创建同步日志器(标准输出/文件)
    // auto logger = spdlog::stdout_color_mt("default-logger");  // 标准输出
    auto logger = spdlog::basic_logger_mt("file-logger", "sync.log");  // 标准输出


    // 设置日志器的刷新策略, 以及日志器的输出等级
    // 但是上面设置过全局的所以这里其实也可以不需要
    // logger->flush_on(spdlog::level::level_enum::debug);
    // logger->set_level(spdlog::level::level_enum::debug);
    
    // 设置日志的输出格式 [日志器名称][时分秒][线程][日志等级] 日志内容
    logger->set_pattern("[%n][%H%M%S][%t][%-8l] %v");

    // 进行简单的日志输出
    logger->trace("你好！{}", "小明"); // 这个是不输出的, 因为我们设置了输出 debug 以上的
    logger->debug("你好！{}", "小明"); 
    logger->info("你好！{}", "小明"); 
    logger->warn("你好！{}", "小明"); 
    logger->error("你好！{}", "小明"); 
    logger->critical("你好！{}", "小明"); 
    std::cout << "日志输出完毕!" << std::endl;

    return 0;
}