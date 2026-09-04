#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>
#include <ostream>
#include <sstream>
#include <iostream>
#include <memory>
#include <string>

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

int main(int argc, char* argv[])
{
    char name[] = "Lotso";
    int age = 18;
    float score[3] = {88, 90.5, 99};

    Json::Value stu;
    stu["姓名"] = name;
    stu["年龄"] = 18;
    stu["成绩"].append(score[0]);
    stu["成绩"].append(score[1]);
    stu["成绩"].append(score[2]);
    // 打印出来序列化的结果看看
    std::string stu_str;
    bool ret = Serialize(stu, stu_str);
    if(ret == false) return -1;
    std::cout << stu_str << std::endl;

    // 反序列化
    Json::Value val;
    ret = UnSerialize(stu_str, val);
    if(ret == false) return -1;
    std::cout << val["姓名"].asString() << std::endl;
    std::cout << val["年龄"].asInt() << std::endl;
    int size = val["成绩"].size();
    for(int i = 0; i < size; i++)
    {
        std::cout << val["成绩"][i].asFloat() << std::endl;
    }
    return 0;
}