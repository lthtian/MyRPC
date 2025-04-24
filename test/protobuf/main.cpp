#include "test.pb.h"
#include <iostream>
#include <string>
using namespace fixbug;

void test1()
{
    LoginRequest req;
    req.set_name("zhang san");
    req.set_pwd("123456");

    std::string send_str;
    if (req.SerializeToString(&send_str))
    {
        std::cout << send_str.c_str() << std::endl;
    }

    // 从 send_str反序列化一个login请求对象

    LoginRequest reqB;
    if (reqB.ParseFromString(send_str))
    {
        std::cout << reqB.name() << std::endl;
        std::cout << reqB.pwd() << std::endl;
    }
}

void test2()
{
    // LoginResponse rsp;
    // ResultCode* rc = rsp.mutable_result();
    // rc->set_errcode(1);
    // rc->set_errmsg("登录处理失败了");

    // 处理对象类型 / 列表类型
    GetFriendListsResponse rsp;
    ResultCode *rc = rsp.mutable_result();
    rc->set_errcode(0);

    // add_ + 列表名
    User *user1 = rsp.add_friend_list();
    user1->set_name("张三");
    user1->set_age(20);
    user1->set_sex(User::MAN);

    User *user2 = rsp.add_friend_list();
    user2->set_name("张三");
    user2->set_age(20);
    user2->set_sex(User::MAN);

    std::cout << rsp.friend_list_size() << std::endl;
}

int main()
{
    test2();
    return 0;
}