#include "mprpcchannel.h"
#include "rpcheader.pb.h"
#include "mprpcapplication.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <error.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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
        cout << "serialize request error!" << endl;
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
        cout << "serialize header error" << endl;
        return;
    }

    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char *)&header_size, 4));
    send_rpc_str += rpc_header_str;
    send_rpc_str += args_str;

    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        cout << "create socket error!" << endl;
        exit(EXIT_FAILURE);
    }

    std::string ip = MprpcApplication::GetInstance().getConfig().Load("rpcserverip");
    std::string port = MprpcApplication::GetInstance().getConfig().Load("rpcserverport");

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port.c_str()));
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (-1 == connect(clientfd, (struct sockaddr *)&server_addr, sizeof(server_addr)))
    {
        cout << "connection error! errno:" << errno << " errmsg:" << strerror(errno) << endl;
        close(clientfd);
        exit(EXIT_FAILURE);
    }

    if (-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
    {
        cout << "send error" << endl;
        close(clientfd);
        return;
    }

    // 接收rpc请求的响应值
    char buf[1024] = {0};
    int n = recv(clientfd, buf, 1024, 0);
    if (-1 == n)
    {
        cout << "receive error" << endl;
        close(clientfd);
        return;
    }

    if (!response->ParseFromArray(buf, n))
    {
        cout << "parse error!" << endl;
        close(clientfd);
        return;
    }
    close(clientfd);
}