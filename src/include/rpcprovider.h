#pragma once
#include "google/protobuf/service.h"
#include <memory>
#include <mymuduo/TcpServer.h>
#include <mymuduo/EventLoop.h>
#include <mymuduo/InetAddress.h>

class RpcProvider
{
public:
    // 所有rpc服务都会继承于goole::protobuf::Service
    // 选其为类型可以接收任意服务
    void NotifyService(google::protobuf::Service *service);

    // 启动Rpc节点, 开始启动远程网络调用服务
    void Run();

private:
    void OnConnection(const TcpConnectionPtr &);
    void OnMessage(const TcpConnectionPtr &, Buffer *, Timestamp);

    std::unique_ptr<TcpServer> m_tcpserverPtr;
    EventLoop m_eventLoop;
};