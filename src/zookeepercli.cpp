#include "zookeepercli.h"
#include "mprpcapplication.h"
#include <iostream>

void global_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT && state == ZOO_CONNECTED_STATE)
    {
        sem_t *sem = (sem_t *)zoo_get_context(zh);
        sem_post(sem);
    }
}

ZkClient::ZkClient() : _zhandle(nullptr)
{
}

ZkClient::~ZkClient()
{
    if (_zhandle != nullptr)
        zookeeper_close(_zhandle);
}

void ZkClient::start()
{
    std::string ip = MprpcApplication::GetInstance().getConfig().Load("zookeeperip");
    std::string port = MprpcApplication::GetInstance().getConfig().Load("zookeeperport");
    std::string connstr = ip + ":" + port;

    // zookeeper_mt 多线程版本
    // zookeeper的API提供三个线程
    // API调用线程 / 网络IO线程 / watchar回调线程

    // 这部分只是本地内容准备, 需要后面等待连接服务端
    _zhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    if (!_zhandle)
    {
        std::cout << "zookeeper init fail!" << std::endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(_zhandle, &sem);

    sem_wait(&sem);
    std::cout << "zookeeper init success!" << std::endl;
}

// 要新开辟的路径, 存入对应路径节点的数据(ip/port), 数据长度, 是永久还是临时
void ZkClient::create(const char *path, const char *data, int datalen, int state)
{
    char path_buf[128];
    int bufferlen = sizeof path_buf;
    // 先判断path表示的节点是否存在
    int flag = zoo_exists(_zhandle, path, 0, nullptr);

    if (ZNONODE == flag)
    {
        flag = zoo_create(_zhandle, path, data, datalen,
                          &ZOO_OPEN_ACL_UNSAFE, state, path_buf, bufferlen);
        if (flag == ZOK)
        {
            std::cout << "znode create success, path: " << path << std::endl;
        }
        else
        {
            std::cout << "znode create error, path " << path << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

// 访问一个服务
std::string ZkClient::getData(const char *path)
{
    char buf[64];
    int buflen = sizeof buf;
    int flag = zoo_get(_zhandle, path, 0, buf, &buflen, nullptr);
    if (flag == ZOK)
        return buf;
    else
    {
        std::cout << "get znode error!" << std::endl;
        return "";
    }
}