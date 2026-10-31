#pragma once
#include "user.hxx"
#include "user-odb.hxx"
#include "logger.hpp"
#include <exception>
#include <fcntl.h>
#include <odb/forward.hxx>
#include <odb/mysql/connection-factory.hxx>
#include <odb/query.hxx>
#include <odb/result.hxx>
#include <odb/transaction.hxx>
#include <sstream>
#include <string>
#include <memory>
#include <cstdlib>
#include <odb/database.hxx>
#include <odb/mysql/database.hxx>
#include <vector>

namespace lotso_im {
class ODBFactory
{
public:
    static std::shared_ptr<odb::core::database> create(const std::string &user, const std::string &pswd, const std::string &host,
                                                       const std::string &db, const std::string &cset, int port, int conn_pool_count)
    {
        std::unique_ptr<odb::mysql::connection_pool_factory> cpf(new odb::mysql::connection_pool_factory(conn_pool_count, 0));
        auto res = std::make_shared<odb::mysql::database>(user, pswd, db, host, port, "", cset, 0, std::move(cpf));
        return res;
    }
};

class UserTable
{
public:
    using ptr = std::shared_ptr<UserTable>;
    UserTable(const std::shared_ptr<odb::core::database> &db) : _db(db) {}
    bool insert(const std::shared_ptr<User> &user)
    {
        try {
            odb::transaction trans(_db->begin());
            _db->persist(*user);
            trans.commit();
        } catch (const std::exception &e) {
            LOG_ERROR("新增用户失败: {}:{}! ", user->nickname(), e.what());
            return false;
        }
        return true;
    }
    bool update(const std::shared_ptr<User> &user)
    {
        try {
            odb::transaction trans(_db->begin());
            _db->update(*user);
            trans.commit();
        } catch (const std::exception &e) {
            LOG_ERROR("更新用户失败:  {}:{}! ", user->nickname(), e.what());
            return false;
        }
        return true;
    }
    std::shared_ptr<User> select_by_nickname(const std::string &nickname)
    {
        std::shared_ptr<User> res;
        try {
            odb::transaction trans(_db->begin());
            typedef odb::query<User> query;
            typedef odb::result<User> result;
            res = std::shared_ptr<User>(_db->query_one<User>(query::nickname == nickname));
            trans.commit();
        } catch (const std::exception &e) {
            LOG_ERROR("根据昵称查询用户失败: {}:{}! ", nickname, e.what());
        }
        return res;
    }
    std::shared_ptr<User> select_by_phone(const std::string &phone)
    {
        std::shared_ptr<User> res;
        try {
            odb::transaction trans(_db->begin());
            typedef odb::query<User> query;
            typedef odb::result<User> result;
            res = std::shared_ptr<User>(_db->query_one<User>(query::phone == phone));
            trans.commit();
        } catch (const std::exception &e) {
            LOG_ERROR("根据手机号查询用户失败: {}:{}! ", phone, e.what());
        }
        return res;
    }
    std::shared_ptr<User> select_by_id(const std::string &user_id)
    {
        std::shared_ptr<User> res;
        try {
            odb::transaction trans(_db->begin());
            typedef odb::query<User> query;
            typedef odb::result<User> result;
            res = std::shared_ptr<User>(_db->query_one<User>(query::user_id == user_id));
            trans.commit();
        } catch (const std::exception &e) {
            LOG_ERROR("根据用户ID查询用户失败: {}:{}! ", user_id, e.what());
        }
        return res;
    }
    std::vector<User> select_multi_users(const std::vector<std::string> &id_list)
    {
        std::vector<User> res;
        try {
            odb::transaction trans(_db->begin());
            typedef odb::query<User> query;
            typedef odb::result<User> result;
            std::stringstream ss;
            ss << "user_id in (";
            for(const auto &id : id_list)
            {
                ss << "'" << id << "',";
            }
            std::string condition = ss.str();
            condition.pop_back();
            condition += ")";
            result r(_db->query<User>(condition));
            for(result::iterator i(r.begin()); i != r.end(); i++)
            {
                res.push_back(*i);
            }
            trans.commit();
        } catch (const std::exception &e) {
            LOG_ERROR("根据用户ID查询多个用户失败: {}! ", e.what());
        }
        return res;
    }
private:
    std::shared_ptr<odb::core::database> _db;
};
}
