#include "rpcprovider.h"
#include "mprpcapplication.h"
#include <functional>

void RpcProvider::NotifyService(google::protobuf::Service *service)
{
}

// 启动Rpc节点, 开始启动远程网络调用服务
void RpcProvider::Run()
{
    std::string ip = MprpcApplication::GetInstance().getConfig().Load("rcpserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().getConfig().Load("rcpserverport").c_str());
    InetAddress address(port, ip);
    TcpServer server(&m_eventLoop, address, "RpcProvider");

    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));

    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this,
                                        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    server.setThreadNum(3);

    cout << "RpcProvider start service at ip:" << ip << " port:" << port << endl;

    server.start();
    m_eventLoop.loop();
}

void RpcProvider::OnConnection(const TcpConnectionPtr &)
{
}

void RpcProvider::OnMessage(const TcpConnectionPtr &, Buffer *, Timestamp)
{
}
