
#include <iostream>
#include <string>
#include "../user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
using namespace fixbug;

// UserServic原本是一个本地服务
// 只要继承UserServiceRpc再重写两个虚函数就可以实现rpc服务
class UserService : public fixbug::UserServiceRpc
{
public:
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "doing local servie : Login" << std::endl;
        std::cout << "name:" << name << " pwd:" << pwd << std::endl;
        return true;
    }

    void Login(::google::protobuf::RpcController *controller,
               const ::fixbug::LoginRequest *request,
               ::fixbug::LoginResponse *response,
               ::google::protobuf::Closure *done)
    {
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool login_result = Login(name, pwd);

        // 把结果response
        ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(login_result);

        // 执行回调操作 直接执行响应对象数据的序列化和网络发送
        done->Run();
    }
};

int main(int argc, char *argv[])
{
    // 调用框架初始化操作
    MprpcApplication::Init(argc, argv);
    // 把UserService发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new UserService());
    // 启动一个rpc服务发布节点
    provider.Run();
}