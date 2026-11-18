#include "icsearch.hpp"
#include "logger.hpp"
#include "../odb/user.hxx"
#include "message.hxx"
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <elasticlient/client.h>
#include <json/value.h>
#include <memory>
#include <string>
#include <vector>

namespace lotso_im
{
    const std::string &user_key = "user_id";
    const std::string &nickname_key = "nickname";
    const std::string &phone_key = "phone";
    const std::string &description_key = "description";
    const std::string &avatar_key = "avatar_id";
    class ESClientFactory
    {
    public:
        static std::shared_ptr<elasticlient::Client> create(const std::vector<std::string> host_list)
        {
            return std::make_shared<elasticlient::Client>(host_list);
        }
    };
    class ESUser
    {
    public:
        using ptr = std::shared_ptr<ESUser>;
        ESUser(const std::shared_ptr<elasticlient::Client> &client) : _es_client(client)
        {}
        bool createIndex()
        {
            bool ret = ESIndex(_es_client, "user")
                .append(user_key, "keyword", "standard", true)
                .append(nickname_key)
                .append(phone_key, "keyword", "standard", true)
                .append(description_key, "keyword", "standard", false)
                .append(avatar_key, "keyword", "standard", false)
                .create();
            if(ret == false)
            {
                LOG_ERROR("用户信息索引创建失败!");
                return false;
            }
            LOG_INFO("用户信息索引创建成功!");
            return true;
        }
        bool appendData(const std::string &uid, const std::string &nickname, const std::string &phone, const std::string &description, const std::string &avatar_id)
        {
            int ret = ESInsert(_es_client, "user")
                .append(user_key, uid)
                .append(nickname_key, nickname)
                .append(phone_key, phone)
                .append(description_key, description)
                .append(avatar_key, avatar_id)
                .insert(uid);
            if(ret == false)
            {
                LOG_ERROR("用户数据插入/更新失败!");
                return false;
            }
            LOG_INFO("用户数据插入/更新成功!");
            return true;
        }
        std::vector<User> search(const std::string &key, const std::vector<std::string> &uid_list)
        {
            std::vector<User> ret;
            Json::Value json_user = ESSearch(_es_client, "user")
                .append_should_match(phone_key + ".keyword", key)
                .append_should_match(user_key + ".keyword", key)
                .append_should_match(nickname_key, key)
                .append_must_not_terms(user_key + ".keyword", uid_list)
                .search();
            if(json_user.isArray() == false)
            {
                LOG_ERROR("用户搜索结果为空, 或者结果不是数组类型");
                return ret;
            }
            int sz = json_user.size();
            for(int i = 0; i < sz; i++)
            {
                User user;
                user.user_id(json_user[i]["_source"][user_key].asString());
                user.nickname(json_user[i]["_source"][nickname_key].asString());
                user.phone(json_user[i]["_source"][phone_key].asString());
                user.description(json_user[i]["_source"][description_key].asString());
                user.avatar_id(json_user[i]["_source"][avatar_key].asString());
                ret.push_back(user);
            }
            return ret;
        }
    private:
        std::shared_ptr<elasticlient::Client> _es_client;
    };

    // 之前没有的
    const std::string &message_key = "message_id";
    const std::string &ctime_key = "create_time";
    const std::string &session_key = "chat_session_id";
    const std::string &content_key = "content";
    class ESMessage {
    public:
        using ptr = std::shared_ptr<ESMessage>;
        ESMessage(const std::shared_ptr<elasticlient::Client> &es_client): _es_client(es_client) {}
        bool createIndex()
        {
            bool ret = ESIndex(_es_client, "message")
                .append(user_key, "keyword", "standard", false)
                .append(message_key, "keyword", "standard", false)
                .append(ctime_key, "long", "standard", false)
                .append(session_key, "keyword", "standard", true)
                .append(content_key)
                .create();
            if(ret == false)
            {
                LOG_ERROR("用户信息索引创建失败!");
                return false;
            }
            LOG_INFO("用户信息索引创建成功!");
            return true;
        }
        bool appendData(const std::string &user_id, const std::string &message_id, const long &create_time, const std::string &chat_session_id, const std::string &content)
        {
            int ret = ESInsert(_es_client, "message")
                .append(message_key, message_id)
                .append(ctime_key, create_time)
                .append(user_key, user_id)
                .append(session_key, chat_session_id)
                .append(content_key, content)
                .insert(message_id);
            if(ret == false)
            {
                LOG_ERROR("用户数据插入/更新失败!");
                return false;
            }
            LOG_INFO("用户数据插入/更新成功!");
            return true;
        }
        bool remove(const std::string &mid) {
            bool ret = ESRemove(_es_client, "message").remove(mid);
            if(ret == false)
            {
                LOG_ERROR("消息数据删除失败!");
                return false;
            }
            LOG_INFO("消息数据删除成功!");
            return true;
        }
        std::vector<lotso_im::Message> search(const std::string &key, const std::string &ssid) {
            std::vector<lotso_im::Message> ret;
            Json::Value json_message = ESSearch(_es_client, "message")
                .append_must_term(session_key + ".keyword", ssid)
                .append_must_match(content_key, key)
                .search();
            if(json_message.isArray() == false)
            {
                LOG_ERROR("用户搜索结果为空, 或者结果不是数组类型");
                return ret;
            }
            int sz = json_message.size();
            for(int i = 0; i < sz; i++)
            {
                lotso_im::Message message;
                message.user_id(json_message[i]["_source"][user_key].asString());
                message.message_id(json_message[i]["_source"][message_key].asString());
                boost::posix_time::ptime ctime(boost::posix_time::from_time_t(json_message[i]["_source"][ctime_key].asInt64()));
                message.create_time(ctime);
                message.session_id(json_message[i]["_source"][session_key].asString());    
                message.content(json_message[i]["_source"][content_key].asString());
                ret.push_back(message);
            }
            return ret;
        }
    private:
        std::shared_ptr<elasticlient::Client> _es_client;
    };
}