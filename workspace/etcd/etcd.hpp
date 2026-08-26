#include <cstdint>
#include <etcd/Value.hpp>
#include <etcd/Client.hpp>
#include <etcd/KeepAlive.hpp>
#include <etcd/Watcher.hpp>
#include <etcd/Response.hpp>
#include <functional>
#include <memory>
#include <string>
#include <system_error>
#include "../common/logger.hpp"

// 服务注册客户端类
class Registry 
{
public:
    Registry(const std::string &host)
        : _client(std::make_shared<etcd::Client>(host))
        , _keep_alive(_client->leasekeepalive(3).get())
        , _lease_id(_keep_alive->Lease())
        {}
    ~Registry()
    {
        // 取消保活
        _keep_alive->Cancel();
    }
    bool registry(const std::string &key, const std::string& val)
    {
        auto resp = _client->put("/service/user", "127.0.0.1:8080", _lease_id).get();
        if(resp.is_ok() == false)
        {
            LOG_ERROR("注册数据失败: {}", resp.error_message());
            return false;
        }
        return true;
    }
private:
    std::shared_ptr<etcd::Client> _client;
    std::shared_ptr<etcd::KeepAlive> _keep_alive;
    uint64_t _lease_id;
};

// 服务器发现客户端
class Discovery
{
public:
    using NotifyCallback = std::function<void(std::string, std::string)>;
    Discovery(const std::string &host, const std::string &basedir, const NotifyCallback &put_cb, const NotifyCallback &del_cb)
        : _client(std::make_shared<etcd::Client>(host))
        , _put_cb(put_cb)
        , _del_cb(del_cb)
    {
        // 先进行服务发现, 先获取到当前已有的数据
        auto resp = _client->ls(basedir).get();
        if(resp.is_ok() == false)
        {
            LOG_ERROR("获取服务信息数据失败: {}", resp.error_message());
            exit(-1);
        }
        int sz = resp.keys().size();
        for(int i = 0; i < sz; i++)
        {
            if(_put_cb) _put_cb(resp.key(i), resp.value(i).as_string());
        }
        // 然后进行事件监控，监控数据发生的改变并调用回调进行处理
        _watcher = std::make_shared<etcd::Watcher>(*_client.get(), basedir, std::bind(&Discovery::callback, this, std::placeholders::_1), true);
    }
private:
    void callback(const etcd::Response &resp)
    {
        if(resp.is_ok() == false)
        {
            LOG_ERROR("收到一个错误的事件处理通知: {}", resp.error_message());
            return;
        }
        for(auto const& ev : resp.events())
        {
            if(ev.event_type() == etcd::Event::EventType::PUT)
            {
                if(_put_cb) _put_cb(ev.kv().key(), ev.kv().as_string());
                LOG_DEBUG("新增服务: {}-{}", ev.kv().key(), ev.kv().as_string());
            }
            else if(ev.event_type() == etcd::Event::EventType::DELETE_)
            {
                if(_del_cb) _del_cb(ev.prev_kv().key(), ev.prev_kv().as_string());
                LOG_DEBUG("下线服务: {}-{}", ev.prev_kv().key(), ev.prev_kv().as_string());
            }
        }
    }
private:
    std::shared_ptr<etcd::Client> _client;
    std::shared_ptr<etcd::Watcher> _watcher;
    NotifyCallback _put_cb;
    NotifyCallback _del_cb;
};