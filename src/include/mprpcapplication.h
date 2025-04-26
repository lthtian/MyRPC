#pragma once
#include "mprpcconfig.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"

// mprpc框架的初始类, 用来加载配置文件
class MprpcApplication
{
public:
    static void Init(int argc, char *argv[]);
    static MprpcApplication &GetInstance();
    static MprpcConfig &getConfig();

private:
    MprpcApplication() {}
    MprpcApplication(const MprpcApplication &) = delete;
    MprpcApplication(MprpcApplication &&) = delete;

    static MprpcConfig _config;
};