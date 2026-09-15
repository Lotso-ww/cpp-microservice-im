// ===========================================================================
// person.hxx —— ODB 持久化类定义
// ===========================================================================
// 使用方法：
//   1. 生成 ODB 支持代码（注意：用了 boost::posix_time 必须带 --profile）：
//        odb -d mysql --std c++11 --generate-query --generate-schema \
//            --profile boost/date-time person.hxx
//   2. 生成的文件：person-odb.cxx / person-odb.hxx / person-odb.ixx / person.sql
// ===========================================================================

#pragma once

#include <string>
#include <cstddef>                  // std::size_t
#include <boost/date_time/posix_time/posix_time.hpp>

// ====== 关键：odb/core.hxx 必须放在 #pragma db object 之前 ======
// 因为它定义了 odb::access 类（下文 friend class odb::access; 需要它可见）
#include <odb/core.hxx>

typedef boost::posix_time::ptime ptime;

// ---- 类的持久化声明 ----
// #pragma db object 让 ODB 编译器把 Person 类视为数据库对象
#pragma db object
class Person {
public:
    // 构造函数（业务代码用）
    Person(const std::string& name, int age, const ptime& update)
        : _name(name), _age(age), _update(update) {}

    // 访问器 / 修改器
    const std::string& name()  const { return _name; }
    int age()                  const { return _age; }
    unsigned long id()         const { return _id; }   // 暴露 auto id 给外部查询
    std::string update()       const { return boost::posix_time::to_simple_string(_update); }

    void name(const std::string& v) { _name = v; }
    void age(int v)                 { _age = v; }

private:
    // ---- ODB 必须 ----
    // 1) 友元声明：让 ODB 生成的数据库支持代码能访问 Person 的私有成员和默认构造函数
    friend class odb::access;

    // 2) 默认构造函数（ODB 重建对象时用）
    Person() {}

    // ---- 字段 ----
    // #pragma db id auto：把 _id 作为主键，auto 表示数据库自增（MySQL 的 AUTO_INCREMENT）
    #pragma db id auto
    unsigned long _id;

    unsigned short _age;
    std::string    _name;

    // #pragma db type("TIMESTAMP") not_null：精确指定 MySQL 列类型 + 非空
    #pragma db type("TIMESTAMP") not_null
    ptime _update;
};
