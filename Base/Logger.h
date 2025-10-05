#pragma once

#include <string>
#include "noncopyable.h"

#define LOG_INFO(logFormat, ...)                                      \
    do                                                                \
    {                                                                 \
        Logger& logger=Logger::instance();                            \
        char buf[1024]={'\0'};                                        \
        snprintf(buf,sizeof(buf),logFormat,##__VA_ARGS__);            \
        logger.log(LogLevel::INFO,__FILE__,__LINE__,__func__,buf);                               \
    } while (0)

#define LOG_ERROR(logFormat,...)                                      \
    do{                                                               \
        Logger& logger=Logger::instance();                            \
        char buf[1024]={'\0'};                                        \
        snprintf(buf,sizeof(buf),logFormat,##__VA_ARGS__);            \
        logger.log(LogLevel::ERROR,__FILE__,__LINE__,__func__,buf);                              \
    }while(0)

#define LOG_FATAL(logFormat,...)                                      \
    do{                                                               \ 
        Logger& logger=Logger::instance();                            \
        char buf[1024]={'\0'};                                        \
        snprintf(buf,sizeof(buf),logFormat,##__VA_ARGS__);            \
        logger.log(LogLevel::FATAL,__FILE__,__LINE__,__func__,buf);                              \
        exit(-1);                                                     \
    }while(0)

#ifdef DEBUG
#define LOG_DEBUG(logFormat,...)                                      \
    do{                                                               \
        Logger& logger=Logger::instance();                            \
        char buf[1024]={'\0'};                                        \
        snprintf(buf,sizeof(buf),logFormat,##__VA_ARGS__);            \
        logger.log(LogLevel::DEBUG,__FILE__,__LINE__,__func__,buf);                              \
    }while(0)
#else
#define LOG_DEBUG(logFormat,...);
#endif

enum class LogLevel
{
    INFO,  // 普通信息
    ERROR, // 错误信息
    FATAL, // core dump信息
    DEBUG, // 调试信息
};

class Logger: public noncopyable
{
public:
    static Logger& instance();
    void log(LogLevel logLevel,const char* file,int line,const char* func, const std::string& message);
};
