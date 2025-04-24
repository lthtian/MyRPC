#include "mprpcconfig.h"
#include <string>

// 解析加载配置文件
void MprpcConfig::LoadConfigFile(const char *config_file)
{
    FILE *pf = fopen(config_file, "r");
    if (nullptr == pf)
    {
        exit(EXIT_FAILURE);
    }
    while (!feof(pf))
    {
        char buf[512] = {0};
        fgets(buf, 512, pf);
        std::string sbuf(buf);
        for (int i = 0; i < sbuf.size(); i++)
        {
            if (sbuf[i] == ' ' || sbuf[i] == '\r' || sbuf[i] == '\n')
                sbuf.erase(i, 1), i--;
        }
        if (sbuf[0] == '#')
            continue;

        int idx = sbuf.find('=');
        std::string key = sbuf.substr(0, idx);
        std::string value = sbuf.substr(idx + 1);
        _configMap[key] = value;
    }
}
// 查询配置项信息
std::string MprpcConfig::Load(const std::string &key)
{
    if (_configMap.count(key) == 0)
        return "";
    return _configMap[key];
}