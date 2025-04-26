#include "rpcprovider.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include "zookeepercli.h"
using std::cout;
using std::endl;
using std::string;

void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;
    // 获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *serviceDesc = service->GetDescriptor();
    std::string service_name(serviceDesc->name());
    int methodCnt = serviceDesc->method_count();
    cout << "service name : " << service_name << endl;

    for (int i = 0; i < methodCnt; i++)
    {
        // 获取指定下标的服务方法的描述
        const google::protobuf::MethodDescriptor *methodDesc = serviceDesc->method(i);
        std::string method_name(methodDesc->name());
        service_info.m_methodMap.insert({method_name, methodDesc});
        cout << "method name : " << method_name << endl;
    }
    service_info.m_service = service;
    m_serviceMap.insert({service_name, service_info});
}

// 启动Rpc节点, 开始启动远程网络调用服务
void RpcProvider::Run()
{
    std::string ip = MprpcApplication::GetInstance().getConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().getConfig().Load("rpcserverport").c_str());
    InetAddress address(port, ip);
    TcpServer server(&m_eventLoop, address, "RpcProvider");

    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));

    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this,
                                        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    server.setThreadNum(3);

    // 把当前rpc节点要发布的所有服务都注册到zk上, 让客户端通过zk发现服务而不是ip/port
    // 服务节点设置为永久, 方法节点设置为暂时, 只要rpc节点断开连接, 方法节点就会失效
    ZkClient zkCli;
    zkCli.start();
    for (auto &[name, service_info] : m_serviceMap)
    {
        std::string service_path = "/" + name;
        zkCli.create(service_path.c_str(), nullptr, 0);
        for (auto &[method_name, ptr] : service_info.m_methodMap)
        {
            std::string method_path = service_path + "/" + method_name;
            char method_data[129] = {0};
            sprintf(method_data, "%s:%d", ip.c_str(), port);
            zkCli.create(method_path.c_str(), method_data, strlen(method_data), ZOO_EPHEMERAL);
        }
    }

    cout << "RpcProvider start service at ip:" << ip << " port:" << port << endl;

    server.start();
    m_eventLoop.loop();
}

void RpcProvider::OnConnection(const TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        conn->shutdown();
    }
}

// 已建立连接用户的读事件回调, 如果远程有一个rpc服务的调用请求, 会响应该方法
// 执行请求的反序列化 和 响应的序列化
void RpcProvider::OnMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp timestamp)
{
    std::string recv_buf = buf->retrieveAllAsString();

    // 约定请求rpc服务需要 service_name / method_name / args_size / args_str
    // args_size是为了解决粘包问题
    uint32_t header_size = 0;
    recv_buf.copy((char *)&header_size, 4, 0);
    string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;
    string service_name, method_name;
    uint32_t args_size;
    if (rpcHeader.ParseFromString(rpc_header_str))
    {
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else
    {
        cout << "rpc_header_str:" << rpc_header_str << "parse error!" << endl;
        return;
    }

    string args_str = recv_buf.substr(4 + header_size, args_size);

    cout << "=============================================" << endl;
    cout << "header_size" << header_size << endl;
    cout << "service_name" << service_name << endl;
    cout << "method_name" << method_name << endl;
    cout << "args_str" << args_str << endl;
    cout << "=============================================" << endl;

    // 获取service对象和method对象
    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end())
    {
        cout << service_name << "is not exist!" << endl;
        return;
    }

    auto mit = it->second.m_methodMap.find(method_name);
    if (mit == it->second.m_methodMap.end())
    {
        std::cout << service_name << ":" << method_name << "is not exist" << endl;
        return;
    }

    google::protobuf::Service *service = it->second.m_service;
    const google::protobuf::MethodDescriptor *method = mit->second;

    // 生成rpc方法调用的请求request和response
    // 生成一个message对象, 设定其内部对应目标method的参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str)) // 反序列化存储到request中
    {
        cout << "request prase error" << endl;
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    // 给下面的method方法调用绑定一个Closure的回调函数
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider, const TcpConnectionPtr &, google::protobuf::Message *>(this, &RpcProvider::SendRpcResponse, conn, response);

    // 在框架上根据远端rpc的请求, 调用当前rpc节点上发布的方法
    service->CallMethod(method, nullptr, request, response, done);
}

void RpcProvider::SendRpcResponse(const TcpConnectionPtr &conn, google::protobuf::Message *response)
{
    std::string response_str;
    if (response->SerializeToString(&response_str))
    {
        conn->send(response_str);
    }
    else
    {
        cout << "serialize response_str error!" << endl;
    }
    conn->shutdown(); // rpc服务的提供方主动断开连接
}
