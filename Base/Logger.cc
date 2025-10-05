#include <iostream>
#include <sstream>

#include "Logger.h"
#include "Timestamp.h"

Logger& Logger::instance()
{
    static Logger logger;
    return logger;
}

void Logger::log(LogLevel logLevel,const char* file,int line,const char* func, const std::string& message)
{
    std::stringstream ss;

    ss<<Timestamp::now().toString();
    switch (logLevel)
    {
    case LogLevel::INFO:
        ss<<" [INFO]";
        break;
    case LogLevel::ERROR:
        ss<<" [ERROR]";
        break;
    case LogLevel::FATAL:
        ss<<" [FATAL]";
        break;
    case LogLevel::DEBUG:
        ss<<" [DEBUG]";
        break;
    default:
        break;
    };

    ss<<file<<' '<<line<<" ("<<func<<") "<<':'<<message;

    std::cout<<ss.str()<<'\n';
}

// int main()
// {
//     LOG_INFO("This is an %s message with number %d", "info", 1);
//     LOG_ERROR("This is an %s message with number %d", "error", 2);
//     LOG_DEBUG("This is a %s message with number %d", "debug", 3);
//     LOG_FATAL("This is a %s message with number %d", "fatal", 4);
//     LOG_ERROR("This is an %s message with number %d", "error", 5);
//     return 0;
// }