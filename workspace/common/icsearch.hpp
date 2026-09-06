#include <cpr/response.h>
#include <exception>
#include <iostream>
#include <elasticlient/client.h>
#include <cpr/cpr.h>
#include <json/json.h>
#include <json/value.h>
#include <memory>
#include <string>
#include <vector>
#include "logger.hpp"

// 序列化
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

// 反序列化
bool UnSerialize(const std::string &src, Json::Value &val)
{
    Json::CharReaderBuilder crb;
    std::unique_ptr<Json::CharReader> cr(crb.newCharReader());
    std::string err;
    bool ret = cr->parse(src.c_str(), src.c_str() + src.size(), &val, &err);
    if(ret == false)
    {
        std::cout << "Json反序列化失败!" << std::endl;
        return false;
    }
    return true;
}



// 索引的创建
class ESIndex
{
public:
    ESIndex(std::shared_ptr<elasticlient::Client> client, const std::string &name, const std::string &type = "_doc")
        : _client(client)
        , _name(name)
        , _type(type)
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
    bool create(const std::string &index_id = "default_index")
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
        // 发起搜索请求
        try 
        {
            auto rsp = _client->index(_name, _type, index_id, body);
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
        return true;
    }
private:
    std::string _name;
    std::string _type;
    Json::Value _index;
    Json::Value _properties;
    std::shared_ptr<elasticlient::Client> _client;
};

// 数据的新增
class ESInsert
{
public:
    ESInsert(std::shared_ptr<elasticlient::Client> client, const std::string &name, const std::string &type = "_doc")
        : _client(client)
        , _name(name)
        , _type(type){}
    // 针对这个项目我们只需要使用字符串类型即可, 如果需要使用其他类型的话, 就需要进行重载
    ESInsert& append(const std::string &key, const std::string &val)
    {
        _item[key] = val;
        return *this;
    }
    bool insert(const std::string &id = "")
    {
        std::string body;
        bool ret = Serialize(_item, body);
        if(ret == false)
        {
            LOG_ERROR("索引序列化失败!");
            return false;
        }
        // 发起搜索请求
        try 
        {
            auto rsp = _client->index(_name, _type, id, body);
            if(rsp.status_code < 200 || rsp.status_code >= 300)
            {
                LOG_ERROR("新增数据 {} 失败, 响应状态码异常: {}", _name, rsp.status_code);
                return false;
            }
        } 
        catch (std::exception &e)
        {
            LOG_ERROR("新增数据 {} 失败: {}", _name, e.what());
            return false;
        }
        return true;
    }
private:
    std::string _name;
    std::string _type;
    Json::Value _item;
    std::shared_ptr<elasticlient::Client> _client;
};


// 数据的删除
class ESRemove
{
public:
    ESRemove(std::shared_ptr<elasticlient::Client> client, const std::string &name, const std::string &type = "_doc")
        : _client(client)
        , _name(name)
        , _type(type){}
    bool remove(const std::string &id)
    {
        try 
        {
            auto rsp = _client->remove(_name, _type, id);
            if(rsp.status_code < 200 || rsp.status_code >= 300)
            {
                LOG_ERROR("删除数据 {} 失败, 响应状态码异常: {}", _name, rsp.status_code);
                return false;
            }
        } 
        catch (std::exception &e)
        {
            LOG_ERROR("删除数据 {} 失败: {}", _name, e.what());
            return false;
        }
        return true;
    }
private:
    std::string _name;
    std::string _type;
    std::shared_ptr<elasticlient::Client> _client;
};

// 数据的搜索
class ESSearch
{
public:
    ESSearch(std::shared_ptr<elasticlient::Client> client, const std::string &name, const std::string &type = "_doc")
        : _client(client)
        , _name(name)
        , _type(type){}
    // 必须不包含 -- terms -- 精确匹配,不分词
    ESSearch& append_must_not_terms(const std::string &key, const std::vector<std::string>& vals)
    {
        Json::Value fields;
        for(const auto& val : vals)
        {
            fields[key].append(val);
        }
        Json::Value terms;
        terms["terms"] = fields;
        _must_not.append(terms);
        return *this;
    }
    // 应该包含 -- match -- 分词匹配, 不用精确
    ESSearch& append_should_match(const std::string &key, const std::string& val)
    {
        Json::Value field;
        field[key] = val;
        Json::Value match;
        match["match"] = field;
        _should.append(match);
        return *this;
    }
    Json::Value search()
    {
        Json::Value cond;
        if(!_must_not.empty()) cond["must_not"] = _must_not;
        if(!_should.empty()) cond["should"] = _should;
        Json::Value query;
        query["bool"] = cond;
        Json::Value root;
        root["query"] = query;

        std::string body;
        bool ret = Serialize(root, body);
        if(ret == false)
        {
            LOG_ERROR("索引序列化失败!");
            return Json::Value();
        }
        LOG_DEBUG("检索正文: 【{}】", body);

        // 发起搜索请求
        cpr::Response rsp;
        try 
        {
            rsp = _client->search(_name, _type, body);
            if(rsp.status_code < 200 || rsp.status_code >= 300)
            {
                LOG_ERROR("检索数据 {} 失败, 响应状态码异常: {}", _name, rsp.status_code);
                return Json::Value();
            }
        } 
        catch (std::exception &e)
        {
            LOG_ERROR("检索数据 {} 失败: {}", _name, e.what());
            return false;
        }
        // 需要对响应正文进行反序列化
        LOG_DEBUG("检索响应正文: 【{}】", rsp.text);
        Json::Value json_res;
        UnSerialize(rsp.text, json_res);
        auto val = json_res["hits"]["hits"];
        return val;
    }
private:
    std::string _name;
    std::string _type;
    Json::Value _must_not;
    Json::Value _should;
    std::shared_ptr<elasticlient::Client> _client;
};