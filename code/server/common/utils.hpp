// 实现项目中一些公共的工具类接口
#include "logger.hpp"
#include <cstddef>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <atomic>
#include <random>
#include <iomanip>

namespace lotso_im {
// 1. 生成一个唯一 ID 的接口
std::string uuid()
{
    // 生成一个由16位随机字符组成的字符串作为唯一 ID
    // 1. 生成6个0-255之间的随机数字(1字节 -- 转为2个16进制字符) -- 生成12位16进制字符
    std::random_device rd;             // 实例化设备随机数对象, 用于生成设备随机数
    std::mt19937 generator(rd());  // 使用梅森旋转算法, 以设备随机数为种子, 实例化随机数对象
    std::uniform_int_distribution<int> distribution(0,255); // 限定数据范围

    std::stringstream ss;
    for(int i = 0; i < 6; i++)
    {
        if(i == 2) ss << "-";
        ss << std::setw(2) << std::setfill('0') << std::hex << distribution(generator);
    }
    ss << "-";
    // 2. 通过一个静态变量生成一个2字节的编号数字 -- 生成4位16进制数字字符
    static std::atomic<short> idx(0);
    short tmp = idx.fetch_add(1); // 每次来都会 + 1
    ss << std::setw(4) << std::setfill('0') << std::hex << tmp;
    return ss.str();
}

// 2. 文件的读写操作接口
bool readFile(const std::string &filename, std::string &body)
{
    // 实现读取一个文件的所有数据, 放入 body 中
    std::ifstream ifs(filename, std::ios::in | std::ios::binary);
    if(ifs.is_open() == false)
    {
        LOG_ERROR("打开文件 {} 失败! ", filename);
        return false;
    }
    // 获取文件大小
    ifs.seekg(0, std::ios::end); // 跳转到文件末尾
    size_t flen = ifs.tellg();    // 获取当前偏移量 -- 文件大小
    ifs.seekg(0, std::ios::beg); // 跳转到文件开始
    // 文件内容
    body.resize(flen);
    ifs.read(&body[0], flen);
    if(ifs.good() == false)
    {
        LOG_ERROR("读取文件 {} 数据失败! ", filename);
        ifs.close();
        return false;
    }
    ifs.close();
    return true;
}
bool writeFile(const std::string &filename, const std::string &body)
{
    // 实现将 body 中的数据, 写入 filename 对应的文件中
    std::ofstream ofs(filename, std::ios::out | std::ios::binary | std::ios::trunc);
    if(ofs.is_open() == false)
    {
        LOG_ERROR("打开文件 {} 失败! ", filename);
        return false;
    }
    ofs.write(body.c_str(), body.size());
    if(ofs.good() == false)
    {
        LOG_ERROR("写入文件 {} 数据失败! ", filename);
        ofs.close();
        return false;
    }
    ofs.close();
    return true;
}
}