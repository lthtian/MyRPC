#include "logger.h"
#include <iostream>
#include <time.h>
using std::cout;
using std::endl;

namespace mprpc
{
    RpcLogger::RpcLogger()
    {
        // 启动专门的写日志线程
        std::thread writeLogTask([&]()
                                 {
        while(true)
        {
            // 获取当前的日期, 然后获取日志信息, 写入相应的日志文件
            time_t now = time(nullptr);
            tm* nowtm = localtime(&now);

            char file_name[128];
            sprintf(file_name, "%d-%d-%d-log.txt", 
                nowtm->tm_year + 1900, nowtm->tm_mon + 1, nowtm->tm_mday);
            FILE* pf = fopen(file_name, "a+");
            if(!pf)
            {
                cout << "logger file: " << file_name << " open error!" << endl;
                exit(EXIT_FAILURE);
            }

            std::string msg = _asyncQueue.pop();

            char time_buf[128] = {0};

            sprintf(time_buf, "%d:%d:%d =>[%s] ",
                nowtm->tm_hour, nowtm->tm_min, nowtm->tm_sec,
                (_loglevel == info ? "info" : "error"));
            msg.insert(0, time_buf);
            msg.append("\n");

            fputs(msg.c_str(), pf);
            fclose(pf);
        } });
        // 设置线程分离
        writeLogTask.detach();
    }

    void RpcLogger::SetLogLevel(RpcLogLevel lv)
    {
        _loglevel = lv;
    }

    void RpcLogger::Log(std::string msg)
    {
        _asyncQueue.push(msg);
    }

    RpcLogger &RpcLogger::getInstance()
    {
        static RpcLogger logger;
        return logger;
    }
}