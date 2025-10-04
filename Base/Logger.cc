#include <iostream>

#include "Logger.h"

Logger& Logger::instance()
{
    static Logger logger;
    return logger;
}

void Logger::log(LogLevel logLevel,const std::string& message)
{
    std::string pre="";
    switch (logLevel)
    {
    case LogLevel::INFO:
        pre="[INFO]";
        break;
    case LogLevel::ERROR:
        pre="[ERROR]";
        break;
    case LogLevel::FATAL:
        pre="[FATAL]";
        break;
    case LogLevel::DEBUG:
        pre="[DEBUG]";
        break;
    default:
        break;
    };

    std::cout<<pre<<':'<<message<<'\n';
}

// int main()
// {
//     LOG_INFO("This is an %s message with number %d", "info", 1);
//     LOG_ERROR("This is an %s message with number %d", "error", 2);
//     LOG_FATAL("This is a %s message with number %d", "fatal", 3);
//     LOG_DEBUG("This is a %s message with number %d", "debug", 4);
//     return 0;
// }