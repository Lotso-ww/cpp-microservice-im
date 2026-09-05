#include <exception>
#include <iostream>
#include <elasticlient/client.h>
#include <cpr/cpr.h>

int main(int argc, char *argv[])
{
    // 1. 构造 ES 客户端
    elasticlient::Client client({"http://127.0.0.1:9200/"});
    // 2. 发起搜索请求
    try 
    {
        auto rsp = client.search("user", "_doc", "{\"query\": {\"match_all\":{} }}");
        // 3. 打印响应结果和响应正文
        std::cout << rsp.status_code << std::endl;
        std::cout << rsp.text << std::endl;   
    } 
    catch (std::exception &e) 
    {
        std::cout << "请求失败: " << e.what() << std::endl;
    }
    return 0;
}