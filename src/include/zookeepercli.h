#pragma once

#include <semaphore.h>
#define THREADED
extern "C"
{
#include <zookeeper/zookeeper.h>
}
#include <string>

class ZkClient
{
public:
    ZkClient();
    ~ZkClient();
    void start();
    void create(const char *path, const char *data, int datalen, int state = 0);
    std::string getData(const char *path);

private:
    zhandle_t *_zhandle;
};