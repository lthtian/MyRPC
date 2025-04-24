#pragma once
#include "google/protobuf/service.h"
#include <memory>
#include <mymuduo/TcpServer.h>
#include <mymuduo/EventLoop.h>
#include <mymuduo/InetAddress.h>

// #include <muduo/net/TcpServer.h>
// #include <muduo/net/EventLoop.h>
// #include <muduo/net/InetAddress.h>
// using namespace muduo;
// using namespace muduo::net;

#include <functional>
#include <google/protobuf/descriptor.h>
#include <unordered_map>

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
    void SendRpcResponse(const TcpConnectionPtr &, google::protobuf::Message *);

    std::unique_ptr<TcpServer> m_tcpserverPtr;
    EventLoop m_eventLoop;

    // service服务类型信息
    struct ServiceInfo
    {
        google::protobuf::Service *m_service;
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor *> m_methodMap;
    };
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;
};