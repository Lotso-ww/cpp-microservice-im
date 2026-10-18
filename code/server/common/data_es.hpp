#include "icsearch.hpp"
#include "logger.hpp"
#include "../odb/user.hxx"
#include <elasticlient/client.h>
#include <json/value.h>
#include <memory>
#include <string>
#include <vector>

namespace lotso_im
{
    const std::string & user_key = "user_id";
    const std::string & nickname_key = "nickname";
    const std::string & phone_key = "phone";
    const std::string & description_key = "description";
    const std::string & avatar_key = "avatar_id";
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
}