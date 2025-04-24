#pragma once
#include <iostream>
#include <unordered_map>

// rpcserver_ip= rpcserver_port= zookeeper_ip= zookeeper_port=

class MprpcConfig
{
public:
    // 解析加载配置文件
    void LoadConfigFile(const char *config_file);
    // 查询配置项信息
    std::string Load(const std::string &key);

private:
    std::unordered_map<std::string, std::string> _configMap;
};