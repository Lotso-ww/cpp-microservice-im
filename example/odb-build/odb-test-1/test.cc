// ===========================================================================
// test.cc —— ODB 端到端示例
// ===========================================================================
// 编译命令（注意：-lodb-boost 必须有，对应 person.hxx 里的 boost/date-time profile）：
//   g++ -std=c++11 -o test test.cc person-odb.cxx \
//       -lodb-mysql -lodb-boost -lodb -lmysqlclient
// ===========================================================================

#include <string>
#include <memory>
#include <iostream>
#include <exception>

// ODB 核心头文件
#include <odb/database.hxx>
#include <odb/transaction.hxx>
#include <odb/query.hxx>        // 支持 query:: 类型安全查询

// MySQL 后端头文件
#include <odb/mysql/database.hxx>

// 业务代码头文件
#include "person.hxx"
#include "person-odb.hxx"

// ===========================================================================
// 数据库连接参数 —— 和本项目 imuser/imuser_666/my_im 对齐
// ===========================================================================
static const char* DB_USER     = "imuser";
static const char* DB_PASSWD   = "imuser_666";
static const char* DB_NAME     = "my_im";
static const char* DB_HOST     = "127.0.0.1";
static const int   DB_PORT     = 0;   // 0 = MySQL 默认 3306
static const char* DB_CHARSET  = "utf8";

// ===========================================================================
// 主流程
// ===========================================================================
int main() {
    try {
        std::cout << "=== 1. 连接数据库 ===" << std::endl;
        auto db = std::make_shared<odb::mysql::database>(
            DB_USER, DB_PASSWD, DB_NAME, DB_HOST,
            DB_PORT, /*socket=*/nullptr, DB_CHARSET, /*flags=*/0
        );
        std::cout << "连接 OK (" << DB_HOST << "/" << DB_NAME << ")" << std::endl;

        // -------------------------------------------------------------------
        // 2. 插入两条记录 —— 事务内 persist + commit
        // -------------------------------------------------------------------
        std::cout << "\n=== 2. 插入 Person 记录 ===" << std::endl;

        {
            odb::transaction t(db->begin());    // 开启事务

            ptime now = boost::posix_time::second_clock::local_time();
            Person zhang("小张", 18, now);
            Person wang("小王", 19, now);

            // persist 返回 Person::_id 的类型（unsigned long），显式 cast 方便直接流输出
            unsigned long zid = static_cast<unsigned long>(db->persist(zhang));
            unsigned long wid = static_cast<unsigned long>(db->persist(wang));

            t.commit();

            std::cout << "INSERT person id=" << zid << " " << zhang.name()
                      << " age=" << zhang.age() << std::endl;
            std::cout << "INSERT person id=" << wid << " " << wang.name()
                      << " age=" << wang.age() << std::endl;
        }

        // -------------------------------------------------------------------
        // 3. 条件查询 —— 使用 odb::query<Person> 类型安全查询
        //    query::update 对应 person.hxx 里的 _update 字段
        // -------------------------------------------------------------------
        std::cout << "\n=== 3. 查询 update 落在指定时间窗内的 Person ===" << std::endl;

        {
            odb::transaction t(db->begin());

            typedef odb::query<Person> query;
            typedef odb::result<Person> result;

            // 注：MySQL TIMESTAMP UTC 范围 1970-01-01 ~ 2038-01-19，时间窗要收在此范围内
            ptime p = boost::posix_time::time_from_string("2020-01-01 00:00:00");
            ptime e = boost::posix_time::time_from_string("2037-12-31 23:59:59");

            result r = db->query<Person>(
                query::update < e && query::update > p
            );

            for (result::iterator i(r.begin()); i != r.end(); ++i) {
                std::cout << "query hit: " << i->name()
                          << " age=" << i->age()
                          << " update=" << i->update() << std::endl;
            }

            t.commit();
        }

        // -------------------------------------------------------------------
        // 4. 按 id 更新
        // -------------------------------------------------------------------
        std::cout << "\n=== 4. 更新小王的年龄 ===" << std::endl;
        {
            odb::transaction t(db->begin());

            typedef odb::query<Person> query;
            auto p = db->query_one<Person>(query::name == "小王");
            if (p) {
                p->age(20);
                db->update(*p);
                std::cout << "UPDATE 小王 age -> 20" << std::endl;
            } else {
                std::cout << "WARN: 小王不存在" << std::endl;
            }

            t.commit();
        }

        // -------------------------------------------------------------------
        // 5. 再次查询验证
        // -------------------------------------------------------------------
        std::cout << "\n=== 5. 最终校验 ===" << std::endl;
        {
            odb::transaction t(db->begin());

            typedef odb::query<Person> query;
            typedef odb::result<Person> result;
            result r = db->query<Person>("order by age");
            for (result::iterator i(r.begin()); i != r.end(); ++i) {
                std::cout << "FINAL: id=" << i->id() << " "
                          << i->name() << " age=" << i->age() << std::endl;
            }

            t.commit();
        }

        std::cout << "\n=== ODB 示例执行完毕 ===" << std::endl;
    } catch (const odb::exception& e) {
        std::cerr << "ODB exception: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
