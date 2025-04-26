#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"

using std::cout;
using std::endl;

int main(int argc, char **argv)
{
    // 想要使用rpc服务, 就要调用框架的初始化服务
    MprpcApplication::Init(argc, argv);

    // 演示调用远程发布的rpc方法的Login
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());

    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");
    fixbug::LoginResponse response;

    MprpcController controller;

    stub.Login(&controller, &request, &response, nullptr);

    // 如果rpc服务确实成功再继续接下来的内容
    if (controller.Failed())
    {
        std::cout << controller.ErrorText() << endl;
        return 0;
    }
    // rpc方法调用完成, 读响应
    if (response.result().errcode() == 0)
    {
        cout << "rpc login response:" << response.success() << endl;
    }
    else
    {
        cout << "rpc login response error : " << response.result().errmsg() << endl;
    }

    return 0;
}