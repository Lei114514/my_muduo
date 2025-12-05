#pragma once

#include <map>
#include <string>

class Buffer;

class HttpResponse
{
public:
    enum HttpStatusCode{
        kUnknown,
        K200Ok= 200,
        k301MovedPermanently = 301,
        k400BadRequest = 400,
        k404NotFound = 404,
    };

    explicit HttpResponse(bool closeConnection)
        :statusCode_(kUnknown)
        ,closeConnection_(closeConnection)
        {};

    void setStatusCode(HttpStatusCode code);
    void setReasonPhrase(const std::string& reasonPhrase) { reasonPhrase_ = reasonPhrase; }
    void addHeader(const std::string& key, const std::string& value) { headers_[key] = value; }
    void setContentType(const std::string& contentType) { addHeader("Content-Type",contentType); }
    void setBody(const std::string& body) { body_ = body; }

    bool closeConnection() const { return closeConnection_; }
    
    void appendToBuffer(Buffer* buf) const ;
private:
    HttpStatusCode statusCode_;
    std::string reasonPhrase_; 
    
    bool closeConnection_;
    std::map<std::string,std::string> headers_;

    std::string body_;
};