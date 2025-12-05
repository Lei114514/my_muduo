#include "HttpResponse.h"
#include "../Base/Logger.h"
#include "../Net/Buffer.h"

void HttpResponse::setStatusCode(HttpResponse::HttpStatusCode code)
{
    
    statusCode_=code;
    switch (code)
    {
    case HttpStatusCode::K200Ok:
        setReasonPhrase("OK");
        break;
    case HttpStatusCode::k301MovedPermanently:
        setReasonPhrase("Moved Permanently");
        break;
    case HttpStatusCode::k400BadRequest:
        setReasonPhrase("Bad Request");
        break;
    case HttpStatusCode::k404NotFound:
        setReasonPhrase("Not Found");
        break;
    case HttpStatusCode::kUnknown:
        LOG_ERROR("HttpStatusCode is kUnknown");
        break;
    default:
        LOG_ERROR("HttpStatusCode is undefined");
        break;
    }
}

void HttpResponse::appendToBuffer(Buffer* buf) const
{
    //stateLine
    std::string statusLine= "HTTP/1.1 "
                            + std::to_string(static_cast<int>(statusCode_)) + " "
                            + reasonPhrase_+"\r\n"; 
    buf->append(statusLine.c_str(),statusLine.length());

    //headerLine
    for(auto t:headers_)
    {
        std::string headerLine= t.first + ": " + t.second + "\r\n" ;
        buf->append(headerLine.c_str(),headerLine.length());
    }

    std::string connectionHeader = "Connection: "
                                  + std::string(closeConnection_?"close":"keep-alive")
                                  + "\r\n";
    buf->append(connectionHeader.c_str(),connectionHeader.length());

    if(!body_.empty())
    {
        std::string contentLength = "Content-Length: "
                                   +std::to_string(body_.size())
                                   +"\r\n";
        buf->append(contentLength.c_str(),contentLength.length());
    }

    buf->append("\r\n",2);

    //body
    if(!body_.empty())
    {
        buf->append(body_.c_str(),body_.length());
    }
}