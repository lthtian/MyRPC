#include "mprpcapplication.h"
#include <iostream>
using std::cout;
using std::endl;
#include <unistd.h>

MprpcConfig MprpcApplication::_config;

MprpcApplication &MprpcApplication::GetInstance()
{
    static MprpcApplication app;
    return app;
}

void MprpcApplication::Init(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "format: command -i <configfile>" << endl;
        exit(EXIT_FAILURE);
    }

    int c = 0;
    std::string config_file;
    while ((c = getopt(argc, argv, "i:")) != -1)
    {
        switch (c)
        {
        case 'i':
            config_file = optarg;
            break;
        case '?':
            exit(EXIT_FAILURE);
        case ':':
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }

    // 开始加载配置文件 rpcserver_ip= rpcserver_port= zookeeper_ip= zookeeper_port=
    _config.LoadConfigFile(config_file.c_str());
    std::cout << "rpcserverip:" << _config.Load("rpcserverip") << std::endl;
    std::cout << "rpcserverport:" << _config.Load("rpcserverport") << std::endl;
    std::cout << "zookeeperip:" << _config.Load("zookeeperip") << std::endl;
    std::cout << "zookeeperport:" << _config.Load("zookeeperport") << std::endl;
}

MprpcConfig &MprpcApplication::getConfig()
{
    return _config;
}