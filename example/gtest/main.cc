#include <gtest/gtest.h>
#include <iostream>

int Add(int a, int  b)
{
    return a + b;
}

TEST(测试名称, 加法测试用例)
{
    // 判断相等
    ASSERT_EQ(Add(10, 20), 30);
    // 判断小于
    ASSERT_LT(Add(20, 20), 50);
}
TEST(测试名称, 字符串比较测试用例)
{
    std::string teststr = "Hello";
    // // 使用 ASSERT -- 失败之后直接退出这个用例，不会往后走
    // ASSERT_EQ(teststr, "hello"); // 这里就会出错
    // printf("失败后的打印\n");
    // ASSERT_EQ(teststr, "Hello");

    // 使用 EXECPT -- 失败之后会继续往后走
    EXPECT_EQ(teststr, "hello"); // 这里就会出错
    printf("失败后的打印\n");
    EXPECT_EQ(teststr, "Hello");
}

int main(int argc, char* argv[])
{
    // 单元测试框架的初始化
    testing::InitGoogleTest(&argc, argv);
    // 启动所有的单元测试
    return RUN_ALL_TESTS();
}