#pragma once
#include "asyncqueue.h"

namespace mprpc
{
    enum RpcLogLevel
    {
        info,
        error,
    };

    // Rpc框架提供的日志系统
    class RpcLogger
    {
    public:
        void SetLogLevel(RpcLogLevel lv);
        void Log(std::string msg);

        static RpcLogger &getInstance();

    private:
        RpcLogger();
        RpcLogger(const RpcLogger &) = delete;
        RpcLogger(RpcLogger &&) = delete;

        int _loglevel; // 记录日志级别
        AsyncQueue<std::string> _asyncQueue;
    };

#define log_info(logmsgformat, ...)                     \
    do                                                  \
    {                                                   \
        RpcLogger &logger = RpcLogger::getInstance();   \
        logger.SetLogLevel(info);                       \
        char c[1024] = {0};                             \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c);                                  \
    } while (0)

#define log_error(logmsgformat, ...)                    \
    do                                                  \
    {                                                   \
        RpcLogger &logger = RpcLogger::getInstance();   \
        logger.SetLogLevel(error);                      \
        char c[1024] = {0};                             \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c);                                  \
    } while (0)
}