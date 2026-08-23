#include <chrono>
#include <spdlog/async_logger.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>
#include <iostream>


int main()
{
    spdlog::flush_every(std::chrono::seconds(1));
    spdlog::flush_on(spdlog::level::level_enum::debug);
    spdlog::set_level(spdlog::level::level_enum::debug);

    // 初始化异步日志输出线程配置 -- 但是不设置使用默认的也是可以的
    spdlog::init_thread_pool(3072, 1);
    // 创建异步日志器(标准输出/文件) -- 其实就是指定一下模版参数即可 -- 工厂接口默认创建的就是同步日志器
    auto logger = spdlog::stdout_color_mt<spdlog::async_factory>("async-logger");  // 标准输出
    
    logger->set_pattern("[%n][%H%M%S][%t][%-8l] %v");
    logger->trace("你好！{}", "小明"); // 这个是不输出的, 因为我们设置了输出 debug 以上的
    logger->debug("你好！{}", "小明"); 
    logger->info("你好！{}", "小明"); 
    logger->warn("你好！{}", "小明"); 
    logger->error("你好！{}", "小明"); 
    logger->critical("你好！{}", "小明"); 
    // 这次输出的时候，最后的打印在最前面，这个是因为我们这次使用的是异步的
    // 这个时候的日志不是立即输出的，是先放在内存里面，由我们的工作线程进行输出
    // 当然我们现在没对工作线程进行任何设置就是使用的默认的，不过我们自己也是可以进行设置的
    std::cout << "日志输出完毕!" << std::endl; 
    return 0;
}