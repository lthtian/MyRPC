#include "mprpcchannel.h"
#include "rpcheader.pb.h"
#include "mprpcapplication.h"
#include "zookeepercli.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <error.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

using std::cout;
using std::endl;

void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                              google::protobuf::RpcController *controller, const google::protobuf::Message *request,
                              google::protobuf::Message *response, google::protobuf::Closure *done)
{
    const google::protobuf::ServiceDescriptor *sd = method->service();
    std::string service_name(sd->name());
    std::string method_name(method->name());

    // 获取参数的序列化字符串长度
    uint32_t args_size = 0;
    std::string args_str;
    if (request->SerializeToString(&args_str))
    {
        args_size = args_str.size();
    }
    else
    {
        controller->SetFailed("serialize request error!");
        return;
    }

    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    uint32_t header_size = 0;
    std::string rpc_header_str;
    if (rpcHeader.SerializeToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();
    }
    else
    {
        controller->SetFailed("serialize header error");
        return;
    }

    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char *)&header_size, 4));
    send_rpc_str += rpc_header_str;
    send_rpc_str += args_str;

    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        controller->SetFailed("create socket error!");
        exit(EXIT_FAILURE);
    }

    // 最普通就是直接从配置文件中读取rpcserver的消息
    // std::string ip = MprpcApplication::GetInstance().getConfig().Load("rpcserverip");
    // std::string port = MprpcApplication::GetInstance().getConfig().Load("rpcserverport");

    // 还可以通过服务名和方法名从zookeeper上查询
    ZkClient zkCli;
    zkCli.start();

    std::string path = "/" + service_name + "/" + method_name;
    std::string data = zkCli.getData(path.c_str());
    if (data == "")
    {
        std::string str = "no method found in path: " + path;
        controller->SetFailed(str);
        return;
    }

    int idx = data.find_first_of(":");
    std::string ip = data.substr(0, idx);
    std::string port = data.substr(idx + 1);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port.c_str()));
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (-1 == connect(clientfd, (struct sockaddr *)&server_addr, sizeof(server_addr)))
    {
        std::string str = "connection error! errno:" + std::to_string(errno) + " errmsg:" + strerror(errno);
        controller->SetFailed(str);
        close(clientfd);
        exit(EXIT_FAILURE);
    }

    if (-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
    {
        controller->SetFailed("send error");
        close(clientfd);
        return;
    }

    // 接收rpc请求的响应值
    char buf[1024] = {0};
    int n = recv(clientfd, buf, 1024, 0);
    if (-1 == n)
    {
        controller->SetFailed("receive error");
        close(clientfd);
        return;
    }

    if (!response->ParseFromArray(buf, n))
    {
        controller->SetFailed("parse error!");
        close(clientfd);
        return;
    }
    close(clientfd);
}