#include <exception>
#include <memory>
#include <odb/database.hxx>
#include <odb/mysql/connection-factory.hxx>
#include <odb/mysql/connection.hxx>
#include <odb/mysql/database.hxx>
#include "student.hxx"
#include "student-odb.hxx"
#include <gflags/gflags.h>
#include <odb/mysql/transaction-impl.hxx>
#include <odb/transaction.hxx>
#include <utility>

 
DEFINE_string(host, "127.0.0.1", "这是 Mysql 服务器地址");
DEFINE_int32(port, 0, "这是 Mysql 服务器端口");
DEFINE_string(db, "my_im", "数据库默认库名称");
DEFINE_string(user, "imuser", "这是 Mysql 用户名");
DEFINE_string(pswd, "imuser_666", "这是 Nysql 密码");
DEFINE_string(cset, "utf8", "这是 Mysql 客户端字符集");
DEFINE_int32(max_pool, 3, "这是 Mysql 连接池最大连接数量");

void insert_classes(odb::mysql::database &db)
{
    try 
    {
        // 获取事务对象开启事务
        odb::transaction trans(db.begin());
        Classes c1("一年级一班");
        Classes c2("一年级二班");
        db.persist(c1);
        db.persist(c2);
        // 提交事务
        trans.commit();
    }
    catch(std::exception &e)
    {
        std::cout << "插入班级数据出错: " << e.what() << std::endl;
    }
}

void insert_student(odb::mysql::database &db)
{
    try 
    {
        // 获取事务对象开启事务
        odb::transaction trans(db.begin());
        Student s1(1, "张三", 18, 1);
        Student s2(2, "李四", 19, 1);
        Student s3(3, "王五", 18, 1);
        Student s4(4, "赵六", 15, 2);
        Student s5(5, "田七", 18, 2);
        Student s6(6, "刘八", 23, 2);
        db.persist(s1);
        db.persist(s2);
        db.persist(s3);
        db.persist(s4);
        db.persist(s5);
        db.persist(s6);
        // 提交事务
        trans.commit();
    }
    catch(std::exception &e)
    {
        std::cout << "插入学生数据出错: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    // 1. 构建连接池工厂配置对象
    std::unique_ptr<odb::mysql::connection_pool_factory> cpf(
            new odb::mysql::connection_pool_factory(FLAGS_max_pool, 0));
    // 2. 构造数据库操作对象
    odb::mysql::database db(
        FLAGS_user, FLAGS_pswd, FLAGS_db, FLAGS_host,
        FLAGS_port, "", FLAGS_cset, 0, std::move(cpf)
    );
    // 3. 数据操作
    insert_classes(db);
    // insert_student(db);
    return 0;
}