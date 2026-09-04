#pragma once
#include <brpc/channel.h>
#include <brpc/reloadable_flags.h>
#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_set>
#include <utility>
#include <vector>
#include <string>
#include <unordered_map>
#include "logger.hpp"

// 单个服务的信道管理类
class ServiceChannel
{
public:
    using ptr = std::shared_ptr<ServiceChannel>;
    using Channelptr = std::shared_ptr<brpc::Channel>;
    ServiceChannel(const std::string &name)
        : _index(0)
        , _servicename(name)
    {}
    // 服务器上线了一个节点, 则调用 append 新增信道
    void append(const std::string &host)
    {
        auto channel = std::make_shared<brpc::Channel>();
        brpc::ChannelOptions options;
        options.connect_timeout_ms = -1; 
        options.timeout_ms = -1; 
        options.max_retry = 3; 
        options.protocol = "baidu_std"; 
        int ret = channel->Init(host.c_str(), &options);
        if(ret == -1)
        {
            LOG_ERROR("初始化{}-{}信道失败!",_servicename, host);
            return;
        }
        std::unique_lock<std::mutex> lock_guard(_mutex);
        _hosts.insert(std::make_pair(host, channel));
        _channels.push_back(channel);
    }
    // 服务器下线了一个节点, 则调用 remove 释放信道
    void remove(const std::string &host)
    {
        std::unique_lock<std::mutex> lock_guard(_mutex);
        auto it = _hosts.find(host);
        if(it == _hosts.end())
        {
            LOG_WARN("{}-{}节点删除信道时, 没有找到信道信息!", _servicename, host);
            return;
        }
        for(auto vit = _channels.begin(); vit != _channels.end(); ++vit)
        {
            if(*vit == it->second)
            {
                _channels.erase(vit);
                break;
            }
        }
        _hosts.erase(it);
    }
    // 通过 RR 轮转策略, 获取一个 channel 用于发起对应服务的 rpc 调用
    Channelptr choose()
    {
        std::unique_lock<std::mutex> lock_guard(_mutex);
        if(_channels.size() == 0)
        {
            LOG_ERROR("当前没有能够提供 {} 服务的节点", _servicename);
            return Channelptr();
        } 
        int32_t idx = _index++ % _channels.size();
        return _channels[idx];
    }
private:
    std::mutex _mutex;
    int32_t _index; // 当前轮转下标计数器
    std::string _servicename; // 服务名称
    std::vector<Channelptr> _channels; // 当前服务对应的信道集合
    std::unordered_map<std::string, Channelptr> _hosts; // 主机地址与信道映射关系
};


// 总体的服务信道管理类
class ServiceManager
{
public:
    using ptr = std::shared_ptr<ServiceManager>;
    ServiceManager(){}
    ~ServiceManager(){}
    // 获取指定服务的节点信道
    ServiceChannel::Channelptr choose(const std::string &service_name)
    {
        std::unique_lock<std::mutex> lock_guard(_mutex);
        auto sit = _services.find(service_name);
        if(sit == _services.end())
        {
            LOG_ERROR("当前没有能够提供 {} 服务的节点", service_name);
            return ServiceChannel::Channelptr();
        }
        return sit->second->choose();
    }
    // 先声明, 我关注哪些服务的上下线, 不关心的就不需要管理了
    void declared(const std::string &service_name)
    {
        std::unique_lock<std::mutex> lock_guard(_mutex);
        _follow_services.insert(service_name);
    }
    // 服务器上线时调用的回调接口, 将服务节点管理起来
    void onServiceOnline(const std::string &service_instance, const std::string &host)
    {
        std::string service_name = getServiceName(service_instance);
        ServiceChannel::ptr service;
        {
            std::unique_lock<std::mutex> lock_guard(_mutex);
            auto fit = _follow_services.find(service_name);
            if(fit == _follow_services.end())
            {
                LOG_DEBUG("{}-{} 服务上线了, 但是当前并不关心!", service_name, host);
                return;
            }
            // 先获取管理对象, 没有则创建, 有则添加节点
            auto sit = _services.find(service_name);
            if(sit == _services.end())
            {
                service = std::make_shared<ServiceChannel>(service_name);
                _services.insert(std::make_pair(service_name, service));
            }
            else 
            {
                service = sit->second;
            }
        }
        if(!service)
        {
            LOG_ERROR("新增 {} 服务管理节点失败!", service_name);
            return;
        }
        service->append(host); // 这个操作内部我们也是加锁了的, 所以写在这里加锁的外面
        LOG_DEBUG("{}-{} 服务上线新节点, 进行添加管理!", service_name, host);
    }
    // 服务下线时调用的接口, 从服务信道管理中, 删除指定节点信道
    void onServiceOffline(const std::string &service_instance, const std::string &host)
    {
        std::string service_name = getServiceName(service_instance);
        ServiceChannel::ptr service;
        {
            std::unique_lock<std::mutex> lock_guard(_mutex);
            auto fit = _follow_services.find(service_name);
            if(fit == _follow_services.end())
            {
                LOG_DEBUG("{}-{} 服务下线了, 但是当前并不关心!", service_name, host);
                return;
            }
            // 先获取管理对象, 没有则创建, 有则删除节点
            auto sit = _services.find(service_name);
            if(sit == _services.end())
            {
                LOG_WARN("删除{}服务节点时, 没有找到管理对象", service_name);
                return;
            }
            service = sit->second;
        }
        service->remove(host);
        LOG_DEBUG("{}-{} 服务下线节点, 进行删除管理!", service_name, host);
    }
private:
    std::string getServiceName(const std::string &service_instance)
    {
        auto pos = service_instance.find_last_of('/');
        if(pos == std::string::npos) return service_instance;
        return service_instance.substr(0, pos);
    }
private:
    std::mutex _mutex;
    std::unordered_set<std::string> _follow_services; // 关心的服务
    std::unordered_map<std::string, ServiceChannel::ptr> _services; 
};