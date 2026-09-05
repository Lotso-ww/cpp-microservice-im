#include <cstdint>
#include <exception>
#include <iostream>
#include <elasticlient/client.h>
#include <cpr/cpr.h>
#include <json/json.h>
#include <json/value.h>
#include <memory>
#include <string>
#include "logger.hpp"

// 封装一下序列化的操作
bool Serialize(const Json::Value &val, std::string &dst)
{
    // 先定义 Json::Streamwriter 工厂类 Json::StreamWriterBuilder
    Json::StreamWriterBuilder swb;
    std::unique_ptr<Json::StreamWriter> sw(swb.newStreamWriter());
    // 通过 Json::StreamWriter 中的 write 接口进行序列化
    std::stringstream ss;
    int ret = sw->write(val, &ss);
    if(ret != 0)
    {
        std::cout << "Json序列化失败!" << std::endl;
        return false;
    }
    dst = ss.str();
    return true;
}

class ESIndex
{
public:
    ESIndex(const std::string &name, const std::string &type, std::shared_ptr<elasticlient::Client> client)
        : _name(name)
        , _type(type)
        , _client(client)
    {
        Json::Value settings;
        Json::Value analysis;
        Json::Value analyzer;
        Json::Value ik;
        ik["tokenizer"] = "ik_max_word";
        analyzer["ik"] = ik;
        analysis["analyzer"] = analyzer;
        settings["analysis"] = analysis;
        _index["settings"] = settings;
    }
    // 直接使用 void 也是可以的, 但是这里这样使用可以让我们使用这个接口的时候更加优雅, 连续调用
    ESIndex& append(const std::string &key, const std::string &type = "text", const std::string &analyzer = "ik_max_word", bool enabled = true)
    {
        Json::Value fields;
        fields["type"] = type;
        fields["analyzer"] = analyzer;
        if(enabled == false) fields["enabled"] = enabled;
        _properties[key] = fields;
        return *this;
    }
    bool create()
    {
        Json::Value mappings;
        mappings["dynamic"] = true;
        mappings["properties"] = _properties;
        _index["mappings"] = mappings;

        // 序列化
        std::string body;
        bool ret = Serialize(_index, body);
        if(ret == false)
        {
            LOG_ERROR("索引序列化失败!");
            return false;
        }
        LOG_INFO("请求正文[{}]", body);
        // 发起搜索请求
        try 
        {
            std::cout << (uint64_t)_client.get() << std::endl;
            auto rsp = _client->index(_name, _type, "", body);
            if(rsp.status_code < 200 || rsp.status_code >= 300)
            {
                LOG_ERROR("创建ES索引 {} 失败, 响应状态码异常: {}", _name, rsp.status_code);
                return false;
            }
        } 
        catch (std::exception &e)
        {
            LOG_ERROR("创建ES索引 {} 失败: {}", _name, e.what());
            return false;
        }
        LOG_DEBUG("测试创建索引成功");
        return true;
    }
private:
    std::string _name;
    std::string _type;
    Json::Value _index;
    Json::Value _properties;
    std::shared_ptr<elasticlient::Client> _client;
};
